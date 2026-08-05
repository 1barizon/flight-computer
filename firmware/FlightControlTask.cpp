/**
 * @file FlightControlTask.cpp
 * @brief Implementation of the 50Hz flight control task
 *
 * @see FlightControlTask.h for API and configuration documentation
 * @see firmware/REFACTORING_PLAN.md - Fase 7
 */

#include "flight/FlightControlTask.h"

#include <esp_task_wdt.h>
#include <esp_timer.h>
#include <ESP32Servo.h>

#include "config.h"
#include "sensors/BMP585Sensor.h"
#include "sensors/LSM6DS3Sensor.h"
#include "modules/parachute_module.h"
#include "flight/FlightStateMachine.h"
#include "flight/LoggerTask.h"

TaskHandle_t  g_flightControlTaskHandle = nullptr;
QueueHandle_t sensorDataQueue           = nullptr;

namespace {

BMP585Sensor*       g_baro = nullptr;
LSM6DS3Sensor*       g_imu = nullptr;
FlightStateMachine*  g_fsm = nullptr;

bool  g_parachuteActuated = false;

FlightControlStats g_stats = {0, 0, 0, 0, 0};

/**
 * @brief Consolida as leituras atuais dos sensores e o estado da FSM
 * @note Campos de GPS ficam nos valores default (0/false) — GPS e'
 *       responsabilidade da TelemetryTask (5Hz), fora do escopo desta task
 */
SensorData buildSensorData() {
  SensorData data;

  data.timestamp     = millis();
  data.packet_count  = static_cast<uint16_t>(g_stats.cycleCount);

  data.altitude         = g_baro->getAltitude();
  data.pressure         = g_baro->getPressure();
  data.temperature      = g_baro->getTemperature();
  data.verticalVelocity = g_baro->getVerticalVelocity();
  data.maxAltitude      = g_baro->getMaxAltitude();

  g_imu->getAcceleration(&data.accelX, &data.accelY, &data.accelZ);
  g_imu->getGyroscope(&data.gyroX, &data.gyroY, &data.gyroZ);
  data.totalAccel = g_imu->getTotalAccel();

  data.state              = g_fsm->getState();
  data.parachute_deployed = g_fsm->isParachuteDeployed();

  return data;
}

/**
 * @brief Aciona o servo de liberacao do paraquedas (one-shot, idempotente)
 * @note Chamada apenas quando a FSM confirma as condicoes de deploy
 *       (FlightStateMachine::detectParachute — apogeu + Vz negativo estavel,
 *       Opcao A). O servo (ParachuteServo) e' dono deste modulo.
 */
void deployParachute() {
  ParachuteServo.write(SERVO_OPEN);
}

}  // namespace

bool initFlightControlTask() {
  g_baro = new BMP585Sensor();
  g_imu  = new LSM6DS3Sensor();

  if (!g_baro->begin() || !g_imu->begin()) {
    Serial.println("[FlightControl] FATAL: sensor initialization failed");
    return false;
  }

  g_fsm = new FlightStateMachine(g_baro, g_imu);
  if (!g_fsm->begin()) {
    Serial.println("[FlightControl] FATAL: FSM initialization failed");
    return false;
  }
  // g_fsm->begin() restored the NVS snapshot after a watchdog reboot:
  // mark the actuator as already fired so the task does not re-deploy,
  // and keep the servo open if the chute was already released mid-flight.
  g_parachuteActuated = g_fsm->isParachuteDeployed();

  sensorDataQueue = xQueueCreate(SENSOR_DATA_QUEUE_LEN, sizeof(SensorData));
  if (sensorDataQueue == nullptr) {
    Serial.println("[FlightControl] FATAL: failed to create sensorDataQueue");
    return false;
  }

  if (!setupServo(g_fsm->isParachuteDeployed())) {
    Serial.println("[FlightControl] FATAL: failed to close parachute");
    return false;
  }

  

  const BaseType_t created = xTaskCreatePinnedToCore(
      taskFlightControl, "FlightControl", FLIGHT_CONTROL_STACK_SIZE,
      nullptr, FLIGHT_CONTROL_PRIORITY, &g_flightControlTaskHandle,
      FLIGHT_CONTROL_CORE);

  if (created != pdPASS) {
    Serial.println("[FlightControl] FATAL: failed to create task");
    return false;
  }

  return true;
}

void taskFlightControl(void* pvParameters) {
  (void)pvParameters;

  // Inicializado aqui (nao em initFlightControlTask()) para que o watchdog
  // so seja armado quando a task que o alimenta esta de fato rodando — se
  // xTaskCreatePinnedToCore falhasse com o init la, o TWDT ficaria armado
  // globalmente sem nenhuma task para chamar esp_task_wdt_reset().
  esp_task_wdt_config_t twdt_config = {
      .timeout_ms = FLIGHT_CONTROL_WDT_TIMEOUT_S * 1000,
      .idle_core_mask = 0,
      .trigger_panic = true
  };
  if (esp_task_wdt_init(&twdt_config) != ESP_OK) {
    Serial.println("[FlightControl] ERROR: watchdog init failed");
    logMessage(TASK_ID_FLIGHT_CONTROL, LOG_LEVEL_ERROR, "Watchdog init failed");
  }

  if (esp_task_wdt_add(nullptr) != ESP_OK) {
    Serial.println("[FlightControl] ERROR: failed to register with watchdog");
    logMessage(TASK_ID_FLIGHT_CONTROL, LOG_LEVEL_ERROR,
               "Failed to register with watchdog");
  }

  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(FLIGHT_CONTROL_PERIOD_MS);

  for (;;) {
    vTaskDelayUntil(&lastWakeTime, period);

    const int64_t t0 = esp_timer_get_time();

    // 1) Sensores (ordem sequencial, ambos non-blocking)
    g_baro->update();
    g_imu->update();

    // 2) FSM
    g_fsm->update();

    // 3) Consolidar dados
    const SensorData data = buildSensorData();

    // 4) Deploy do paraquedas (safety-critical, one-shot)
    if (data.parachute_deployed && !g_parachuteActuated) {
      deployParachute();
      g_parachuteActuated = true;
      Serial.println("[FlightControl] PARACHUTE DEPLOYED");
      logMessage(TASK_ID_FLIGHT_CONTROL, LOG_LEVEL_WARN, "Parachute deployed");
    }

    // 5) Queue -> TelemetryTask
    if (xQueueSend(sensorDataQueue, &data, 0) != pdPASS) {
      g_stats.queueDropCount++;
    }

    // 6) Watchdog
    esp_task_wdt_reset();

    // 7) Metricas de tempo de execucao
    const int64_t execTimeUs = esp_timer_get_time() - t0;
    g_stats.cycleCount++;
    g_stats.lastExecTimeUs = static_cast<int32_t>(execTimeUs);
    if (execTimeUs > g_stats.maxExecTimeUs) {
      g_stats.maxExecTimeUs = static_cast<int32_t>(execTimeUs);
    }
    if (execTimeUs > static_cast<int64_t>(FLIGHT_CONTROL_PERIOD_MS) * 1000) {
      g_stats.overrunCount++;
      logMessage(TASK_ID_FLIGHT_CONTROL, LOG_LEVEL_WARN, "Cycle overrun");
    }
  }
}

const FlightControlStats& getFlightControlStats() {
  return g_stats;
}

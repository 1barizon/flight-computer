/**
 * @file FlightControlTask.cpp
 * @brief Implementation of the 50Hz flight control task
 *
 * @see FlightControlTask.h for API and configuration documentation
 * @see firmware/REFACTORING_PLAN.md - Fase 7
 */

#include "FlightControlTask.h"

#include <esp_task_wdt.h>
#include <esp_timer.h>

#include "BMP585Sensor.h"
#include "sensors/LSM6DS3Sensor.h"
#include "FlightStateMachine.h"

TaskHandle_t  g_flightControlTaskHandle = nullptr;
QueueHandle_t sensorDataQueue           = nullptr;

namespace {

BMP585Sensor*       g_baro = nullptr;
LSM6DS3Sensor*       g_imu = nullptr;
FlightStateMachine*  g_fsm = nullptr;

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

  sensorDataQueue = xQueueCreate(SENSOR_DATA_QUEUE_LEN, sizeof(SensorData));
  if (sensorDataQueue == nullptr) {
    Serial.println("[FlightControl] FATAL: failed to create sensorDataQueue");
    return false;
  }

  esp_task_wdt_init(FLIGHT_CONTROL_WDT_TIMEOUT_S, true);

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

  esp_task_wdt_add(nullptr);

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

    // 4) Queue -> TelemetryTask
    if (xQueueSend(sensorDataQueue, &data, 0) != pdPASS) {
      g_stats.queueDropCount++;
    }

    // 5) Watchdog
    esp_task_wdt_reset();

    // 6) Metricas de tempo de execucao
    const int64_t execTimeUs = esp_timer_get_time() - t0;
    g_stats.cycleCount++;
    g_stats.lastExecTimeUs = static_cast<int32_t>(execTimeUs);
    if (execTimeUs > g_stats.maxExecTimeUs) {
      g_stats.maxExecTimeUs = static_cast<int32_t>(execTimeUs);
    }
    if (execTimeUs > static_cast<int64_t>(FLIGHT_CONTROL_PERIOD_MS) * 1000) {
      g_stats.overrunCount++;
    }
  }
}

const FlightControlStats& getFlightControlStats() {
  return g_stats;
}

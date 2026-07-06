#include "../sensors/BMP585Sensor.h"
#include "../sensors/LSM6DS3Sensor.h"







void taskFlightControl(void* pvParameters) {
  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(20);

  for (;;) {
    vTaskDelayUntil(&lastWakeTime, period);

    const int64_t t0 = esp_timer_get_time();

    // 1) Sensores
    updateBMP585();
    updateLSM6DS3();

    // 2) FSM
    flightStateMachine.update();

    // 3) Consolidar dados
    SensorData data = buildSensorData();

    // 4) Queue
    if (xQueueSend(sensorDataQueue, &data, 0) != pdPASS) {
      queue_drop_count++;
    }

    // 5) Watchdog
    feedWatchdog();

    // 6) Tempo de execução
    const int64_t dt_us = esp_timer_get_time() - t0;
    if (dt_us > 20000) {
      overrun_count++;
    }
  }
}

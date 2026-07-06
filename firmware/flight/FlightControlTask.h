#pragma once

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

// Forward declarations (ajuste para os tipos reais do projeto)
struct SensorData;
class FlightStateMachine;

// Handle da task (definido no .cpp)
extern TaskHandle_t g_flightControlTaskHandle;

// Queue de saída (se ela for global no seu projeto; senão remova daqui)
extern QueueHandle_t sensorDataQueue;

// Inicializa/cria a task pinned no Core 1
bool initFlightControlTask();

// Função da task (entrypoint FreeRTOS)
void taskFlightControl(void* pvParameters);

// (Opcional) métricas para debug/telemetria
struct FlightControlStats {
  uint32_t cycleCount;
  uint32_t overrunCount;    // dt > 20ms
  uint32_t queueDropCount;  // falha ao enviar na queue
  int32_t  lastExecTimeUs;
  int32_t  maxExecTimeUs;
};

const FlightControlStats& getFlightControlStats();

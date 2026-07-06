/**
 * @file TelemetryTask.cpp
 * @brief Implementation of the 5Hz telemetry task
 *
 * CSV field order mirrors the v2.0 draft table in
 * firmware/REFACTORING_PLAN.md (Fase 10). `packetQuality` and real `rssi`
 * are intentionally omitted until Fase 10 defines them — emitting
 * placeholder zeros for those would be misleading to downstream consumers
 * (Receiver / WebUI).
 *
 * @see TelemetryTask.h for API and configuration documentation
 * @see firmware/REFACTORING_PLAN.md - Fase 7
 */

#include "TelemetryTask.h"

#include "../config.h"
#include "../modules/filesystem_module.h"
#include "../modules/lora_module.h"
#include "../sensors/GPSModule.h"
#include "FlightControlTask.h"  // extern sensorDataQueue

TaskHandle_t g_telemetryTaskHandle = nullptr;

namespace {

GPSModule* g_gps = nullptr;

TelemetryStats g_stats = {0, 0, 0};

/**
 * @brief Deriva o caminho do arquivo CSV a partir da hora do GPS
 * @note Sem fix disponivel no momento da chamada, usa "NOFIX" (nao bloqueia
 *       esperando o GPS — ver REFACTORING_PLAN.md, tabela de riscos)
 */
String buildDataFilePath() {
  String stamp = g_gps->getTimeString();  // "HH:MM:SS" ou "nan"
  if (stamp == "nan") {
    stamp = "NOFIX";
  } else {
    stamp.replace(":", "");
  }
  return "/" + stamp + "-" + file_name;
}

/**
 * @brief Monta a linha CSV de telemetria (Serial file + LoRa)
 * @note Formato provisorio — Fase 10 alinha o formato final com o parser
 *       do Receiver (rssi/packetQuality ficam pendentes ate la)
 */
String assembleTelemetry(const SensorData& data) {
  return TEAM_ID + "," +
         String(data.timestamp) + "," +
         String(data.packet_count) + "," +
         String(data.altitude, 2) + "," +
         String(data.temperature, 2) + "," +
         "0" + ","  // umi — sem sensor de umidade (placeholder ate ser adicionado)
         + String(data.pressure, 2) + "," +
         String(data.gyroX, 2) + "," +
         String(data.gyroY, 2) + "," +
         String(data.gyroZ, 2) + "," +
         String(data.accelX, 2) + "," +
         String(data.accelY, 2) + "," +
         String(data.accelZ, 2) + "," +
         String(data.verticalVelocity, 2) + "," +
         String(data.maxAltitude, 2) + "," +
         String(static_cast<int>(data.state)) + "," +
         (data.gps_valid ? String(data.gpsAltitude, 2) : "nan") + "," +
         (data.gps_valid ? String(data.latitude, 6) : "nan") + "," +
         (data.gps_valid ? String(data.longitude, 6) : "nan") + "," +
         String(data.satellites) + "," +
         String(data.parachute_deployed ? 1 : 0);
}

/**
 * @brief Monta uma linha legivel para o Serial Monitor
 */
String formatForSerial(const SensorData& data) {
  char buf[192];
  snprintf(buf, sizeof(buf),
           "[T+%lums #%u] %s | alt=%.1fm vz=%.2fm/s maxAlt=%.1fm | "
           "acc=%.2fm/s2 | GPS: %s (%u sats) | chute=%s",
           data.timestamp, data.packet_count, getFlightStateName(data.state),
           data.altitude, data.verticalVelocity, data.maxAltitude,
           data.totalAccel,
           data.gps_valid ? "fix" : "no fix", data.satellites,
           data.parachute_deployed ? "YES" : "no");
  return String(buf);
}

}  // namespace

bool initTelemetryTask() {
  if (sensorDataQueue == nullptr) {
    Serial.println(
        "[Telemetry] FATAL: sensorDataQueue not created — call "
        "initFlightControlTask() first");
    return false;
  }

  g_gps = new GPSModule(&Serial1);
  if (!g_gps->begin()) {
    Serial.println("[Telemetry] WARNING: GPS init failed — continuing without fix");
  }
  g_gps->update();  // Non-blocking best-effort read before building the filename

  if (!setupLittleFS()) {
    Serial.println("[Telemetry] FATAL: LittleFS mount failed");
    return false;
  }

  file_dir = buildDataFilePath();
  Serial.print("[Telemetry] Saving data to: ");
  Serial.println(file_dir);

  const String header =
      "TEAM_ID,millis,count,altp,temp,umi,p,gx,gy,gz,ax,ay,az,vz,"
      "maxAltitude,state,alt,lat,lon,sat,pqd";
  if (!writeFile(file_dir, header)) {
    Serial.println("[Telemetry] FATAL: failed to write CSV header");
    return false;
  }

  if (!setupLoRa()) {
    Serial.println("[Telemetry] WARNING: LoRa init failed — continuing without radio");
  }

  const BaseType_t created = xTaskCreatePinnedToCore(
      taskTelemetry, "Telemetry", TELEMETRY_STACK_SIZE, nullptr,
      TELEMETRY_PRIORITY, &g_telemetryTaskHandle, TELEMETRY_CORE);

  if (created != pdPASS) {
    Serial.println("[Telemetry] FATAL: failed to create task");
    return false;
  }

  return true;
}

void taskTelemetry(void* pvParameters) {
  (void)pvParameters;

  TickType_t lastWakeTime = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(TELEMETRY_PERIOD_MS);

  SensorData data;

  for (;;) {
    vTaskDelayUntil(&lastWakeTime, period);

    // 1) GPS (non-blocking NMEA feed)
    g_gps->update();

    // 2) Queue receive — bounded wait, never blocks indefinitely
    if (xQueueReceive(sensorDataQueue, &data, pdMS_TO_TICKS(TELEMETRY_QUEUE_TIMEOUT_MS)) == pdPASS) {
      // Enrich the sample with the current GPS fix (owned by this task only)
      data.gps_valid = g_gps->hasValidFix();
      if (data.gps_valid) {
        data.latitude    = g_gps->getLatitude();
        data.longitude   = g_gps->getLongitude();
        data.gpsAltitude = g_gps->getGPSAltitude();
      }
      data.satellites = g_gps->getSatellites();

      const String telemetry = assembleTelemetry(data);

      // 3) Multi-channel fan-out
      Serial.println(formatForSerial(data));
      sendLoRa(telemetry);
      appendFile(file_dir, telemetry);

      g_stats.packetsSent++;
    } else {
      g_stats.queueTimeoutCount++;
    }

    g_stats.cycleCount++;
  }
}

const TelemetryStats& getTelemetryStats() {
  return g_stats;
}

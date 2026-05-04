/**
 * @file telemetry_module.cpp
 * @brief Telemetry module implementation
 */

#include "telemetry_module.h"
#include "sensors/LSM6DS3Sensor.h"

int packet_count = 0;
unsigned long previous_millis = 0;

// Forward declarations for sensor objects (from firmware.ino)
extern LSM6DS3Sensor* g_lsm_sensor;

String getDataString()
{
  // Try to use LSM6DS3 if available, fallback to MPU6050 for compatibility
  String imu_data;
  if (g_lsm_sensor != nullptr && g_lsm_sensor->isReady()) {
    imu_data = g_lsm_sensor->getData();  // Use new LSM6DS3 sensor
  } else {
    imu_data = MPUData();  // Fallback to legacy MPU6050
  }
  
  return BMPData() + "," + imu_data + "," + GPSData();
}

void printBoth(const String &message, bool beep)
{
  Serial.println(message);     // Output to USB serial connection
  sendLoRa(message);           // Transmit via LoRa radio

  if (beep) {
    buzzSignal("Beep");        // Audio confirmation of low-rate events
  }
}

void logData(unsigned long current_millis, bool parachute_deployed)
{
  // Collect all sensor readings in CSV format
  String readings = getDataString();

  // Assemble complete telemetry packet with metadata
  String data_string = TEAM_ID + "," + String(current_millis) + "," +
                       String(packet_count) + "," + readings + "," +
                       parachute_deployed;

  // Transmit through multiple channels and store to filesystem
  printBoth(data_string, false);       // Send via Serial and LoRa without blocking beep
  appendFile(file_dir, data_string);   // Append to data file on LittleFS

  // Increment packet counter for next transmission
  packet_count++;
}

/**
 * @file telemetry_module.cpp
 * @brief Telemetry module implementation
 */

#include "telemetry_module.h"

int packet_count = 0;
unsigned long previous_millis = 0;

String getDataString()
{
  return BMPData() + "," + MPUData() + "," + GPSData();
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

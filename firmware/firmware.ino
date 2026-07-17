/**
 * @file firmware.ino
 * @brief Main firmware entry point for #11 Flight Computer (Avionics System)
 * 
 * This is the main program file for a rocket/drone flight computer that handles
 * sensor data collection, parachute deployment, telemetry transmission, and
 * data storage. The system uses a modular architecture with separate header files
 * for each subsystem.
 * 
 * System capabilities:
 * - Altitude and atmospheric pressure monitoring (BMP280)
 * - Inertial measurement unit for orientation (LSM6DS3 primary, MPU6050 fallback)
 * - GPS position and time tracking
 * - Long-range telemetry via LoRa radio
 * - Autonomous parachute deployment based on flight profile
 * - Data logging to onboard flash storage (LittleFS)
 * - Audio feedback via buzzer for system status
 * 
 * Flight phases:
 * 1. Initialization: System startup and sensor calibration
 * 2. Pre-launch: Monitoring and data collection on ground
 * 3. Ascent: High-frequency data logging during powered flight
 * 4. Apogee detection: Tracking maximum altitude
 * 5. Descent: Parachute deployment and controlled landing
 * 6. Recovery: Post-flight data access via storage retrieval
 * 
 * @note All configuration parameters are in config.h
 * @note Data logging interval is 200ms (5Hz) as defined by INTERVAL
 * 
 * @author #11 Avionics
 * @date 2024
 */

//==============================================================================
// LIBRARY INCLUDES
//==============================================================================

#include <Wire.h>    // I2C communication for BMP280 and MPU6050
#include <SPI.h>     // SPI communication for LoRa module

//==============================================================================
// MODULE INCLUDES
//==============================================================================

#include "config.h"             // Global configuration and constants

#include "sensors/BMP585Sensor.h"
#include "sensors/LSM6DS3Sensor.h"
#include "sensors/GPSModule.h"
#include "flight/FlightStateMachine.h"
#include "flight/FlightControlTask.h"
#include "flight/TelemetryTask.h"
#include "flight/LoggerTask.h"


#include "modules/buzzer_module.h"
#include "modules/filesystem_module.h"
#include "modules/lora_module.h"
#include "modules/parachute_module.h"


//==============================================================================
// GLOBAL SENSOR OBJECTS (v2.0 - OOP Migration)
//==============================================================================

/**
 * @brief Global instance of LSM6DS3 IMU sensor
 * 
 * This sensor is used by telemetry_module.cpp for IMU data collection.
 * If not initialized or not ready, the system falls back to legacy MPUData().
 * 
 * @see LSM6DS3Sensor class in sensors/LSM6DS3Sensor.h
 * @see telemetry_module.cpp for usage
 */

BMP585Sensor* baroSensor;
LSM6DS3Sensor* imuSensor;
GPSModule* gpsModule;
FlightStateMachine* flightFSM;

QueueHandle_t sensorDataQueue;
QueueHandle_t logQueue;

//==============================================================================
// SETUP - ONE-TIME INITIALIZATION
//==============================================================================

/**
 * @brief Initialize all system components and prepare for flight
 * 
 * This function runs once at power-on and performs the following operations:
 * 1. Initialize communication buses (Serial, I2C)
 * 2. Initialize servo motor for parachute deployment
 * 3. Startup delay for system stabilization
 * 4. Acquire GPS time for unique filename generation
 * 5. Initialize filesystem and create data file with CSV header
 * 7. Initialize all sensors (BMP280, MPU6050) and LoRa radio
 * 8. Provide audio feedback on initialization status
 * 
 * The system will restart automatically if critical initialization fails
 * (e.g., filesystem mount error). Non-critical errors (sensor failures)
 * are logged but allow the system to continue operating.
 * 
 * @note Serial monitor must be set to 115200 baud
 * @note The 5-second startup delay allows time to open Serial monitor
 * @warning System will restart on filesystem initialization failure
 * 
 * @see setup() is called automatically once by Arduino framework
 */
void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(BUZZER_PIN, OUTPUT)

  if (!initFlightControlTask()) {
    Serial.println("FATAL: FlightControl init failed");
    ESP.restart();
  }
  if (!initTelemetryTask()) {
    Serial.println("FATAL: Telemetry init failed");
    ESP.restart();
  }
  if (!initLoggerTask()) {
    Serial.println("FATAL: Logger init failed");
    ESP.restart();
  }
}

void loop() {
  vTaskDelay(portMAX_DELAY);

}

//==============================================================================
// MAIN LOOP - CONTINUOUS OPERATION
//==============================================================================

/**
 * @brief Main flight computer control loop
 * 
 * This function runs continuously after setup() completes. It performs
 * time-based sensor sampling, data logging, and parachute deployment logic
 * at regular intervals defined by INTERVAL (default: 200ms = 5Hz).
 * 
 * Operations performed each cycle:
 * 1. Check if logging interval has elapsed (200ms)
 * 2. Read current altitude from BMP280 sensor
 * 3. Calculate vertical velocity from altitude change
 * 4. Log telemetry data (all sensors + timestamp + parachute status)
 * 5. Update highest altitude reached (for apogee detection)
 * 6. Evaluate parachute deployment conditions
 * 7. Update timestamp for next cycle
 * 
 * Parachute deployment logic (see parachute_module.h for details):
 * - Must be descending from apogee (altitude drop threshold)
 * - AND either: altitude below threshold OR descent velocity exceeds threshold
 * - Once deployed, parachute remains deployed (no retraction)
 * 
  * @note Data access happens via storage retrieval workflows
 * @note GPS updates happen automatically via hardware serial interrupts
 * @note Loop frequency is controlled by INTERVAL constant in config.h
 * 
 * @see loop() is called repeatedly by Arduino framework
 * @see INTERVAL is defined in config.h (default: 200ms)
 * @see handleParachute() in parachute_module.h for deployment logic
 */

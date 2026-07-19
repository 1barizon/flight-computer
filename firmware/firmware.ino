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
 * - Altitude and atmospheric pressure monitoring (BMP585)
 * - Inertial measurement unit for orientation (LSM6DS3)
 * - GPS position and time tracking
 * - Long-range telemetry via LoRa radio
 * - Autonomous parachute deployment based on flight profile (apogee, Option A)
 * - Data logging to onboard flash storage (LittleFS)
 * - Audio feedback via buzzer for system status
 * 
 * Flight phases:
 * 1. Initialization: System startup and sensor calibration
 * 2. Pre-launch: Monitoring and data collection on ground
 * 3. Ascent: High-frequency data logging during powered flight
 * 4. Apogee detection: Tracking maximum altitude
 * 5. Descent: Parachute deployment at apogee and controlled landing
 * 6. Recovery: Post-flight data access via storage retrieval
 * 
 * @note All configuration parameters are in config.h
 * @note FlightControlTask runs at 50Hz (FLIGHT_CONTROL_PERIOD_MS);
 *       TelemetryTask and LoggerTask run at 5Hz.
 * 
 * @author Serra Rocketry
 * @date 2026
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
// SETUP - ONE-TIME INITIALIZATION
//==============================================================================

/**
 * @brief Initialize all system components and prepare for flight
 * 
 * This function runs once at power-on and delegates subsystem startup to the
 * task init functions (each owns its objects, queues and watchdog):
 *  - initFlightControlTask(): BMP585 + LSM6DS3 + FSM + sensorDataQueue + servo
 *  - initTelemetryTask(): GPS + telemetry queue consumer + LoRa/file fan-out
 *  - initLoggerTask(): logQueue + Serial/file logger
 * 
 * On any critical init failure the system restarts (ESP.restart).
 * 
 * @note Serial monitor must be set to 115200 baud
 * @note The watchdog (TWDT) is armed inside taskFlightControl once it is
 *       actually running, not here, to avoid a dangling armed watchdog.
 * 
 * @see setup() is called automatically once by Arduino framework
 */
void setup() {
  Serial.begin(115200);
  Wire.begin();
  pinMode(BUZZER_PIN, OUTPUT);

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
 * Intentionally empty: all real-time work happens in the FreeRTOS tasks
 * created by the init functions (FlightControl @50Hz, Telemetry @5Hz,
 * Logger @5Hz). The loop blocks forever on portMAX_DELAY so the Arduino
 * framework's loop() does not spin.
 * 
 * Parachute deployment (safety-critical) is decided by FlightStateMachine
 * on apogee detection (Option A) and actuated by taskFlightControl — not
 * here.
 */

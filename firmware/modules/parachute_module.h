/**
 * @file parachute_module.h
 * @brief Parachute deployment control module
 * 
 * This module manages the recovery system, controlling parachute deployment.
 * The parachute is deployed using a servo motor that releases the parachute compartment.
 * 
 * Deployment logic:
 * 1. Detects apogee (maximum altitude reached)
 * 2. Waits for descent confirmation (altitude drops below apogee)
 * 3. Checks safety conditions (altitude and velocity thresholds)
 * 4. Activates servo to deploy parachute
 * 5. Verifies deployment success
 * 
 * Safety features:
 * - Minimum altitude threshold (prevents deployment too high)
 * - Velocity threshold (confirms free fall)
 * - Apogee detection (waits until descent starts)
 * - One-time deployment (prevents multiple activations)
 * - Servo verification (checks position after actuation)
 * 
 * @author #11
 * @date 2026
 */

#ifndef PARACHUTE_MODULE_H
#define PARACHUTE_MODULE_H

#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"
#include "bmp280_sensor.h"

//==============================================================================
// PARACHUTE GLOBAL VARIABLES
//==============================================================================

/**
 * Servo motor object for parachute deployment
 * Controls mechanical release mechanism
 */
Servo ParachuteServo;

/**
 * Parachute deployment status flag
 * true: Parachute has been deployed
 * false: Parachute is still packed
 * Prevents multiple deployment attempts
 */
bool parachute_deployed = false;

/**
 * Tracks whether servo position verification is still pending after command.
 * This avoids blocking the control loop with busy-wait delays.
 */
bool parachute_verification_pending = false;

/**
 * Timestamp when parachute open command was issued.
 */
unsigned long parachute_command_millis = 0;

//==============================================================================
// FORWARD DECLARATIONS
//==============================================================================

/**
 * Forward declaration for printBoth function
 * Defined in telemetry_module.h
 * Used to send deployment status via Serial and LoRa
 */
void printBoth(const String &message);

//==============================================================================
// INITIALIZATION FUNCTIONS
//==============================================================================

/**
 * Initialize servo motor and set to closed position
 * 
 * Attaches servo to control pin and moves it to locked position (SERVO_CLOSED).
 * The 500ms delay allows servo to reach position before flight operations begin.
 *
 * Servo positions:
 * - SERVO_CLOSED (90°): Parachute compartment locked/closed
 * - SERVO_OPEN (0°): Parachute compartment released/open
 *
 * @note Call during setup() before flight operations
 * @see config.h for SERVO_PIN, SERVO_CLOSED, and SERVO_OPEN definitions
 */
bool setupServo()
{
  // Attach servo to PWM pin
  if (ParachuteServo.attach(SERVO_PIN) == 0){
    return false
  }

  // Wait for servo to initialize
  delay(500);

  // Move to closed/locked position
  ParachuteServo.write(SERVO_CLOSED);

  return true;
}

//==============================================================================
// PARACHUTE DEPLOYMENT LOGIC
//==============================================================================

/**
 * Handle parachute deployment based on altitude and velocity criteria
 * 
 * Implements multi-condition deployment logic to ensure safe parachute release:
 * 
 * Deployment conditions (ALL must be true):
 * 1. altitude <= max_altitude - ALTITUDE_DROP_THRESHOLD (descent detected)
 * 2. altitude < ALTITUDE_THRESHOLD OR abs(velocity) > VELOCITY_THRESHOLD
 *    (either below minimum altitude OR in free fall)
 * 3. !parachute_deployed (not already deployed)
 * 
 * Deployment sequence (non-blocking):
 * 1. Command servo to SERVO_OPEN (open position)
 * 2. Record command timestamp and continue loop immediately
 * 3. In subsequent calls, verify servo reached SERVO_OPEN (or timeout at 500ms)
 * 4. Report error only if timeout occurs without reaching expected position
 * 
 * After deployment:
 * - Updates previous_altitude for velocity calculation
 * - Continues to emit "Activated" buzzer signal (handled externally)
 * 
 * @param altitude Current barometric altitude in meters
 * @param velocity Current vertical velocity in m/s (negative = descending)
 * 
 * @note Velocity calculated from altitude difference between loop iterations
 * @note max_altitude is updated by checkHighest() in bmp280_sensor.h
 * @see config.h for threshold values
 * @see bmp280_sensor.h for altitude and max_altitude variables
 */
void handleParachute(float altitude, float velocity)
{
  // Non-blocking verification phase: check servo state across loop iterations
  if (parachute_verification_pending)
  {
    if (ParachuteServo.read() == SERVO_OPEN)
    {
      parachute_verification_pending = false;
    }
    else if (millis() - parachute_command_millis >= 500)
    {
      printBoth("ERROR: Servo failed to open!");
      parachute_verification_pending = false;
    }
  }

  // Check if parachute has already been deployed
  if (!parachute_deployed)
  {
    // Check all deployment conditions
    if (altitude <= max_altitude - ALTITUDE_DROP_THRESHOLD && 
        (altitude < ALTITUDE_THRESHOLD || abs(velocity) > VELOCITY_THRESHOLD))
    {
      // Command servo to open position
      ParachuteServo.write(SERVO_OPEN);

      // Start asynchronous verification without blocking the flight loop
      parachute_command_millis = millis();
      parachute_verification_pending = true;
      
      // Report deployment with telemetry
      printBoth("Parachute deployed. Altitude: " + String(altitude) + 
                " Vel: " + String(velocity));
      
      // Set flag to prevent re-deployment
      parachute_deployed = true;
    }
  }
  
  // Update previous altitude for next velocity calculation
  previous_altitude = altitude;
}

#endif // PARACHUTE_MODULE_H

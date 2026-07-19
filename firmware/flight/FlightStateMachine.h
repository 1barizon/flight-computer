/**
 * @file FlightStateMachine.h
 * @brief Flight State Machine for rocket avionics
 *
 * Implements ISensor interface for the 4-state flight FSM:
 *   IDLE → ASCENT → DESCENT → LANDED
 *
 * Detection logic is a direct port of the validated Python simulation
 * (extras/FSM_tester/FSM_Tester.py) and test sketch (test/FSM/FSM.ino).
 * All thresholds are identical to those validated with real flight data
 * (extras/FSM_tester/13_30_11-Dados.csv, 1873 data points).
 *
 * Internal sub-events (liftoff, burnout, apogee, freefall, parachute)
 * are tracked via boolean flags within the two outer states (ASCENT,
 * DESCENT), keeping the outer FSM at 4 states while preserving full
 * diagnostic resolution.
 *
 * @author #11 - Serra Rocketry
 * @date 2026-06-08
 * @version 1.0.0
 *
 * @see test/FSM/FSM.ino             — Reference implementation (validated)
 * @see extras/FSM_tester/FSM_Tester.py — Python simulation with thresholds
 * @see firmware/REFACTORING_PLAN.md — Phase 6 specification
 * @see firmware/flight/SensorData.h — FlightState enum definition
 */

#ifndef FLIGHT_STATE_MACHINE_H
#define FLIGHT_STATE_MACHINE_H

#include <Arduino.h>
#include <cmath>
#include "sensors/BMP585Sensor.h"
#include "sensors/ISensor.h"
#include "sensors/LSM6DS3Sensor.h"
#include "SensorData.h"

/**
 * @brief Flight State Machine
 *
 * Consumes BMP585Sensor (altitude, Vz) and LSM6DS3Sensor (ax, ay, az)
 * to detect flight events and drive a 4-state machine.
 *
 * Designed to run in FlightControlTask at 50 Hz (Core 1).
 * All detection methods are O(1) and non-blocking.
 */
class FlightStateMachine : public ISensor {
public:
  /**
   * @brief Construct with sensor references
   * @param baro  Initialized BMP585Sensor (must outlive this object)
   * @param imu   Initialized LSM6DS3Sensor (must outlive this object)
   */
  FlightStateMachine(BMP585Sensor* baro, LSM6DS3Sensor* imu);

  // ── ISensor interface ──────────────────────────────────────────────────────

  /**
   * @brief Validates sensor pointers; does not re-initialize hardware
   * @return true if both sensor pointers are non-null
   */
  bool begin() override;

  /**
   * @brief Reads sensors, applies filter, and advances state machine
   *
   * Must be called every cycle from FlightControlTask.
   * Silently returns if sensors are not ready or data contains NaN/Inf.
   */
  void update() override;

  /**
   * @brief Returns FSM state and event flags as a comma-separated string
   * @return e.g. "FSM:ASCENT,liftoff=1,burnout=0,apogee=0,freefall=0,parachute=0"
   */
  String getData() override;

  /**
   * @return true after a successful begin() call
   */
  bool isReady() override;

  // ── FSM accessors ─────────────────────────────────────────────────────────

  FlightState getState() const;
  const char* getStateName() const;

  // Sub-event flags (set once, never cleared until reset)
  bool isLiftoffDetected() const;
  bool isBurnoutDetected() const;
  bool isApogeeDetected() const;
  bool isFreefallDetected() const;
  bool isParachuteDeployed() const;

  /**
   * @brief Resets FSM to IDLE and clears all event flags
   *
   * Use between flights or after a watchdog reset.
   */
  void reset();

  // ── Detection thresholds (validated — do not change without re-validation) ─
  //
  // Reference: test/FSM/FSM.ino and extras/FSM_tester/FSM_Tester.py

private:
  BMP585Sensor*  _baro;
  LSM6DS3Sensor* _imu;
  FlightState    _state;
  bool           _ready;

  // Sub-event flags
  bool _liftoffDetected;
  bool _burnoutDetected;
  bool _apogeeDetected;
  bool _freefallDetected;
  bool _parachuteDeployed;

  // Parachute deploy confirmation counter (Option A: deploy on apogee +
  // stable negative Vz, never on ascent or near ground)
  uint8_t _parachuteConfirmCount;

  // IIR filter state for accelerometer (matches test/FSM/FSM.ino, ALPHA=0.2)
  float    _filtAx;
  float    _filtAy;
  float    _filtAz;
  bool     _firstReading;

  void transitionTo(FlightState next);

  // Detection helpers — exact port from test/FSM/FSM.ino
  bool detectLiftoff(float ax, float ay, float az) const;
  bool detectBurnout(float ax, float ay, float az, float height, float vz) const;
  bool detectApogee(float vz, float az) const;
  bool detectFreefall(float vz, float height, float totalAcc) const;
  bool detectParachute(float height, float vz) const;
  bool detectLanded(float vz, float height) const;

  static float smoothFilter(float value, float prev);
  static float totalAccel(float ax, float ay, float az);
};

#endif // FLIGHT_STATE_MACHINE_H

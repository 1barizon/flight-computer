/**
 * @file FlightStateMachine.cpp
 * @brief Flight State Machine implementation
 *
 * All detection logic is a direct port from test/FSM/FSM.ino (the
 * validated Arduino test sketch). Threshold values are identical to
 * those validated against real flight data in
 * extras/FSM_tester/FSM_Tester.py and extras/FSM_tester/13_30_11-Dados.csv.
 *
 * State machine:
 *   IDLE → (liftoff) → ASCENT → (apogee) → DESCENT → (landed) → LANDED
 *
 * Within ASCENT: burnout event is flagged (no state change).
 * Within DESCENT: freefall and parachute events are flagged (no state change).
 *
 * @see FlightStateMachine.h for class and threshold documentation
 * @see test/FSM/FSM.ino lines 76–117 — original detection functions
 */

#include "FlightStateMachine.h"

// ─────────────────────────────────────────────────────────────────────────────
// Constructor / lifecycle
// ─────────────────────────────────────────────────────────────────────────────

FlightStateMachine::FlightStateMachine(BMP585Sensor* baro, LSM6DS3Sensor* imu)
    : _baro(baro),
      _imu(imu),
      _state(IDLE),
      _ready(false),
      _liftoffDetected(false),
      _burnoutDetected(false),
      _apogeeDetected(false),
      _freefallDetected(false),
      _parachuteDeployed(false),
      _filtAx(0.0f),
      _filtAy(0.0f),
      _filtAz(0.0f),
      _firstReading(true),
      _stateEnteredAt(0) {}

bool FlightStateMachine::begin() {
  if (!_baro || !_imu) {
    Serial.println("[FSM] ERROR: null sensor pointer");
    return false;
  }
  _ready = true;
  Serial.println("[FSM] Ready — IDLE");
  return true;
}

bool FlightStateMachine::isReady() {
  return _ready;
}

void FlightStateMachine::reset() {
  _state            = IDLE;
  _liftoffDetected  = false;
  _burnoutDetected  = false;
  _apogeeDetected   = false;
  _freefallDetected = false;
  _parachuteDeployed = false;
  _filtAx = _filtAy = _filtAz = 0.0f;
  _firstReading = true;
  _stateEnteredAt = millis();
  Serial.println("[FSM] Reset -> IDLE");
}

// ─────────────────────────────────────────────────────────────────────────────
// Main update — called at 50 Hz from FlightControlTask
// ─────────────────────────────────────────────────────────────────────────────

void FlightStateMachine::update() {
  if (!_ready || !_baro->isReady() || !_imu->isReady()) return;

  const float height = _baro->getAltitude();
  const float vz     = _baro->getVerticalVelocity();

  float ax, ay, az;
  _imu->getAcceleration(&ax, &ay, &az);

  // Reject corrupt sensor frames before any computation
  if (!std::isfinite(height) || !std::isfinite(vz) ||
      !std::isfinite(ax)     || !std::isfinite(ay)  || !std::isfinite(az)) {
    return;
  }

  // IIR low-pass filter on accelerometer (ALPHA=0.2 — matches test/FSM/FSM.ino)
  // On first reading, seed the filter with the raw value so the filter state
  // starts at a real measurement rather than 0.
  if (_firstReading) {
    _filtAx = ax;
    _filtAy = ay;
    _filtAz = az;
    _firstReading = false;
  } else {
    _filtAx = smoothFilter(ax, _filtAx);
    _filtAy = smoothFilter(ay, _filtAy);
    _filtAz = smoothFilter(az, _filtAz);
  }

  const float acc = totalAccel(_filtAx, _filtAy, _filtAz);
  if (!std::isfinite(acc)) return;

  // ── State machine ────────────────────────────────────────────────────────

  switch (_state) {

    case IDLE:
      if (!_liftoffDetected && detectLiftoff(_filtAx, _filtAy, _filtAz)) {
        _liftoffDetected = true;
        transitionTo(ASCENT);
      }
      break;

    case ASCENT:
      // Burnout is an informational sub-event; it does not change the state.
      if (!_burnoutDetected && detectBurnout(_filtAx, _filtAy, _filtAz, height, vz)) {
        _burnoutDetected = true;
        Serial.printf("[FSM] BURNOUT  h=%.1f vz=%.2f acc=%.2f\n", height, vz, acc);
      }
      if (!_apogeeDetected && detectApogee(vz, _filtAz)) {
        _apogeeDetected = true;
        transitionTo(DESCENT);
      } else if ((millis() - _stateEnteredAt) >= STATE_TIMEOUT_MS) {
        Serial.printf("[FSM] TIMEOUT ASCENT (%.0fs) -> forcing DESCENT\n", STATE_TIMEOUT_MS / 1000.0f);
        _apogeeDetected = true;
        transitionTo(DESCENT);
      }
      break;

    case DESCENT:
      // Freefall and parachute are informational sub-events within descent.
      if (!_freefallDetected && detectFreefall(vz, height, acc)) {
        _freefallDetected = true;
        Serial.printf("[FSM] FREEFALL h=%.1f vz=%.2f acc=%.2f\n", height, vz, acc);
      }
      if (!_parachuteDeployed && detectParachute(height, vz)) {
        _parachuteDeployed = true;
        Serial.printf("[FSM] PARACHUTE DEPLOYED h=%.1f vz=%.2f\n", height, vz);
      }
      if (detectLanded(vz, height)) {
        transitionTo(LANDED);
      } else if ((millis() - _stateEnteredAt) >= STATE_TIMEOUT_MS) {
        Serial.printf("[FSM] TIMEOUT DESCENT (%.0fs) -> forcing LANDED\n", STATE_TIMEOUT_MS / 1000.0f);
        transitionTo(LANDED);
      }
      break;

    case LANDED:
      // Terminal state — awaits explicit reset()
      break;
  }
}

// ─────────────────────────────────────────────────────────────────────────────
// ISensor::getData
// ─────────────────────────────────────────────────────────────────────────────

String FlightStateMachine::getData() {
  // Format mirrors the Serial output style of test/FSM/FSM.ino
  return String("FSM:") + getStateName()
       + ",liftoff="   + (_liftoffDetected  ? "1" : "0")
       + ",burnout="   + (_burnoutDetected  ? "1" : "0")
       + ",apogee="    + (_apogeeDetected   ? "1" : "0")
       + ",freefall="  + (_freefallDetected ? "1" : "0")
       + ",parachute=" + (_parachuteDeployed ? "1" : "0");
}

// ─────────────────────────────────────────────────────────────────────────────
// Accessors
// ─────────────────────────────────────────────────────────────────────────────

FlightState FlightStateMachine::getState() const         { return _state; }
const char* FlightStateMachine::getStateName() const     { return getFlightStateName(_state); }
bool FlightStateMachine::isLiftoffDetected() const       { return _liftoffDetected; }
bool FlightStateMachine::isBurnoutDetected() const       { return _burnoutDetected; }
bool FlightStateMachine::isApogeeDetected() const        { return _apogeeDetected; }
bool FlightStateMachine::isFreefallDetected() const      { return _freefallDetected; }
bool FlightStateMachine::isParachuteDeployed() const     { return _parachuteDeployed; }

// ─────────────────────────────────────────────────────────────────────────────
// Private helpers
// ─────────────────────────────────────────────────────────────────────────────

void FlightStateMachine::transitionTo(FlightState next) {
  Serial.printf("[FSM] %s -> %s\n", getFlightStateName(_state), getFlightStateName(next));
  _state = next;
  _stateEnteredAt = millis();
}

// ── Detection functions ───────────────────────────────────────────────────────
// Each function is a 1-to-1 port of the corresponding function in
// test/FSM/FSM.ino.  Comments reference the original line numbers.

// test/FSM/FSM.ino lines 85-88
bool FlightStateMachine::detectLiftoff(float ax, float ay, float az) const {
  return totalAccel(ax, ay, az) > LIFTOFF_ACCEL_THRESHOLD;
}

// test/FSM/FSM.ino lines 90-98
bool FlightStateMachine::detectBurnout(float ax, float ay, float az,
                                        float height, float vz) const {
  if (height < BURNOUT_MIN_HEIGHT || vz <= BURNOUT_MIN_VZ) return false;
  const float acc = totalAccel(ax, ay, az);
  return (az < BURNOUT_AZ_THRESHOLD || acc < BURNOUT_ACC_THRESHOLD);
}

// test/FSM/FSM.ino lines 100-102
bool FlightStateMachine::detectApogee(float vz, float az) const {
  return (fabsf(vz) < APOGEE_MAX_VZ && az < APOGEE_AZ_THRESHOLD);
}

// test/FSM/FSM.ino lines 104-109  (az param unused in the original too)
bool FlightStateMachine::detectFreefall(float vz, float height, float totalAcc) const {
  if (height < FREEFALL_MIN_HEIGHT || vz >= FREEFALL_MAX_VZ) return false;
  return totalAcc < FREEFALL_ACC_THRESHOLD;
}

// test/FSM/FSM.ino lines 111-113
bool FlightStateMachine::detectParachute(float height, float vz) const {
  return (height <= PARACHUTE_ALTITUDE && vz < 0.0f);
}

// test/FSM/FSM.ino lines 115-117
bool FlightStateMachine::detectLanded(float vz, float height) const {
  return (fabsf(vz) < LANDED_MAX_VZ && height < LANDED_MAX_HEIGHT);
}

// ── Filter / math helpers ────────────────────────────────────────────────────

// test/FSM/FSM.ino lines 80-83  (exponential moving average, alpha=0.2)
float FlightStateMachine::smoothFilter(float value, float prev) const {
  return FILTER_ALPHA * value + (1.0f - FILTER_ALPHA) * prev;
}

// test/FSM/FSM.ino lines 76-78
float FlightStateMachine::totalAccel(float ax, float ay, float az) {
  return sqrtf(ax * ax + ay * ay + az * az);
}

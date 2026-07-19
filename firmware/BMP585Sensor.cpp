/**
 * @file BMP585Sensor.cpp
 * @brief Implementation of BMP585 barometric sensor driver
 * 
 * @see BMP585Sensor.h for class definition
 * @see firmware/REFACTORING_PLAN.md Fase 3
 */

#include "sensors/BMP585Sensor.h"

BMP585Sensor::BMP585Sensor()
    : _ready(false), _basePressure(0.0F), _altitude(0.0F), _temperature(0.0F),
      _pressure(0.0F), _maxAltitude(0.0F), _prevAltitude(0.0F), _prevTime(0UL),
      _verticalVelocity(0.0F) {}

/**
 * @brief Initializes BMP585 sensor and calibrates base pressure
 * 
 * Performs I2C communication test, first reading validation, and
 * base pressure calibration (single sample at startup).
 * 
 * @return true if initialization successful, false on error
 * @note Blocking: performs one sensor reading during calibration
 */
bool BMP585Sensor::begin() {
  if (!_bmp.begin()) {
    Serial.println("BMP585 initialization failed.");
    _ready = false;
    return false;
  }

  if (!_bmp.performReading()) {
    Serial.println("BMP585 first reading failed.");
    _ready = false;
    return false;
  }

  _pressure = _bmp.pressure / 100.0F;
  _temperature = _bmp.temperature;
  _basePressure = _pressure;
  _altitude = _bmp.readAltitude(_basePressure);

  _prevAltitude = _altitude;
  _maxAltitude = _altitude;
  _prevTime = millis();
  _verticalVelocity = 0.0F;
  _ready = true;

  return true;
}

/**
 * @brief Updates sensor readings and calculates vertical velocity
 * 
 * Non-blocking sensor read with numerical differentiation for Vz calculation.
 * Vertical velocity is clipped to ±200 m/s to reject noise spikes.
 * Invalid readings (NaN, out of range) are silently discarded,
 * preserving the last known good values as fallback.
 * 
 * @return void
 * @note Called by FlightControlTask at 50Hz
 * @note Calls checkHighest() to update max altitude
 */
void BMP585Sensor::update() {
  if (!isReady()) {
    return;
  }

  if (!_bmp.performReading()) {
    return;
  }

  const unsigned long current_time = millis();
  const float current_altitude = _bmp.readAltitude(_basePressure);

  // Validate reading — fallback to previous values on corruption
  if (isnan(current_altitude) || current_altitude < -500.0F ||
      current_altitude > 50000.0F) {
    return;
  }

  _pressure = _bmp.pressure / 100.0F;
  _temperature = _bmp.temperature;
  _altitude = current_altitude;

  const float dt = (current_time - _prevTime) / 1000.0F;
  if (dt > 0.001F) {
    float vz = (current_altitude - _prevAltitude) / dt;

    if (vz > 200.0F) {
      vz = 200.0F;
    } else if (vz < -200.0F) {
      vz = -200.0F;
    }

    _verticalVelocity = vz;
    _prevAltitude = current_altitude;
    _prevTime = current_time;
  }

  checkHighest();
}

String BMP585Sensor::getData() {
  return String(_altitude) + "," + String(_temperature) + ",nan," +
         String(_pressure);
}

bool BMP585Sensor::isReady() { return _ready; }

float BMP585Sensor::getAltitude() const { return _altitude; }

float BMP585Sensor::getPressure() const { return _pressure; }

float BMP585Sensor::getTemperature() const { return _temperature; }

float BMP585Sensor::getMaxAltitude() const { return _maxAltitude; }

float BMP585Sensor::getVerticalVelocity() const { return _verticalVelocity; }

void BMP585Sensor::checkHighest() {
  if (_altitude > _maxAltitude) {
    _maxAltitude = _altitude;
  }
}

/**
 * @file BMP585Sensor.cpp
 * @brief Implementation of BMP585 barometric sensor driver
 * 
 * @see BMP585Sensor.h for class definition
 * @see firmware/REFACTORING_PLAN.md Fase 3
 */

#include "sensors/BMP585Sensor.h"

BMP585Sensor::BMP585Sensor()
    : _ready(false), base_pressure(0.0F), altitude(0.0F), temperature(0.0F),
      pressure(0.0F), max_altitude(0.0F), prev_altitude(0.0F), prev_time(0UL),
      vertical_velocity(0.0F) {}

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

  pressure = _bmp.pressure / 100.0F;
  temperature = _bmp.temperature;
  base_pressure = pressure;
  altitude = _bmp.readAltitude(base_pressure);

  prev_altitude = altitude;
  max_altitude = altitude;
  prev_time = millis();
  vertical_velocity = 0.0F;
  _ready = true;

  return true;
}

/**
 * @brief Updates sensor readings and calculates vertical velocity
 * 
 * Non-blocking sensor read with numerical differentiation for Vz calculation.
 * Vertical velocity is clipped to ±200 m/s to reject noise spikes.
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
  const float current_altitude = _bmp.readAltitude(base_pressure);

  pressure = _bmp.pressure / 100.0F;
  temperature = _bmp.temperature;
  altitude = current_altitude;

  const float dt = (current_time - prev_time) / 1000.0F;
  if (dt > 0.001F) {
    float vz = (current_altitude - prev_altitude) / dt;

    if (vz > 200.0F) {
      vz = 200.0F;
    } else if (vz < -200.0F) {
      vz = -200.0F;
    }

    vertical_velocity = vz;
    prev_altitude = current_altitude;
    prev_time = current_time;
  }

  checkHighest();
}

String BMP585Sensor::getData() {
  return String(altitude) + "," + String(temperature) + ",nan," +
         String(pressure);
}

bool BMP585Sensor::isReady() { return _ready; }

float BMP585Sensor::getAltitude() const { return altitude; }

float BMP585Sensor::getPressure() const { return pressure; }

float BMP585Sensor::getTemperature() const { return temperature; }

float BMP585Sensor::getMaxAltitude() const { return max_altitude; }

float BMP585Sensor::getVerticalVelocity() const { return vertical_velocity; }

void BMP585Sensor::checkHighest() {
  if (altitude > max_altitude) {
    max_altitude = altitude;
  }
}

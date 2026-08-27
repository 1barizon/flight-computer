/**
 * @file BMP585Sensor.cpp
 * @brief Implementation of BMP585 barometric sensor driver
 * 
 * @see BMP585Sensor.h for class definition
 * @see firmware/REFACTORING_PLAN.md Fase 3
 */

#include "sensors/BMP585Sensor.h"
#include "config.h"

BMP585Sensor::BMP585Sensor()
    : _ready(false), _useBMP585(true), _basePressure(0.0F), _altitude(0.0F), _temperature(0.0F),
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
  // Primary: BMP585 at the bench-measured address
  if (_bmp.begin(I2C_ADDR_BMP585, &Wire)) {
    _useBMP585 = true;
  } else {
    // Fallback: BMP280 (satellite BME280Sensor.cpp pattern) — try both
    // strap-selected addresses. Sampling mirrors the satellite config.
    Serial.println("BMP585 not found, trying BMP280 fallback...");
    bool found280 = false;
    for (uint8_t addr : {I2C_ADDR_BMP280_PRIMARY, I2C_ADDR_BMP280_ALT}) {
      if (_bmp280.begin(addr)) {
        _bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,
                            Adafruit_BMP280::SAMPLING_X2,
                            Adafruit_BMP280::SAMPLING_X16,
                            Adafruit_BMP280::FILTER_X16,
                            Adafruit_BMP280::STANDBY_MS_1);  // fastest for 50 Hz loop
        _useBMP585 = false;
        found280 = true;
        Serial.println("BMP280 fallback active (backend=bmp280).");
        break;
      }
    }
    if (!found280) {
      Serial.println("BMP585 initialization failed (no BMP280 fallback either).");
      _ready = false;
      return false;
    }
  }

  if (!_firstReading()) {
    Serial.println("Barometer first reading failed.");
    _ready = false;
    return false;
  }

  _ready = true;
  return true;
}

/**
 * @brief Take the first (blocking) reading and seed the state
 *
 * Shared by both backends (BMP585 and BMP280 fallback): fills
 * pressure/temperature/altitude, calibrates base pressure, and resets
 * the Vz derivative state.
 *
 * @return true if the reading was valid
 */
bool BMP585Sensor::_firstReading() {
  if (_useBMP585) {
    if (!_bmp.performReading()) {
      return false;
    }
    _pressure = _bmp.pressure / 100.0F;
    _temperature = _bmp.temperature;
    _altitude = _bmp.readAltitude(_basePressure);
  } else {
    _pressure = _bmp280.readPressure() / 100.0F;
    _temperature = _bmp280.readTemperature();
  }

  _basePressure = _pressure;
  _altitude = _useBMP585 ? _bmp.readAltitude(_basePressure)
                         : _bmp280.readAltitude(_basePressure);

  _prevAltitude = _altitude;
  _maxAltitude = _altitude;
  _prevTime = millis();
  _verticalVelocity = 0.0F;
  return !isnan(_altitude) && !isnan(_pressure);
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

  const unsigned long current_time = millis();
  float current_altitude = NAN;

  if (_useBMP585) {
    if (!_bmp.performReading()) {
      return;
    }
    _pressure = _bmp.pressure / 100.0F;
    _temperature = _bmp.temperature;
    current_altitude = _bmp.readAltitude(_basePressure);
  } else {
    _pressure = _bmp280.readPressure() / 100.0F;
    _temperature = _bmp280.readTemperature();
    current_altitude = _bmp280.readAltitude(_basePressure);
  }

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

uint32_t BMP585Sensor::getLastReadingAgeMs() const {
  // _prevTime only advances on valid readings (update()/setBasePressure),
  // so millis() - _prevTime is the age of the last GOOD sample. UINT32_MAX
  // marks a sensor that never produced a valid reading.
  return (_ready) ? (millis() - _prevTime) : 0xFFFFFFFFUL;
}

void BMP585Sensor::setBasePressure(float basePressure) {
  if (!(basePressure > 100.0F && basePressure < 1200.0F)) {
    Serial.println("BMP585: invalid base pressure rejected");
    return;
  }
  _basePressure = basePressure;
  _altitude = _useBMP585 ? _bmp.readAltitude(_basePressure)
                         : _bmp280.readAltitude(_basePressure);

  // Validate like update(): fall back to previous value on corruption
  if (isnan(_altitude) || _altitude < -500.0F || _altitude > 50000.0F) {
    return;
  }

  // Reset derivative state so the first Vz after restore is a real reading
  _prevAltitude = _altitude;
  _prevTime = millis();
}

void BMP585Sensor::setMaxAltitude(float maxAltitude) {
  _maxAltitude = maxAltitude;
}

float BMP585Sensor::getBasePressure() const { return _basePressure; }

void BMP585Sensor::checkHighest() {
  if (_altitude > _maxAltitude) {
    _maxAltitude = _altitude;
  }
}

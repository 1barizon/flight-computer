/**
 * @file BMP585Sensor.h
 * @brief BMP585 barometric pressure sensor driver
 *
 * @see BMP585Sensor.cpp for implementation
 * @see firmware/REFACTORING_PLAN.md Phase 3
 */

#ifndef BMP585_SENSOR_H
#define BMP585_SENSOR_H

#include "ISensor.h"
#include <Adafruit_BMP5xx.h>
#include <Arduino.h>

/**
 * @brief BMP585 barometric sensor implementation
 *
 * Implements ISensor interface for BMP585 barometric sensor.
 * Calculates altitude using barometric formula and computes
 * vertical velocity via numerical differentiation.
 */
class BMP585Sensor : public ISensor {
public:
  BMP585Sensor();

  // ISensor interface
  bool begin() override;
  void update() override;
  String getData() override;
  bool isReady() override;

  // Barometer-specific getters
  float getAltitude() const;
  float getPressure() const;
  float getTemperature() const;
  float getMaxAltitude() const;
  float getVerticalVelocity() const;  // CRITICAL: Computed via numerical differentiation
  void checkHighest();

private:
  Adafruit_BMP5XX _bmp;
  bool _ready;
  float base_pressure;
  float altitude;
  float temperature;
  float pressure;
  float max_altitude;
  float prev_altitude;
  unsigned long prev_time;
  float vertical_velocity;  // Vz = (altitude_current - altitude_previous) / dt
};

#endif // BMP585_SENSOR_H

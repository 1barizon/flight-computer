/**
 * @file LSM6DS3Sensor.h
 * @brief LSM6DS3 IMU sensor driver
 *
 * @see LSM6DS3Sensor.cpp for implementation
 * @see firmware/REFACTORING_PLAN.md Phase 4
 */

#ifndef LSM6DS3SENSOR_H
#define LSM6DS3SENSOR_H

#include "ISensor.h"
#include <Adafruit_LSM6DS3.h>
#include <Adafruit_Sensor.h>
#include <Arduino.h>


/**
 * @brief LSM6DS3 IMU sensor implementation
 *
 * Implements ISensor interface for LSM6DS3 inertial measurement unit.
 * Provides 6-axis motion detection (accelerometer + gyroscope) with
 * safety validations (NaN/Inf rejection, range checking).
 */
class LSM6DS3Sensor : public ISensor {
public:
  LSM6DS3Sensor();

  // ISensor interface
  bool begin() override;
  void update() override;
  String getData() override;
  bool isReady() override;

  // IMU-specific getters
  float getAccelZ() const;
  float getTotalAccel() const;

  // Vector accessors (Issue #6 requirement)
  void getAcceleration(float* x, float* y, float* z) const {
    if (x) *x = accelX;
    if (y) *y = accelY;
    if (z) *z = accelZ;
  }

  void getGyroscope(float* x, float* y, float* z) const {
    if (x) *x = gyroX;
    if (y) *y = gyroY;
    if (z) *z = gyroZ;
  }

private:
  Adafruit_LSM6DS lsm;
  bool ready;
  float accelX, accelY, accelZ;
  float gyroX, gyroY, gyroZ;
  float total_accel;  // Computed in update()
};


#endif // LSM6DS3SENSOR_H
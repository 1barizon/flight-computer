
/**
 * @file LSM6DS3Sensor.cpp
 * @brief Implementation of LSM6DS3 IMU sensor driver
 * 
 * @see LSM6DS3Sensor.h for class definition
 * @see firmware/REFACTORING_PLAN.md Fase 4
 */

#include "sensors/LSM6DS3Sensor.h"
#include <cmath>

/**
 * @brief Default constructor - initializes all members to safe values
 */
LSM6DS3Sensor::LSM6DS3Sensor()
		: ready(false),
			accelX(0.0F),
			accelY(0.0F),
			accelZ(0.0F),
			gyroX(0.0F),
			gyroY(0.0F),
			gyroZ(0.0F),
			total_accel(0.0F) {}

/**
 * @brief Initializes LSM6DS3 IMU sensor
 * 
 * Performs I2C communication test and initial reading to validate sensor.
 * 
 * @return true if initialization successful, false on error
 * @note Blocking: performs initial sensor read
 */
bool LSM6DS3Sensor::begin() {
	if (!lsm.begin_I2C()) {
		Serial.println("LSM6DS3 initialization failed.");
		ready = false;
		return false;
	}

	sensors_event_t accel_event;
	sensors_event_t gyro_event;
	sensors_event_t temp_event;
	lsm.getEvent(&accel_event, &gyro_event, &temp_event);

	accelX = accel_event.acceleration.x;
	accelY = accel_event.acceleration.y;
	accelZ = accel_event.acceleration.z;
	gyroX = gyro_event.gyro.x;
	gyroY = gyro_event.gyro.y;
	gyroZ = gyro_event.gyro.z;

	total_accel = std::sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);
	ready = true;

	return true;
}

/**
 * @brief Updates IMU readings with safety validations
 * 
 * Non-blocking sensor read with comprehensive validation:
 * 1. NaN/Inf rejection for safety-critical FSM
 * 2. Range validation (200 m/s² accel, 2000 °/s gyro)
 * Corrupted samples are silently dropped to prevent FSM errors.
 * 
 * @return void
 * @note Called by FlightControlTask at 50Hz
 * @note CRITICAL: Validates data before updating internal state
 */
void LSM6DS3Sensor::update() {
	if (!isReady()) {
		return;
	}

	sensors_event_t accel_event;
	sensors_event_t gyro_event;
	sensors_event_t temp_event;
	lsm.getEvent(&accel_event, &gyro_event, &temp_event);

	const float newAccelX = accel_event.acceleration.x;
	const float newAccelY = accel_event.acceleration.y;
	const float newAccelZ = accel_event.acceleration.z;
	const float newGyroX = gyro_event.gyro.x;
	const float newGyroY = gyro_event.gyro.y;
	const float newGyroZ = gyro_event.gyro.z;

	// Validation 1: Check for NaN/Inf (critical for safety)
	if (!std::isfinite(newAccelX) || !std::isfinite(newAccelY) || !std::isfinite(newAccelZ) ||
			!std::isfinite(newGyroX) || !std::isfinite(newGyroY) || !std::isfinite(newGyroZ)) {
		return;
	}

	// Validation 2: Check realistic ranges
	// LSM6DS3 typical range: ±16g (±156.96 m/s²) for accel, ±2000 °/s for gyro
	// Allows slightly higher peaks (200 m/s², 2000 °/s) for extreme events
	const float MAX_ACCEL = 200.0f;   // m/s² (allows 20g peaks)
	const float MAX_GYRO = 2000.0f;   // °/s (matches sensor range)
	
	if (std::abs(newAccelX) > MAX_ACCEL ||
	    std::abs(newAccelY) > MAX_ACCEL ||
	    std::abs(newAccelZ) > MAX_ACCEL) {
		return;  // Drop corrupted sample to prevent FSM corruption
	}
	
	if (std::abs(newGyroX) > MAX_GYRO ||
	    std::abs(newGyroY) > MAX_GYRO ||
	    std::abs(newGyroZ) > MAX_GYRO) {
		return;  // Drop corrupted sample
	}

	// Accept sample after validation passes
	accelX = newAccelX;
	accelY = newAccelY;
	accelZ = newAccelZ;
	gyroX = newGyroX;
	gyroY = newGyroY;
	gyroZ = newGyroZ;

	total_accel = std::sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);
}

String LSM6DS3Sensor::getData() {
	return String(accelX) + "," + String(accelY) + "," + String(accelZ) + "," +
				 String(gyroX) + "," + String(gyroY) + "," + String(gyroZ) + "," +
				 String(total_accel);
}

bool LSM6DS3Sensor::isReady() {
	return ready;
}

float LSM6DS3Sensor::getAccelZ() const {
	return accelZ;
}

float LSM6DS3Sensor::getTotalAccel() const {
	return total_accel;
}



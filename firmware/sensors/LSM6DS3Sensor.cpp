
#include "LSM6DS3Sensor.h"

LSM6DS3Sensor::LSM6DS3Sensor()
		: ready(false),
			accelX(0.0F),
			accelY(0.0F),
			accelZ(0.0F),
			gyroX(0.0F),
			gyroY(0.0F),
			gyroZ(0.0F),
			total_accel(0.0F) {}

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

	total_accel = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);
	ready = true;

	return true;
}

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

	if (!isfinite(newAccelX) || !isfinite(newAccelY) || !isfinite(newAccelZ) ||
			!isfinite(newGyroX) || !isfinite(newGyroY) || !isfinite(newGyroZ)) {
		return;
	}

	accelX = newAccelX;
	accelY = newAccelY;
	accelZ = newAccelZ;
	gyroX = newGyroX;
	gyroY = newGyroY;
	gyroZ = newGyroZ;

	total_accel = sqrt(accelX * accelX + accelY * accelY + accelZ * accelZ);
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



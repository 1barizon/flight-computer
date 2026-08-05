/**
 * @file config.h
 * @brief Global configuration file for the avionics system
 * 
 * This file contains all pin definitions, constants, and parameters
 * used throughout the system. Centralizing configurations here makes
 * maintenance and parameter modifications easier.
 * 
 * @author #11
 * @date 2026
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

//==============================================================================
// LORA COMMUNICATION CONFIGURATION
//==============================================================================

/**
 * LoRa module operating frequency in Hz
 * 915 MHz is the ISM frequency for Brazil/Americas (matches receiver-lora).
 * (868 MHz is Europe — do not use here.)
 */
#define LORA_FREQ 915E6

/**
 * Slave Select (SS/CS) pin for SPI communication with LoRa module (RFM95W)
 * @note The LoRa 0.8.0 library uses the global SPI object. We remap the SPI
 *       bus to these pins via SPI.begin(SCK, MISO, MOSI, SS) in setupLoRa()
 *       (same approach as the receiver-lora firmware, which compiles/runs
 *       clean). Only CS/RST/DIO0 are passed to LoRa.setPins().
 */
#define LORA_SCK  4
#define LORA_MISO 2
#define LORA_MOSI 3
#define SS_LORA 5

/**
 * Reset pin for LoRa module
 */
#define RST_LORA 6

/**
 * DIO0 pin of LoRa module (IRQ / interrupt)
 */
#define DIO0_LORA 7

/**
 * Synchronization word for LoRa communication
 * Ensures only devices with the same word communicate
 * Change this value to create isolated LoRa networks
 */
#define SYNC_WORD 0xF3

// Spreading factor / bandwidth / coding rate / TX power.
// Explicitly matched to the receiver (recovery-webui/components/receiver-lora)
// so the link connects. These are also the LoRa.h defaults, but we set them
// explicitly to avoid relying on library defaults.
#define LORA_SF       7      // Spreading Factor 7–12
#define LORA_BW       125E3  // Bandwidth Hz
#define LORA_CR       5      // Coding Rate (4/5)
#define LORA_TX_POWER 17     // dBm

//==============================================================================
// PIN DEFINITIONS - SD CARD (SPI, shares bus with LoRa)
//==============================================================================

/**
 * Chip Select pin for SD card module
 * Shares the SPI bus with LoRa (SCK=4, MISO=2, MOSI=3).
 * GPIO 12 is free on ESP32-C3 SuperMini (not used by I2C, UART, or LoRa).
 * @note If SD fails, data is saved to LittleFS (internal flash) automatically.
 */
#define SD_CS_PIN 12

/**
 * Flush file buffer every N samples when using SD card
 * Balances data safety vs. write endurance.
 */
static constexpr uint8_t FLUSH_EVERY_N = 10;

//==============================================================================
// PIN DEFINITIONS - ACTUATORS
//==============================================================================

/**
 * Digital pin connected to parachute servo motor
 * Uses PWM to control servo position.
 * @note GPIO 10 — free on ESP32-S3 (not a strap, not used by SPI/I2C/LoRa/GPS).
 *       Adjust to match the final schematic.
 */
#define SERVO_PIN 10

/**
 * Digital pin connected to piezoelectric buzzer
 * Emits sound signals for status indication.
 * @note GPIO 11 — free on ESP32-S3 (not a strap, not used by SPI/I2C/LoRa/GPS).
 *       Avoid GPIO0 (boot strap). Adjust to match the final schematic.
 */
#define BUZZER_PIN 11

//==============================================================================
// PIN DEFINITIONS - I2C (SENSORS: BMP585 barometer, LSM6DS3 IMU)
//==============================================================================

/**
 * I2C data pin (SDA) for the sensor bus.
 * @note ESP32-S3 Arduino core default is SDA=8 / SCL=9. The BMP585 and
 *       LSM6DS3 drivers call begin_I2C() with no pins, so they use this
 *       default. Wire.begin() below also uses these. Keep the schematic
 *       wired to 8/9 (or change both here and the calls).
 */
#define I2C_SDA 8

/**
 * I2C clock pin (SCL) for the sensor bus.
 */
#define I2C_SCL 9

//==============================================================================
// PIN DEFINITIONS - GPS
//==============================================================================

/**
 * RX pin for serial communication with GPS module
 * Connects to GPS module TX
 */
#define RX_GPS 20

/**
 * TX pin for serial communication with GPS module
 * Connects to GPS module RX
 */
#define TX_GPS 21

//==============================================================================
// PARACHUTE CONTROL CONSTANTS
//==============================================================================

/**
 * Servo position: parachute open
 * Value in degrees: 0° = fully open
 */
const int SERVO_OPEN = 0;

/**
 * Servo position: parachute closed
 * Value in degrees: 90° = fully closed
 */
const int SERVO_CLOSED = 90;

//==============================================================================
// TEAM IDENTIFICATION
//==============================================================================

/**
 * Team unique identifier
 * Used as prefix in all telemetry transmissions
 * Allows identifying data from different teams
 */
constexpr const char* TEAM_ID = "#100";

//==============================================================================
// FILESYSTEM CONFIGURATION
//==============================================================================

/**
 * CSV data file name
 * Stores all telemetry readings
 */
extern String file_name;

/**
 * Complete data file path
 * Will be filled during setup() with GPS timestamp
 * Format: /HH_MM_SS-Dados.csv
 */
extern String file_dir;


static constexpr float LIFTOFF_ACCEL_THRESHOLD  = 15.0f;  ///< m/s²  total accel
static constexpr float BURNOUT_AZ_THRESHOLD     = -8.0f;  ///< m/s²  vertical accel
static constexpr float BURNOUT_ACC_THRESHOLD    =  2.0f;  ///< m/s²  total accel
static constexpr float BURNOUT_MIN_HEIGHT       =  5.0f;  ///< m     minimum altitude
static constexpr float BURNOUT_MIN_VZ           =  0.5f;  ///< m/s   minimum climb speed
static constexpr float APOGEE_MAX_VZ            =  1.0f;  ///< m/s   |vz| below this
static constexpr float APOGEE_AZ_THRESHOLD      = -0.1f;  ///< m/s²  az below this
static constexpr float FREEFALL_ACC_THRESHOLD   = 11.5f;  ///< m/s²  total accel
static constexpr float FREEFALL_MIN_HEIGHT      =  5.0f;  ///< m     minimum altitude
static constexpr float FREEFALL_MAX_VZ          = -5.0f;  ///< m/s   vz must be below this

// ── Free-fall backstop (FSM-independent safety net) ─────────────────────────
// Detects a real free fall WITHOUT relying on the FSM state. Runs in the
// FlightControlTask loop; fires only when total accel (IIR-filtered) stays
// below near-zero-g for a full second WHILE descending fast and well above
// the ground guard. The vz < -5 m/s condition is what separates real descent
// from burnout/coasting (where vz is still positive) — without it the chute
// would deploy on ascent right after motor cutoff (accel dips to ~0).
// Sized with real flight data: zero-g windows last 8-131s, pad vibration
// spikes (22-122 m/s²) are transient so the 1s window rejects them.
static constexpr float   FREEFALL_BACKSTOP_ACC_THRESHOLD = 3.0f;    ///< m/s²  near zero-g (≈0.3g)
static constexpr float   FREEFALL_BACKSTOP_VZ            = -5.0f;   ///< m/s   must be descending this fast
static constexpr float   FREEFALL_BACKSTOP_MIN_HEIGHT    = 50.0f;   ///< m     ground guard (same as PARACHUTE_MIN_ALTITUDE)
static constexpr uint16_t FREEFALL_BACKSTOP_CYCLES       = 50;      ///< consecutive cycles = 1.0s at 50Hz (FLIGHT_CONTROL_PERIOD_MS=20)
static constexpr float PARACHUTE_MIN_ALTITUDE    = 50.0f;  ///< m  minimum altitude (ground guard — never deploy below)
static constexpr float PARACHUTE_CONFIRM_VZ      = -2.0f;  ///< m/s negative Vz required to confirm descent after apogee
static constexpr uint8_t PARACHUTE_CONFIRM_CYCLES = 3;     ///< consecutive cycles of (vz < CONFIRM_VZ) before deploy
static constexpr float LANDED_MAX_VZ            =  0.5f;  ///< m/s   |vz| below this
static constexpr float LANDED_MAX_HEIGHT        =  2.0f;  ///< m     altitude below this
static constexpr float FILTER_ALPHA             =  0.2f;  ///< IIR low-pass coefficient

#endif // CONFIG_H

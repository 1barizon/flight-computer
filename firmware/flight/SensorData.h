/**
 * @file SensorData.h
 * @brief Estruturas de dados compartilhadas entre tasks
 * 
 * Define as estruturas de dados usadas para comunicacao entre tasks
 * via FreeRTOS queues:
 * - SensorData: Dados dos sensores (BMP585, LSM6DS3, GPS)
 * - LogMessage: Mensagens de log para a task logger
 * 
 * @author Team #100 - Serra Rocketry
 * @date 2026-04-06
 * @version 1.0.0
 */

#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <Arduino.h>

/**
 * @brief Estados da maquina de estados de voo
 * 
 * Representa os 7 estados do ciclo de voo do foguete,
 * validados com dados reais em extras/FSM_tester/13_30_11-Dados.csv
 */
enum FlightState {
  IDLE = 0,       ///< Pré-lançamento, aguardando no solo
  LIFTOFF = 1,    ///< Subida motorizada, alta aceleração
  BURNOUT = 2,    ///< Costa balística (sem motor)
  APOGEE = 3,     ///< Altitude máxima atingida
  FREEFALL = 4,   ///< Queda rápida pós-apogeu
  PARACHUTE = 5,  ///< Descida controlada com paraquedas
  LANDED = 6      ///< Pouso detectado, fim do voo
};

/**
 * @brief Converte enum FlightState para string
 * 
 * @param state Estado de voo
 * @return const char* Nome do estado
 */
inline const char* getFlightStateName(FlightState state) {
  switch (state) {
    case IDLE:      return "IDLE";
    case LIFTOFF:   return "LIFTOFF";
    case BURNOUT:   return "BURNOUT";
    case APOGEE:    return "APOGEE";
    case FREEFALL:  return "FREEFALL";
    case PARACHUTE: return "PARACHUTE";
    case LANDED:    return "LANDED";
    default:        return "UNKNOWN";
  }
}

/**
 * @brief Estrutura de dados dos sensores
 * 
 * Contém leituras de todos os sensores (BMP585, LSM6DS3, GPS) e
 * estado da maquina de estados. Enviada pela FlightControlTask
 * para a TelemetryTask via sensorDataQueue.
 * 
 * Tamanho: ~64 bytes
 * Queue: 25 slots = ~1.6KB RAM
 */
struct SensorData {
  // === TIMESTAMP ===
  unsigned long timestamp;    ///< Timestamp em millisegundos
  uint16_t packet_count;      ///< Numero sequencial do pacote
  
  // === BMP585 BAROMETER ===
  float altitude;             ///< Altitude em metros (relativa ao launchpad)
  float pressure;             ///< Pressao em hPa
  float temperature;          ///< Temperatura em °C
  float verticalVelocity;     ///< Velocidade vertical (Vz) em m/s
  float maxAltitude;          ///< Altitude maxima atingida em metros
  
  // === LSM6DS3 IMU ===
  float accelX, accelY, accelZ;   ///< Aceleracao em m/s²
  float gyroX, gyroY, gyroZ;      ///< Velocidade angular em °/s
  float totalAccel;               ///< Magnitude da aceleracao total em m/s²
  
  // === GPS (OPTIONAL) ===
  double latitude, longitude;     ///< Coordenadas GPS
  float gpsAltitude;              ///< Altitude do GPS em metros
  uint8_t satellites;             ///< Numero de satelites rastreados
  bool gps_valid;                 ///< True se GPS tem fix valido
  
  // === FSM STATE ===
  FlightState state;              ///< Estado atual do voo
  bool parachute_deployed;        ///< True se paraquedas foi desdobrado
};

/**
 * @brief Estrutura de mensagem de log
 * 
 * Enviada por qualquer task para a LoggerTask via logQueue.
 * Permite logging thread-safe com nivels de severidade.
 * 
 * Tamanho: ~140 bytes
 * Queue: 50 slots = ~7KB RAM
 */
struct LogMessage {
  char message[128];          ///< Mensagem de log (max 127 chars + null terminator)
  unsigned long timestamp;    ///< Timestamp em millisegundos
  uint8_t taskId;             ///< ID da task que enviou (1=FSM, 2=Telemetry, 3=Logger)
  uint8_t level;              ///< Nivel de severidade (0=DEBUG, 1=INFO, 2=WARN, 3=ERROR)
};

/**
 * @brief Converte nivel de log para string
 * 
 * @param level Nivel de log
 * @return const char* Nome do nivel
 */
inline const char* getLogLevelName(uint8_t level) {
  switch (level) {
    case 0: return "[DEBUG]";
    case 1: return "[INFO]";
    case 2: return "[WARN]";
    case 3: return "[ERROR]";
    default: return "[???]";
  }
}

#endif // SENSOR_DATA_H

/**
 * @file ISensor.h
 * @brief Interface abstrata para sensores do flight computer
 * 
 * Define a interface padrão que todos os sensores devem implementar,
 * permitindo abstração e polimorfismo para BMP585, LSM6DS3, GPS, etc.
 * 
 * @author Team #100 - Serra Rocketry
 * @date 2026-04-06
 * @version 1.0.0
 * 
 * @see firmware/REFACTORING_PLAN.md - Arquitetura completa
 * @see firmware/sensors/BMP585Sensor.h - Exemplo de implementação
 */

#ifndef ISENSOR_H
#define ISENSOR_H

#include <Arduino.h>

/**
 * @brief Interface abstrata para sensores do flight computer
 * 
 * Todos os sensores (barometro, IMU, GPS, etc.) devem herdar desta classe
 * e implementar os metodos definidos aqui.
 * 
 * Exemplo de uso:
 * @code
 * class BMP585Sensor : public ISensor {
 *   bool begin() override { /* ... */ }
 *   void update() override { /* ... */ }
 *   String getData() override { /* ... */ }
 *   bool isReady() override { /* ... */ }
 * };
 * @endcode
 */
class ISensor {
public:
  /**
   * @brief Destrutor virtual (necessario para polimorfismo)
   * 
   * Permite que objetos derivados sejam deletados via ponteiro base
   * sem vazamento de memoria.
   */
  virtual ~ISensor() = default;
  
  /**
   * @brief Inicializa o sensor
   * 
   * Deve ser chamado na setup() da aplicacao, antes de qualquer update().
   * Pode ser bloqueante (durante calibracao, por exemplo).
   * 
   * @return true se inicializacao bem-sucedida, false em caso de erro
   *         (sensor nao encontrado, comunicacao falhou, etc.)
   */
  virtual bool begin() = 0;
  
  /**
   * @brief Atualiza as leituras do sensor
   * 
   * Esta funcao deve ser **nao-bloqueante** e chamada regularmente
   * pelo task que gerencia o sensor. Para BMP585 e LSM6DS3, deve ser
   * chamada em FlightControlTask a 50Hz. Para GPS, a 5Hz em TelemetryTask.
   */
  virtual void update() = 0;
  
  /**
   * @brief Retorna uma string com os dados do sensor (para Serial/logging)
   * 
   * @return String com dados formatados do sensor (ex: "BMP585: 1234.5m, 101.3hPa")
   */
  virtual String getData() = 0;
  
  /**
   * @brief Verifica se o sensor esta pronto para uso
   * 
   * @return true se sensor esta pronto (inicializado e operacional),
   *         false caso contrario (nao inicializado, erro de I2C, etc.)
   */
  virtual bool isReady() = 0;
};

#endif // ISENSOR_H

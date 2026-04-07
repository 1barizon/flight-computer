/**
 * @file ISensor.h
 * @brief Interface abstrata para sensores do flight computer
 * 
 * Define a interface padrão que todos os sensores devem implementar,
 * permitindo abstração e polimorfismo.
 * 
 * @author Team #100 - Serra Rocketry
 * @date 2026-04-06
 * @version 1.0.0
 */

#ifndef ISENSOR_H
#define ISENSOR_H

#include <Arduino.h>

/**
 * @brief Interface abstrata para sensores do flight computer
 * 
 * Todos os sensores (barometro, IMU, GPS, etc.) devem herdar desta classe
 * e implementar os metodos definidos aqui.
 */
class ISensor {
public:
  /**
   * @brief Destrutor virtual (necessario para polimorfismo)
   */
  virtual ~ISensor() = default;
  
  /**
   * @brief Inicializa o sensor
   * 
   * @return true se inicializacao bem-sucedida, false em caso de erro
   */
  virtual bool begin() = 0;
  
  /**
   * @brief Atualiza as leituras do sensor
   * 
   * Esta funcao deve ser nao-bloqueante e chamada regularmente
   * pelo task que gerencia o sensor.
   */
  virtual void update() = 0;
  
  /**
   * @brief Retorna uma string com os dados do sensor (para Serial/logging)
   * 
   * @return String com dados formatados do sensor
   */
  virtual String getData() = 0;
  
  /**
   * @brief Verifica se o sensor esta pronto para uso
   * 
   * @return true se sensor esta pronto (inicializado), false caso contrario
   */
  virtual bool isReady() = 0;
};

#endif // ISENSOR_H

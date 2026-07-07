#ifndef ADS1115_H
#define ADS1115_H

#include <stdbool.h>
#include <stdint.h>
#include "esp_err.h"

// Dirección I2C predeterminada del ADS1115 (pin ADDR conectado a GND)
#define ADS1115_ADDR_DEFAULT 0x48

// Inicializa el ADS1115 en el bus I2C
bool ads1115_init(void);

// Lee el valor analógico crudo (16 bits) de un canal específico (0 a 3)
bool ads1115_read_raw(uint8_t channel, int16_t *raw_val);

// Convierte la lectura cruda a voltaje en voltios (asumiendo FSR de ±4.096V)
float ads1115_raw_to_voltage(int16_t raw_val);

#endif // ADS1115_H

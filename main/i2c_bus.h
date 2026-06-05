#ifndef I2C_BUS_H
#define I2C_BUS_H

#include <stddef.h>
#include <stdint.h>

#include "driver/i2c.h"
#include "esp_err.h"

#define I2C_BUS_PORT  I2C_NUM_0
#define I2C_BUS_SDA_GPIO 6
#define I2C_BUS_SCL_GPIO 7

void i2c_bus_init_if_needed(void);
esp_err_t i2c_bus_read_u8(uint8_t addr, uint8_t reg, uint8_t *value);
esp_err_t i2c_bus_read(uint8_t addr, uint8_t reg, uint8_t *buffer, size_t len);
esp_err_t i2c_bus_write_u8(uint8_t addr, uint8_t reg, uint8_t value);
esp_err_t i2c_bus_write_byte(uint8_t addr, uint8_t value);

#endif // I2C_BUS_H

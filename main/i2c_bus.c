#include <stdbool.h>

#include "driver/i2c.h"
#include "esp_log.h"

#include "i2c_bus.h"

static const char *TAG = "i2c_bus";

#define I2C_BUS_FREQ_HZ 100000
#define I2C_BUS_TIMEOUT_MS 100

static bool i2c_bus_initialized = false;

void i2c_bus_init_if_needed(void)
{
    if (i2c_bus_initialized) {
        return;
    }

    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_BUS_SDA_GPIO,
        .scl_io_num = I2C_BUS_SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_BUS_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_BUS_PORT, &config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_BUS_PORT, config.mode, 0, 0, 0));

    i2c_bus_initialized = true;
    ESP_LOGI(TAG, "Bus I2C listo en SDA=GPIO%d, SCL=GPIO%d.", I2C_BUS_SDA_GPIO, I2C_BUS_SCL_GPIO);
}

esp_err_t i2c_bus_read_u8(uint8_t addr, uint8_t reg, uint8_t *value)
{
    return i2c_master_write_read_device(
        I2C_BUS_PORT,
        addr,
        &reg,
        sizeof(reg),
        value,
        1,
        pdMS_TO_TICKS(I2C_BUS_TIMEOUT_MS));
}

esp_err_t i2c_bus_read(uint8_t addr, uint8_t reg, uint8_t *buffer, size_t len)
{
    return i2c_master_write_read_device(
        I2C_BUS_PORT,
        addr,
        &reg,
        sizeof(reg),
        buffer,
        len,
        pdMS_TO_TICKS(I2C_BUS_TIMEOUT_MS));
}

esp_err_t i2c_bus_write_u8(uint8_t addr, uint8_t reg, uint8_t value)
{
    uint8_t payload[2] = {reg, value};

    return i2c_master_write_to_device(
        I2C_BUS_PORT,
        addr,
        payload,
        sizeof(payload),
        pdMS_TO_TICKS(I2C_BUS_TIMEOUT_MS));
}

esp_err_t i2c_bus_write_byte(uint8_t addr, uint8_t value)
{
    return i2c_master_write_to_device(
        I2C_BUS_PORT,
        addr,
        &value,
        sizeof(value),
        pdMS_TO_TICKS(I2C_BUS_TIMEOUT_MS));
}

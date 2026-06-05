#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "bh1750.h"
#include "i2c_bus.h"

static const char *TAG = "bh1750";

#define BH1750_ADDR_LOW 0x23
#define BH1750_ADDR_HIGH 0x5C
#define BH1750_POWER_ON 0x01
#define BH1750_CONT_HIGH_RES_MODE 0x10

static bool bh1750_initialized = false;
static uint8_t bh1750_addr = 0;

static bool bh1750_try_init_addr(uint8_t addr)
{
    if (i2c_bus_write_byte(addr, BH1750_POWER_ON) != ESP_OK) {
        return false;
    }

    if (i2c_bus_write_byte(addr, BH1750_CONT_HIGH_RES_MODE) != ESP_OK) {
        return false;
    }

    vTaskDelay(pdMS_TO_TICKS(180));
    bh1750_addr = addr;
    return true;
}

bool bh1750_init(void)
{
    i2c_bus_init_if_needed();

    if (bh1750_try_init_addr(BH1750_ADDR_LOW)) {
        bh1750_initialized = true;
        ESP_LOGI(TAG, "BH1750 detectado correctamente en 0x%02X.", bh1750_addr);
        return true;
    }

    if (bh1750_try_init_addr(BH1750_ADDR_HIGH)) {
        bh1750_initialized = true;
        ESP_LOGI(TAG, "BH1750 detectado correctamente en 0x%02X.", bh1750_addr);
        return true;
    }

    ESP_LOGW(TAG, "No pude inicializar el BH1750 por I2C.");
    ESP_LOGW(TAG, "Revisa VCC/GND y que SDA/SCL compartan el mismo bus del BMP280.");
    return false;
}

bool bh1750_read_lux(float *lux)
{
    uint8_t raw[2] = {0};
    uint16_t raw_lux = 0;

    if (!bh1750_initialized || lux == NULL) {
        return false;
    }

    if (i2c_master_read_from_device(
            I2C_BUS_PORT,
            bh1750_addr,
            raw,
            sizeof(raw),
            pdMS_TO_TICKS(100)) != ESP_OK) {
        return false;
    }

    raw_lux = (uint16_t)((raw[0] << 8) | raw[1]);
    *lux = raw_lux / 1.2f;
    return true;
}

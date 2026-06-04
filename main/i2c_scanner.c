#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "i2c_scanner.h"

static const char *TAG = "i2c_scanner";

#define I2C_SCANNER_PORT I2C_NUM_0
#define I2C_SCANNER_SDA_GPIO 6
#define I2C_SCANNER_SCL_GPIO 7
#define I2C_SCANNER_FREQ_HZ 100000
#define I2C_SCANNER_TIMEOUT_MS 50
#define I2C_SCANNER_FIRST_ADDR 0x03
#define I2C_SCANNER_LAST_ADDR 0x77

static bool i2c_scanner_iniciado = false;

static void i2c_scanner_init_if_needed(void)
{
    if (i2c_scanner_iniciado) {
        return;
    }

    i2c_config_t config = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SCANNER_SDA_GPIO,
        .scl_io_num = I2C_SCANNER_SCL_GPIO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_SCANNER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(i2c_param_config(I2C_SCANNER_PORT, &config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_SCANNER_PORT, config.mode, 0, 0, 0));

    i2c_scanner_iniciado = true;
}

static bool i2c_address_responds(uint8_t addr)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    esp_err_t ret = ESP_FAIL;

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_stop(cmd);

    ret = i2c_master_cmd_begin(
        I2C_SCANNER_PORT,
        cmd,
        pdMS_TO_TICKS(I2C_SCANNER_TIMEOUT_MS));

    i2c_cmd_link_delete(cmd);
    return ret == ESP_OK;
}

static void i2c_scan_once(void)
{
    int encontrados = 0;

    ESP_LOGI(TAG, "Escaneando bus I2C en SDA=GPIO%d, SCL=GPIO%d...", I2C_SCANNER_SDA_GPIO, I2C_SCANNER_SCL_GPIO);

    for (uint8_t addr = I2C_SCANNER_FIRST_ADDR; addr <= I2C_SCANNER_LAST_ADDR; addr++) {
        if (i2c_address_responds(addr)) {
            ESP_LOGI(TAG, "Dispositivo I2C detectado en 0x%02X", addr);
            encontrados++;
        }
    }

    if (encontrados == 0) {
        ESP_LOGW(TAG, "No encontre ningun dispositivo I2C.");
        ESP_LOGW(TAG, "Conecta solo VCC, GND, SDA y SCL; luego revisa si el sensor necesita tambien CSB/SDO para entrar en I2C.");
    } else {
        ESP_LOGI(TAG, "Escaneo terminado. Dispositivos detectados: %d", encontrados);
    }
}

void i2c_scanner_run_forever(void)
{
    i2c_scanner_init_if_needed();

    while (1) {
        i2c_scan_once();
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

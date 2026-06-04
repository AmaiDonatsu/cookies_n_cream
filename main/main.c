#include "esp_log.h"
#include "i2c_scanner.h"

static const char *TAG = "cookies_n_cream";

void app_main(void)
{
    ESP_LOGI(TAG, "Modo debug: escaner I2C en GPIO6/GPIO7.");
    ESP_LOGI(TAG, "Conecta por ahora solo VCC, GND, SDA y SCL para validar el bus.");
    i2c_scanner_run_forever();
}

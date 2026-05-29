#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bmp280_probe.h"

static const char *TAG = "cookies_n_cream";

void app_main(void)
{
    float temperature_c = 0.0f;

    ESP_LOGI(TAG, "Modo debug limpio: solo prueba BMP280 por I2C.");
    ESP_LOGI(TAG, "Wi-Fi desactivado temporalmente para aislar el problema del sensor.");

    if (!bmp280_init()) {
        while (1) {
            ESP_LOGW(TAG, "BMP280 no disponible. Revisa cableado y reinicia para reintentar.");
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    while (1) {
        if (bmp280_read_temperature_c(&temperature_c)) {
            ESP_LOGI(TAG, "Temperatura BMP280: %.2f C", temperature_c);
        } else {
            ESP_LOGW(TAG, "No pude leer la temperatura del BMP280 en este ciclo.");
        }

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

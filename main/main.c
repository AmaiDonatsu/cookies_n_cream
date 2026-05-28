#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "wifi_config.h"

static const char *TAG = "cookies_n_cream";

void app_main(void)
{
    wifi_init_sta();

    int counter = 0;

    ESP_LOGI(TAG, "ESP-IDF listo. Si lees esto por UART, ya tenemos toolchain + flash + monitor funcionando.");

    while (1) {
        if (wifi_esta_conectado()) {
            ESP_LOGI(TAG, "Heartbeat %d", counter++);
        } else {
            ESP_LOGI(TAG, "Esperando conexion Wi-Fi...");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

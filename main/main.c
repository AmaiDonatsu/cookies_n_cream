#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "cookies_n_cream";

void app_main(void)
{
    int counter = 0;

    ESP_LOGI(TAG, "ESP-IDF listo. Si lees esto por UART, ya tenemos toolchain + flash + monitor funcionando.");

    while (1) {
        ESP_LOGI(TAG, "Heartbeat %d", counter++);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

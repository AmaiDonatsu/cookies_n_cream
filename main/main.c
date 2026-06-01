#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "bmp280_probe.h"
#include "mqtt_config.h"
#include "mqtt_telemetry.h"
#include "time_sync.h"
#include "wifi_config.h"

static const char *TAG = "cookies_n_cream";

void app_main(void)
{
    float temperature_c = 0.0f;
    bool mqtt_started = false;
    bool clock_ready = false;

    ESP_LOGI(TAG, "Iniciando estacion Wi-Fi + BMP280 + telemetria MQTT.");
    ESP_LOGI(TAG, "Device ID MQTT: %s", MQTT_DEVICE_ID);

    if (!bmp280_init()) {
        while (1) {
            ESP_LOGW(TAG, "BMP280 no disponible. Revisa cableado y reinicia para reintentar.");
            vTaskDelay(pdMS_TO_TICKS(5000));
        }
    }

    wifi_init_sta();

    while (1) {
        if (bmp280_read_temperature_c(&temperature_c)) {
            ESP_LOGI(TAG, "Temperatura BMP280: %.2f C", temperature_c);

            if (wifi_esta_conectado() && !mqtt_started) {
                if (!clock_ready) {
                    clock_ready = time_sync_wait_for_clock();
                    if (!clock_ready) {
                        ESP_LOGW(TAG, "Seguimos sin hora valida. MQTT puede fallar si el broker exige certificados vigentes.");
                    }
                }
                mqtt_started = mqtt_telemetry_init();
            }

            if (mqtt_started && mqtt_telemetry_connected()) {
                if (mqtt_telemetry_publish_temperature(temperature_c) != ESP_OK) {
                    ESP_LOGW(TAG, "No pude publicar la temperatura en este ciclo.");
                }
            } else if (!wifi_esta_conectado()) {
                ESP_LOGI(TAG, "Esperando conexion Wi-Fi antes de publicar telemetria.");
            } else if (!mqtt_started) {
                ESP_LOGW(TAG, "MQTT aun no esta listo. Revisa la configuracion y certificados.");
            } else {
                ESP_LOGI(TAG, "Esperando conexion MQTT antes de publicar telemetria.");
            }
        } else {
            ESP_LOGW(TAG, "No pude leer la temperatura del BMP280 en este ciclo.");
        }

        vTaskDelay(pdMS_TO_TICKS(MQTT_PUBLISH_INTERVAL_MS));
    }
}

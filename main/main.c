#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "nvs_flash.h"
#include "stdbool.h"
#include "string.h"

#include "wifi_credentials.h"

static const char *TAG = "cookies_n_cream";

static bool wifi_conectado = false;

static void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG, "Wifi iniciando intentando conectar... "); 
        esp_wifi_connect();

    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Desconectado, reintentando... ");
        wifi_conectado = false;
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        
        ESP_LOGI(TAG, "ip obtenida: " IPSTR, IP2STR(&event->ip_info.ip));
        wifi_conectado = true;
    }
}


void wifi_init_sta(void) {
    nvs_flash_init();
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL);

    wifi_config_t wifi_config = {};
    strncpy((char *) wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strncpy((char *) wifi_config.sta.password, WIFI_PASSWORD, sizeof(wifi_config.sta.password));

    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();
}


void app_main(void)
{
    wifi_init_sta();
    
    int counter = 0;

    ESP_LOGI(TAG, "ESP-IDF listo. Si lees esto por UART, ya tenemos toolchain + flash + monitor funcionando.");

    while (1) {
        if (wifi_conectado) {
            ESP_LOGI(TAG, "Heartbeat %d", counter++);
        } else {
            ESP_LOGI(TAG, "Esperando conexion Wi-Fi...");
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

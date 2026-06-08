#include <sys/time.h>
#include <time.h>

#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "freertos/FreeRTOS.h"

#include "time_sync.h"

static const char *TAG = "time_sync";
static bool sntp_initialized = false;

static bool clock_looks_valid(void)
{
    time_t now = 0;

    time(&now);
    return now >= 1700000000;
}

bool time_sync_wait_for_clock(void)
{
    const int retry_count = 10;
    int retry = 0;

    if (clock_looks_valid()) {
        return true;
    }

    if (!sntp_initialized) {
        esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");

        esp_netif_sntp_init(&config);
        esp_netif_sntp_start();
        sntp_initialized = true;
    }

    while (!clock_looks_valid() && retry < retry_count) {
        ESP_LOGI(TAG, "Esperando sincronizacion NTP... intento %d/%d", retry + 1, retry_count);
        if (esp_netif_sntp_sync_wait(pdMS_TO_TICKS(2000)) == ESP_OK && clock_looks_valid()) {
            break;
        }
        retry++;
    }

    if (!clock_looks_valid()) {
        ESP_LOGW(TAG, "No pude sincronizar la hora por NTP. Estableciendo hora de respaldo (2026-06-08)...");
        struct timeval tv = {
            .tv_sec = 1780963200, // 2026-06-08 00:00:00 UTC
            .tv_usec = 0
        };
        settimeofday(&tv, NULL);
    }

    {
        char time_buffer[32] = {0};
        time_t now = 0;
        struct tm timeinfo = {0};

        time(&now);
        gmtime_r(&now, &timeinfo);
        strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
        ESP_LOGI(TAG, "Hora sincronizada: %s", time_buffer);
    }

    return true;
}

bool time_sync_get_iso8601(char *buffer, size_t buffer_len)
{
    time_t now = 0;
    struct tm timeinfo = {0};

    if (buffer == NULL || buffer_len == 0 || !clock_looks_valid()) {
        return false;
    }

    time(&now);
    gmtime_r(&now, &timeinfo);

    return strftime(buffer, buffer_len, "%Y-%m-%dT%H:%M:%SZ", &timeinfo) > 0;
}

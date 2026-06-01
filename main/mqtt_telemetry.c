#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp_timer.h"
#include "mqtt_client.h"

#include "mqtt_config.h"
#include "mqtt_telemetry.h"
#include "time_sync.h"

static const char *TAG = "mqtt_telemetry";

static esp_mqtt_client_handle_t mqtt_client = NULL;
static bool mqtt_connected = false;
static char mqtt_topic[96] = {0};

#if MQTT_CERT_FILES_PRESENT
static char mqtt_client_id[64] = {0};

extern const uint8_t ca_pem_start[] asm("_binary_ca_pem_start");
extern const uint8_t device_crt_start[] asm("_binary_device_crt_start");
extern const uint8_t device_key_start[] asm("_binary_device_key_start");

static void build_client_identity(void)
{
    uint8_t mac[6] = {0};

    ESP_ERROR_CHECK(esp_read_mac(mac, ESP_MAC_WIFI_STA));

    snprintf(
        mqtt_client_id,
        sizeof(mqtt_client_id),
        "sensor_%s_%02X%02X%02X",
        MQTT_DEVICE_ID,
        mac[3],
        mac[4],
        mac[5]);

    snprintf(
        mqtt_topic,
        sizeof(mqtt_topic),
        "devices/%s/telemetry",
        MQTT_DEVICE_ID);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;

    (void)handler_args;
    (void)base;

    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        mqtt_connected = true;
        ESP_LOGI(TAG, "Conectado al broker MQTT por mTLS.");
        break;
    case MQTT_EVENT_DISCONNECTED:
        mqtt_connected = false;
        ESP_LOGW(TAG, "MQTT desconectado.");
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "Publicacion confirmada, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_ERROR:
        mqtt_connected = false;
        ESP_LOGE(TAG, "MQTT error type=0x%x", event->error_handle->error_type);
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            ESP_LOGE(TAG, "esp-tls err=0x%x, tls stack=0x%x, errno=%d",
                     event->error_handle->esp_tls_last_esp_err,
                     event->error_handle->esp_tls_stack_err,
                     event->error_handle->esp_transport_sock_errno);
        }
        break;
    default:
        break;
    }
}
#endif

bool mqtt_telemetry_init(void)
{
#if !MQTT_CERT_FILES_PRESENT
    ESP_LOGE(TAG, "No encontre main/certs/ca.pem, main/certs/device.crt y main/certs/device.key.");
    ESP_LOGW(TAG, "Sin esos archivos no puedo iniciar mTLS. Colocalos fuera de git y recompila.");
    return false;
#else
    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address.uri = MQTT_BROKER_URI,
            .verification.certificate = (const char *)ca_pem_start,
        },
        .credentials = {
            .client_id = mqtt_client_id,
            .authentication = {
                .certificate = (const char *)device_crt_start,
                .key = (const char *)device_key_start,
            },
        },
    };

    if (mqtt_client != NULL) {
        return true;
    }

    build_client_identity();

    ESP_LOGI(TAG, "Broker MQTT: %s", MQTT_BROKER_URI);
    ESP_LOGI(TAG, "Topic de telemetria: %s", mqtt_topic);
    ESP_LOGI(TAG, "Client ID: %s", mqtt_client_id);

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (mqtt_client == NULL) {
        ESP_LOGE(TAG, "No pude crear el cliente MQTT.");
        return false;
    }

    ESP_ERROR_CHECK(esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    ESP_ERROR_CHECK(esp_mqtt_client_start(mqtt_client));

    return true;
#endif
}

bool mqtt_telemetry_connected(void)
{
    return mqtt_connected;
}

esp_err_t mqtt_telemetry_publish_temperature(float temperature_c)
{
    char payload[256];
    char timestamp[32] = {0};
    int msg_id = -1;
    int payload_len = 0;
    uint64_t uptime_ms = (uint64_t)(esp_timer_get_time() / 1000ULL);

    if (mqtt_client == NULL) {
        return ESP_ERR_INVALID_STATE;
    }

    if (time_sync_get_iso8601(timestamp, sizeof(timestamp))) {
        payload_len = snprintf(
            payload,
            sizeof(payload),
            "{\"deviceId\":\"%s\",\"timestamp\":\"%s\",\"uptimeMs\":%" PRIu64 ",\"temperature\":%.2f,\"status\":\"active\"}",
            MQTT_DEVICE_ID,
            timestamp,
            uptime_ms,
            temperature_c);
    } else {
        payload_len = snprintf(
            payload,
            sizeof(payload),
            "{\"deviceId\":\"%s\",\"timestamp\":null,\"uptimeMs\":%" PRIu64 ",\"temperature\":%.2f,\"status\":\"active\"}",
            MQTT_DEVICE_ID,
            uptime_ms,
            temperature_c);
    }

    if (payload_len <= 0 || payload_len >= (int)sizeof(payload)) {
        return ESP_ERR_INVALID_SIZE;
    }

    msg_id = esp_mqtt_client_publish(mqtt_client, mqtt_topic, payload, payload_len, 1, 0);
    if (msg_id < 0) {
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Publicado a %s -> Temp: %.2f C", mqtt_topic, temperature_c);
    return ESP_OK;
}

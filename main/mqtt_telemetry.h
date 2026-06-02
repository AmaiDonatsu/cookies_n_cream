#ifndef MQTT_TELEMETRY_H
#define MQTT_TELEMETRY_H

#include <stdbool.h>

#include "esp_err.h"

bool mqtt_telemetry_init(void);
bool mqtt_telemetry_connected(void);
esp_err_t mqtt_telemetry_publish_temperature(float temperature_c);
esp_err_t mqtt_telemetry_publish_environment(float temperature_c, float pressure_hpa);

#endif // MQTT_TELEMETRY_H

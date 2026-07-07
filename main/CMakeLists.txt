set(component_srcs
    "ads1115.c"
    "bh1750.c"
    "bmp280_probe.c"
    "i2c_bus.c"
    "wifi_config.c"
    "mqtt_telemetry.c"
    "time_sync.c"
    "main.c")

set(component_requires
    driver
    esp_event
    esp_netif
    esp-tls
    esp_wifi
    mqtt
    nvs_flash)

set(component_embed_txtfiles)

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/certs/ca.pem"
   AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/certs/device.crt"
   AND EXISTS "${CMAKE_CURRENT_LIST_DIR}/certs/device.key")
    list(APPEND component_embed_txtfiles
        "certs/ca.pem"
        "certs/device.crt"
        "certs/device.key")
    set(mqtt_cert_files_present 1)
else()
    set(mqtt_cert_files_present 0)
endif()

idf_component_register(
    SRCS ${component_srcs}
    INCLUDE_DIRS "."
    REQUIRES ${component_requires}
    EMBED_TXTFILES ${component_embed_txtfiles})

target_compile_definitions(${COMPONENT_LIB} PRIVATE MQTT_CERT_FILES_PRESENT=${mqtt_cert_files_present})

# MQTT Telemetria desde ESP32-C6

## Objetivo

Este firmware conecta el `ESP32-C6` a tu Wi-Fi, lee temperatura real del `BMP280` y la publica por `MQTT` usando `mTLS`.

## Archivos locales que no van a git

### Configuracion MQTT

Crea o edita este archivo local:

`main/mqtt_config.h`

Basate en:

`main/mqtt_config.h.example`

Valores importantes:

- `MQTT_BROKER_URI`
- `MQTT_DEVICE_ID`
- `MQTT_PUBLISH_INTERVAL_MS`

Ejemplo:

```c
#define MQTT_BROKER_URI "mqtts://192.168.1.50:8883"
#define MQTT_DEVICE_ID "DEV-1A2B3C4D"
#define MQTT_PUBLISH_INTERVAL_MS 5000
```

Importante:

- En el `ESP32`, `localhost` no sirve para apuntar a tu PC.
- Debes usar la IP local o el hostname real del servidor MQTT en tu red.

## Certificados locales

Coloca estos archivos, sin subirlos a git:

- `main/certs/ca.pem`
- `main/certs/device.crt`
- `main/certs/device.key`

## Si vienes de tu JSON del simulador TypeScript

Debes convertir:

- `caCert` -> `main/certs/ca.pem`
- `deviceCert` -> `main/certs/device.crt`
- `deviceKey` -> `main/certs/device.key`

Tambien puedes importarlos automatico con:

```powershell
.\tools\import-mqtt-certs.ps1 .\ruta\certs_DEV-1A2B3C4D.json
```

Cada archivo debe quedar en formato PEM, incluyendo las lineas:

- `-----BEGIN CERTIFICATE-----`
- `-----END CERTIFICATE-----`

o para la llave:

- `-----BEGIN PRIVATE KEY-----`
- `-----END PRIVATE KEY-----`

## Flujo esperado

Al arrancar deberias ver por UART:

1. Conexion Wi-Fi
2. Sincronizacion NTP
3. Deteccion del `BMP280`
4. Inicio del cliente `MQTT`
5. Publicaciones de temperatura

Ejemplo:

```text
I (...) cookies_n_cream: Temperatura BMP280: 30.47 C
I (...) mqtt_telemetry: Publicado a devices/DEV-1A2B3C4D/telemetry -> Temp: 30.47 C
```

## Comando de prueba

```powershell
.\tools\idf.ps1 -Action flash-monitor -Port COM3
```

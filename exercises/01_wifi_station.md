# Ejercicio 01 - Conectar el ESP32-C6 a Wi-Fi

## Objetivo

Modificar la app actual para que tu `ESP32-C6` se conecte a una red Wi-Fi como cliente (`station mode`) y escriba por serial:

- cuando intenta conectarse
- cuando se conecta
- la IP que recibe
- cuando se desconecta

No copies una solucion completa todavia. La idea es que armes el flujo y vayas probando por partes.

## Archivo a modificar

Trabaja en:

- `main/main.c`

## Punto de partida

Ahora mismo tu app solo imprime:

- `ESP-IDF listo...`
- `Heartbeat ...`

Tu tarea es reemplazar o extender ese comportamiento para agregar la conexion Wi-Fi.

## Forma mas simple para este primer intento

Para este ejercicio, mete temporalmente el nombre y password de tu red directo en el codigo con macros como:

```c
#define WIFI_SSID "TU_RED"
#define WIFI_PASS "TU_PASSWORD"
```

Mas adelante lo podemos mover a `menuconfig`, pero por ahora esto te deja enfocarte en entender el flujo.

## Flujo que debes implementar

Tu programa deberia hacer esto, en este orden:

1. Inicializar `NVS`.
2. Inicializar `esp_netif`.
3. Crear el loop de eventos por defecto.
4. Crear la interfaz de red Wi-Fi en modo station.
5. Inicializar el driver Wi-Fi con `WIFI_INIT_CONFIG_DEFAULT()`.
6. Registrar handlers para eventos de Wi-Fi e IP.
7. Cargar `SSID` y `password` en `wifi_config_t`.
8. Poner el modo `WIFI_MODE_STA`.
9. Aplicar la configuracion con `esp_wifi_set_config()`.
10. Arrancar Wi-Fi con `esp_wifi_start()`.
11. Intentar conectar con `esp_wifi_connect()`.
12. Cuando obtengas IP, imprimirla por serial.

## Eventos que necesitas escuchar

Como minimo, maneja estos:

- `WIFI_EVENT_STA_START`
- `WIFI_EVENT_STA_DISCONNECTED`
- `IP_EVENT_STA_GOT_IP`

## Comportamiento esperado de cada evento

### `WIFI_EVENT_STA_START`

Cuando ocurra:

- imprime un log diciendo que el Wi-Fi arranco
- llama `esp_wifi_connect()`

### `WIFI_EVENT_STA_DISCONNECTED`

Cuando ocurra:

- imprime un log indicando que se desconecto
- vuelve a intentar con `esp_wifi_connect()`

No te compliques todavia con un contador de reintentos. Puedes reconectar siempre.

### `IP_EVENT_STA_GOT_IP`

Cuando ocurra:

- extrae la IP del evento
- imprimela por serial

Pista: el dato del evento llega como `ip_event_got_ip_t *`.

## APIs que vas a necesitar buscar

Estas son las funciones clave que deberias usar:

- `nvs_flash_init`
- `esp_netif_init`
- `esp_event_loop_create_default`
- `esp_netif_create_default_wifi_sta`
- `esp_wifi_init`
- `esp_event_handler_instance_register`
- `esp_wifi_set_mode`
- `esp_wifi_set_config`
- `esp_wifi_start`
- `esp_wifi_connect`

## Includes que probablemente vas a necesitar

Investiga y agrega los headers correctos. Seguramente vas a terminar usando varios de estos:

- `esp_event.h`
- `esp_log.h`
- `esp_netif.h`
- `esp_wifi.h`
- `nvs_flash.h`
- `esp_mac.h`
- `esp_system.h`
- `lwip/ip4_addr.h`

No tomes esta lista como obligatoria exacta. Si el compilador pide menos o mas, ajustalo.

## Estructura sugerida

Una forma ordenada de resolverlo es:

1. Definir `TAG`, `WIFI_SSID`, `WIFI_PASS`
2. Crear una funcion handler para eventos
3. Crear una funcion como `wifi_init_sta()`
4. Llamar esa funcion desde `app_main()`

## Criterio de exito

Tu solucion esta bien si al correr:

```powershell
.\tools\idf.ps1 -Action flash-monitor -Port COM3
```

ves algo equivalente a esto:

```text
I (...) wifi: iniciando station
I (...) wifi: intentando conectar...
I (...) wifi: conectado
I (...) wifi: IP obtenida: 192.168.x.x
```

No importa si el texto exacto cambia. Lo importante es que:

- conecte
- reciba IP
- lo muestre por serial

## Pistas si te atoraste

### Pista 1

`wifi_config_t` tiene un campo `sta`.

### Pista 2

Para copiar `SSID` y password al struct, normalmente se usa `strncpy` o asignacion cuidando tamanos.

### Pista 3

Para imprimir la IP, busca la macro:

- `IPSTR`
- `IP2STR`

## Errores comunes

- Usar target `esp32` en vez de `esp32c6`
- Olvidar inicializar `nvs`
- No registrar eventos antes de arrancar Wi-Fi
- No llamar `esp_wifi_connect()`
- Intentar conectar a una red de `5 GHz`

## Tu mini checklist

Marca mentalmente esto antes de probar:

- ya puse `SSID` y `password`
- ya compile sin errores
- ya flashee a `COM3`
- mi red es `2.4 GHz`

## Extra opcional

Si te sale rapido, agrega uno de estos extras:

1. Un log con el numero de reintentos.
2. Un booleano global que indique si ya hay conexion.
3. Dejar de imprimir `Heartbeat` cuando aun no hay IP, y reanudarlo cuando ya hay conexion.

## Cuando termines

Pruebalo y luego me pegas:

- el contenido de `main/main.c`
- o el error de compilacion
- o la salida del monitor serial

Y lo revisamos juntos.

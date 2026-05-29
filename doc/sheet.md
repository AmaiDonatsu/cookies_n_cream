# Circuit Sheet

## Objetivo

Documentar de forma clara como esta conectado el sensor `BMP280 3.3V` al `ESP32-C6`, para poder revisar el cableado sin confusiones.

## Modo de uso esperado

El sensor esta pensado para trabajar en modo `I2C`.

Para eso:

- `SDA` debe ir al pin de datos I2C del ESP32
- `SCL` debe ir al pin de reloj I2C del ESP32
- `CSB` debe ir a `3.3V`
- `SDO` debe ir a `GND`

## Conexion real final

### BMP280 -> ESP32-C6

| Pin del BMP280 | Color | Pin final del ESP32-C6 | Nota |
|---|---|---|---|
| `VCC` | Rojo | `3V3` | Alimentacion del sensor |
| `GND` | Cafe | `G` | Tierra del sensor |
| `SCL` | Naranja | `GPIO7` | Clock I2C |
| `SDA` | Amarillo | `GPIO6` | Data I2C |
| `CSB` | Verde | `3V3` | Necesario para modo I2C |
| `SDO` | Azul | `G` | Direccion I2C `0x76` |

## Validacion visual rapida

### Del lado del sensor

Viendo el modulo BMP280 del mismo lado que muestra los nombres de pines, el orden esperado de colores es:

1. Azul -> `SDO`
2. Verde -> `CSB`
3. Amarillo -> `SDA`
4. Naranja -> `SCL`
5. Cafe -> `GND`
6. Rojo -> `VCC`

### Del lado del ESP32-C6

Los jumpers deben terminar asi:

1. Rojo -> `3V3`
2. Cafe -> `G`
3. Amarillo -> `GPIO6`
4. Naranja -> `GPIO7`
5. Verde -> `3V3`
6. Azul -> `G`

## Detalle por bloques

### 1. Sensor BMP280

| Pin del sensor | Fila en protoboard |
|---|---|
| `VCC` | `a34` |
| `GND` | `a33` |
| `SCL` | `a32` |
| `SDA` | `a31` |
| `CSB` | `a30` |
| `SDO` | `a29` |

### 2. Jumpers del sensor hacia la mitad de la protoboard

| Color | Desde | Hacia |
|---|---|---|
| Rojo | `e34` | `f63` |
| Cafe | `e33` | `f62` |
| Naranja | `e32` | `f61` |
| Amarillo | `e31` | `f60` |
| Verde | `e30` | `f59` |
| Azul | `e29` | `f58` |

### 3. Conexiones del ESP32-C6

| Pin del ESP32-C6 | Posicion | Va a | Funcion |
|---|---|---|---|
| `3V3` | `j10` | `f63` y `f59` | Alimenta `VCC` y fija `CSB` en alto |
| `G` | `j62` | `f62` | Tierra del sensor |
| `GPIO6` | `j60` | `f60` | `SDA` |
| `GPIO7` | `j61` | `f61` | `SCL` |
| `G` | `j58` | `f58` | Fija `SDO` en bajo |

## Linea completa de cada conexion

### Alimentacion

- `BMP280 VCC` -> `a34` -> `e34` -> `f63` -> `ESP32 3V3`
- `BMP280 GND` -> `a33` -> `e33` -> `f62` -> `ESP32 G`

### I2C

- `BMP280 SDA` -> `a31` -> `e31` -> `f60` -> `ESP32 GPIO6`
- `BMP280 SCL` -> `a32` -> `e32` -> `f61` -> `ESP32 GPIO7`

### Pines de seleccion de modo / direccion

- `BMP280 CSB` -> `a30` -> `e30` -> `f59` -> `ESP32 3V3`
- `BMP280 SDO` -> `a29` -> `e29` -> `f58` -> `ESP32 G`

## Interpretacion tecnica

Con este cableado, el sensor deberia comportarse asi:

- modo de comunicacion: `I2C`
- direccion I2C esperada: `0x76`

## Alimentacion esperada

Para esta prueba:

- el `ESP32-C6` se alimenta por `USB`
- el sensor se alimenta desde el pin `3V3` del ESP32

No hace falta una fuente externa si solo estamos probando un BMP280 y Wi-Fi.

## Checklist de revision fisica

Antes de culpar al codigo, revisar esto uno por uno:

1. El `BMP280` esta orientado correctamente y los nombres de pines se leyeron del lado correcto.
2. `VCC` realmente llega a `3.3V`, no a `5V`.
3. `GND` realmente llega a `G`.
4. `SDA` llega a `GPIO6`.
5. `SCL` llega a `GPIO7`.
6. `CSB` llega a `3.3V`.
7. `SDO` llega a `GND`.
8. No hay un cable corrido una fila arriba o abajo en la protoboard.
9. `SDA` y `SCL` no estan intercambiados.

## Si la prueba sigue fallando

Las primeras pruebas fisicas recomendadas son:

1. Revisar si el modulo esta volteado o espejado.
2. Intercambiar temporalmente `SDA` y `SCL`.
3. Verificar con cuidado que `CSB` de verdad este en `3.3V`.
4. Verificar que `SDO` de verdad este en `GND`.

## Resultado esperado en el firmware de prueba

Si todo esta bien conectado, el monitor serie deberia mostrar algo parecido a esto:

```text
I (...) bmp280_probe: Escaneando I2C en SDA=GPIO6, SCL=GPIO7...
I (...) bmp280_probe: Dispositivo I2C detectado en 0x76
I (...) bmp280_probe: BMP280 detectado correctamente en 0x76 con chip ID 0x58.
```


## Referencias

- [Foto de pines del ESP32-C6](C:/Users/rodol/development/novel/cookies_n_cream/doc/esp32.jpeg)
- [Foto de colores en el sensor](C:/Users/rodol/development/novel/cookies_n_cream/doc/cables.jpeg)

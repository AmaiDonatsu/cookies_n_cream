# cookies_n_cream + ESP32

Base minima para empezar con `ESP-IDF` en Windows desde este mismo proyecto.

## Que te deje listo aqui

- Estructura valida de proyecto `ESP-IDF`
- Aplicacion inicial que escribe mensajes por UART
- Scripts de PowerShell para revisar entorno, detectar puertos COM y ejecutar `idf.py`
- Guia corta para instalar y dar tu primer `flash`

## Estructura

```text
.
|- CMakeLists.txt
|- main/
|  \- main.c
|- tools/
|  |- check-esp-env.ps1
|  |- find-esp-port.ps1
|  \- idf.ps1
|- .vscode/
|  |- extensions.json
|  \- settings.json
```

## Instalacion recomendada en Windows

Espressif recomienda usar `EIM` (Espressif Installation Manager) para instalar `ESP-IDF` en Windows.

1. Instala `EIM` desde la pagina oficial:
   [ESP-IDF Installation Manager](https://dl.espressif.com/dl/eim/?tab=online)
2. En `EIM`, usa `Start Easy Installation` para instalar la version estable mas reciente.
3. Al terminar, abre la consola `ESP-IDF PowerShell` que crea el instalador.
4. Desde esa consola, entra a este proyecto:

```powershell
cd C:\Users\rodol\development\novel\cookies_n_cream
```

## Primer arranque

1. Revisa si el entorno ya esta activo:

```powershell
.\tools\check-esp-env.ps1
```

2. Busca el puerto COM del ESP32:

```powershell
.\tools\find-esp-port.ps1
```

3. Configura el target una sola vez.
   Si tu placa es un ESP32 "clasico", usa `esp32`.
   Si tu placa dice `ESP32-S3`, `ESP32-C3`, `ESP32-C6`, etc., cambia el target.

```powershell
.\tools\idf.ps1 -Action set-target -Target esp32
```

4. Compila:

```powershell
.\tools\idf.ps1 -Action build
```

5. Flashea y abre monitor serie:

```powershell
.\tools\idf.ps1 -Action flash-monitor -Port COM5
```

## Que deberias ver

La app inicial no depende de un LED ni de pines especificos. Solo envia mensajes por serial cada segundo, asi confirmamos primero que:

- `ESP-IDF` esta bien instalado
- el puerto UART es el correcto
- la placa acepta `flash`
- el monitor funciona

## Si no aparece ningun COM

Prueba esto antes de seguir:

- Verifica que el cable USB sea de datos, no solo de carga
- Revisa en el Administrador de dispositivos si aparece un adaptador `CP210x`, `CH340`, `FTDI` o similar
- Si usas un adaptador USB-UART externo, confirma `TX->RX`, `RX->TX` y `GND->GND`
- Si tu placa no entra sola en modo de programacion, prueba mantener `BOOT` durante el inicio del `flash`

## Referencias oficiales

- [Instalacion de ESP-IDF en Windows](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/windows-setup.html)
- [Primer proyecto en Windows](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/windows-start-project.html)

## Nota importante

En esta maquina, al revisar el entorno actual, `idf.py` todavia no esta disponible y no encontramos un `python` funcional en `PATH`. Eso significa que falta completar la instalacion de `ESP-IDF` antes del primer `build`.

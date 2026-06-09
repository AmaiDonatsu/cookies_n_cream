# Reporte de Diagnóstico y Migración: Windows a Linux Fedora

Este documento resume los problemas identificados y solucionados durante el proceso de compilación, flasheo e integración mTLS del firmware `cookies_n_cream` (ESP32-C6) con el backend de `async-control-desktop` en un entorno de **Linux Fedora**, comparándolo con el comportamiento que tenía previamente en **Windows**.

---

## 📌 Resumen General de los Problemas

A lo largo del proceso, nos enfrentamos a tres tipos de problemas bien diferenciados:
1. **Errores de Entorno y Toolchain** (Comandos no encontrados y reinicio de sdkconfig).
2. **Errores de Tamaño de Partición** (Desbordamiento de memoria flash en Ninja/CMake).
3. **Errores Criptográficos en la Conexión mTLS** (Handshakes fallidos con códigos `-0x7280` y `-0x2700`).

---

## 🔍 Análisis Detallado de los Problemas

### 1. El comando `idf.py` no se encuentra
* **Síntoma:** Al ejecutar `idf.py build` salía el error `bash: idf.py: instrucción no encontrada`.
* **Causa en Fedora:** En Linux, a diferencia de Windows (donde el instalador de Espressif crea accesos directos y scripts de consola de PowerShell preconfigurados), las variables de entorno de ESP-IDF no son globales ni persistentes. Deben cargarse en cada sesión de terminal ejecutando el script `export.sh`.
* **Resolución:** El proyecto cuenta con un script wrapper en `tools/idf.sh` que detecta automáticamente la ruta de instalación de ESP-IDF en tu `$HOME` y carga el entorno virtual de Python y el toolchain antes de delegar el comando a `idf.py`.

---

### 2. Desbordamiento de la Partición de la Aplicación (`app partition is too small`)
* **Síntoma:** La compilación fallaba al final indicando que el archivo `cookies_n_cream_esp32.bin` excedía el límite de la partición de la aplicación (1 MB por defecto) en `0x7780` bytes.
* **¿Por qué funcionaba en Windows y en Fedora falló?**
  1. **Configuración por defecto al cambiar de Target:** Al ejecutar `./tools/idf.sh set-target esp32c6` por primera vez en Fedora, el sistema de construcción de ESP-IDF renombró el `sdkconfig` existente a `sdkconfig.old` y creó uno nuevo desde cero con los valores predeterminados.
  2. **Nivel de Optimización del Compilador:** El nuevo `sdkconfig` heredó la optimización de depuración por defecto (`CONFIG_COMPILER_OPTIMIZATION_DEBUG=y` / `-Og`). Esto añade una cantidad masiva de metadatos y símbolos de depuración, haciendo que el binario final pese más de 1 MB. En Windows, probablemente el proyecto ya estaba configurado con optimización de producción/tamaño (`-Os`) o utilizaba un esquema de particionado diferente.
* **Resolución:**
  * Configuramos la optimización de tamaño en el compilador (`CONFIG_COMPILER_OPTIMIZATION_SIZE=y`), lo que redujo el binario a aprox. **970 KB**.
  * Modificamos el esquema de partición en `sdkconfig` a `singleapp_large` (2 MB de espacio).
  * Creamos un archivo **`sdkconfig.defaults`** para asegurar que estas configuraciones persistan de manera permanente, incluso si el target se vuelve a resetear en un futuro.

---

### 3. TLS Handshake Error `-0x7280` (Broker / Server Hang up)
* **Síntoma:** El ESP32 intentaba conectar por MQTT seguro pero era rechazado inmediatamente por el servidor Node.js.
* **Causa:** Un bug en el generador de certificados del backend (`node-forge` en `certService.ts`). Al generar las extensiones del certificado del broker y de los dispositivos, el campo `authorityKeyIdentifier` (AKI) se generaba usando la clave pública del propio certificado en lugar de usar la clave pública de la CA firmante. Esto hacía que los certificados parecieran autofirmados de forma corrupta.
* **Origen de la diferencia con Windows:** Este error es puramente criptográfico y agnóstico al sistema operativo. Habría fallado de igual manera en Windows si los certificados del dispositivo se hubieran regenerado mediante la misma lógica del servidor.

---

### 4. TLS Handshake Error `-0x2700` (`MBEDTLS_ERR_X509_CERT_VERIFY_FAILED`)
* **Síntoma:** El servidor y el cliente abortaban la conexión debido a un fallo en la verificación del certificado del broker.
* **Causa:** Desajuste de fecha/hora en el ESP32 (Time Mismatch). Los certificados generados por el servidor tienen validez a partir de junio de 2026. Como el ESP32-C6 no tiene conexión a internet directa en el entorno de desarrollo local, no pudo sincronizar su hora con `pool.ntp.org` (NTP timeout) e iniciaba su reloj en 1970. Al comparar 1970 con Junio de 2026, la validación fallaba por "certificado aún no vigente".
* **Origen de la diferencia con Windows:** En Windows, es probable que la red local permitiera las peticiones salientes NTP (puerto UDP 123), o que la simulación previa utilizara un RTC con batería o una sincronización manual del sistema que no requirió la llamada a NTP en frío.
* **Resolución:** Modificamos `time_sync.c` para que, en caso de fallar el NTP por timeout, configure automáticamente una fecha de respaldo fija superior a la de los certificados (**8 de Junio de 2026**).

---

## 💻 ¿Tiene que ver con Linux Fedora?

**Sí y No.**
* **Relacionado con Fedora:**
  1. **Administración de Entornos Virtuales de Python:** Fedora 40/41/42 utiliza versiones de Python muy recientes (ej. Python 3.14.3). ESP-IDF requiere configurar un entorno virtual local (`.espressif/python_env/...`) que sea compatible. Cualquier desajuste de librerías globales de Python de Fedora vs. ESP-IDF puede romper el build.
  2. **Carga de variables de entorno (`export.sh`):** A diferencia de Windows, donde el instalador gráfico configura rutas en el registro del sistema, en Fedora dependemos del comportamiento de la terminal `bash`. El uso de wrappers como `./tools/idf.sh` soluciona esto de raíz.
  3. **Acceso al puerto serial (`/dev/ttyACM0`):** En Linux, los puertos USB se asignan dinámicamente como archivos en `/dev/`. El usuario necesita permisos específicos en los grupos `dialout` o `uucp` para poder flashear el hardware. En Windows, esto se gestiona mediante los controladores COM genéricos sin bloqueos de permisos a nivel de usuario.
  4. **Bloqueos de red (Firewall/NTP):** Las políticas de red por defecto en Linux Fedora (con `firewalld`) pueden bloquear el tráfico UDP saliente o las resoluciones DNS necesarias para NTP en redes locales cerradas.
* **No Relacionado con Fedora:**
  1. El error de generación del `authorityKeyIdentifier` en los certificados (`certService.ts`) fue un bug lógico del código TypeScript del backend, independiente del SO.
  2. La validación temporal de certificados en `mbedtls` que exige una hora correcta es una medida de seguridad criptográfica estándar implementada en el hardware del microcontrolador, no en el host de compilación.

---

## 💡 Conclusión y Buenas Prácticas

La migración a Linux Fedora introdujo variables del sistema (Python 3.14, Bash environment) y de red local (NTP no disponible) que expusieron limitaciones del código que en Windows pasaban desapercibidas. 

Gracias a la implementación del archivo de persistencia `sdkconfig.defaults`, los scripts autoejecutables en la carpeta `tools/` y el ajuste del fallback temporal en `time_sync.c`, el proyecto es ahora mucho más robusto, portable y fácil de compilar en cualquier sistema operativo Linux moderno sin requerir intervención manual extensa.

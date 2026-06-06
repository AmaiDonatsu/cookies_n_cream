#!/usr/bin/env bash

set -u

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

# shellcheck source=tools/esp-idf-env.sh
source "$SCRIPT_DIR/esp-idf-env.sh"

show_command_status() {
    local name="$1"
    local path

    if path="$(command -v "$name" 2>/dev/null)"; then
        printf '[OK]     %s -> %s\n' "$name" "$path"
    else
        printf '[FALTA]  %s\n' "$name"
    fi
}

echo "=== Revision rapida del entorno ESP-IDF ==="
echo

if import_esp_idf_environment; then
    echo "[OK]     entorno ESP-IDF activado automaticamente"
    echo
fi

show_command_status "idf.py"
show_command_status "git"
show_command_status "cmake"
show_command_status "ninja"
show_command_status "python3"
show_command_status "pip3"
show_command_status "gperf"
show_command_status "dfu-util"

echo
echo "Variables de entorno:"
printf 'IDF_PATH=%s\n' "${IDF_PATH:-<sin definir>}"
printf 'IDF_TOOLS_PATH=%s\n' "${IDF_TOOLS_PATH:-<sin definir>}"

echo
echo "Versiones detectadas:"
python3 --version 2>/dev/null || true
idf.py --version 2>/dev/null || true

echo
echo "Puertos serie detectados:"
found_port=0
for port in /dev/ttyUSB* /dev/ttyACM* /dev/serial/by-id/*; do
    if [[ -e "$port" ]]; then
        printf -- '- %s\n' "$port"
        found_port=1
    fi
done

if [[ "$found_port" -eq 0 ]]; then
    echo "No se detectaron puertos tipo /dev/ttyUSB* o /dev/ttyACM*."
fi

echo
if ! command -v idf.py >/dev/null 2>&1; then
    echo "Siguiente paso: instala ESP-IDF y vuelve a ejecutar este script."
fi

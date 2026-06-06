#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

# shellcheck source=tools/esp-idf-env.sh
source "$SCRIPT_DIR/esp-idf-env.sh"

usage() {
    cat <<'EOF'
Uso:
  tools/idf.sh set-target [esp32|esp32s3|esp32c3|...]
  tools/idf.sh build
  tools/idf.sh flash /dev/ttyUSB0
  tools/idf.sh monitor /dev/ttyUSB0
  tools/idf.sh flash-monitor /dev/ttyUSB0
  tools/idf.sh menuconfig
  tools/idf.sh fullclean
EOF
}

if ! import_esp_idf_environment; then
    echo "No encuentro idf.py. Instala ESP-IDF o define IDF_PATH antes de usar este script." >&2
    exit 1
fi

action="${1:-build}"

case "$action" in
    set-target)
        target="${2:-esp32}"
        idf.py set-target "$target"
        ;;
    build)
        idf.py build
        ;;
    flash)
        port="${2:-}"
        if [[ -z "$port" ]]; then
            echo "Debes indicar el puerto, por ejemplo: tools/idf.sh flash /dev/ttyUSB0" >&2
            exit 2
        fi
        idf.py -p "$port" flash
        ;;
    monitor)
        port="${2:-}"
        if [[ -z "$port" ]]; then
            echo "Debes indicar el puerto, por ejemplo: tools/idf.sh monitor /dev/ttyUSB0" >&2
            exit 2
        fi
        idf.py -p "$port" monitor
        ;;
    flash-monitor)
        port="${2:-}"
        if [[ -z "$port" ]]; then
            echo "Debes indicar el puerto, por ejemplo: tools/idf.sh flash-monitor /dev/ttyUSB0" >&2
            exit 2
        fi
        idf.py -p "$port" flash monitor
        ;;
    menuconfig)
        idf.py menuconfig
        ;;
    fullclean)
        idf.py fullclean
        ;;
    -h|--help|help)
        usage
        ;;
    *)
        echo "Accion no soportada: $action" >&2
        usage >&2
        exit 2
        ;;
esac

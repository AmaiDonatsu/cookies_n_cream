#!/usr/bin/env bash

set -euo pipefail

IDF_VERSION="${IDF_VERSION:-v6.0.1}"
INSTALL_ROOT="${INSTALL_ROOT:-$HOME/esp/${IDF_VERSION}}"
IDF_PATH="${IDF_PATH:-$INSTALL_ROOT/esp-idf}"
IDF_TOOLS_PATH="${IDF_TOOLS_PATH:-$HOME/.espressif}"

missing=()
for command_name in git wget flex bison gperf python3 pip3 cmake ninja dfu-util; do
    if ! command -v "$command_name" >/dev/null 2>&1; then
        missing+=("$command_name")
    fi
done

if [[ "${#missing[@]}" -gt 0 ]]; then
    echo "Faltan dependencias: ${missing[*]}" >&2
    echo "Instalalas en Fedora con:" >&2
    echo "  sudo dnf install -y git wget flex bison gperf python3 python3-pip python3-virtualenv cmake ninja-build ccache dfu-util libusbx-devel openssl-devel libffi-devel" >&2
    exit 1
fi

if [[ -d "$IDF_PATH/.git" ]]; then
    echo "ESP-IDF ya existe en $IDF_PATH"
else
    mkdir -p "$INSTALL_ROOT"
    git clone --branch "$IDF_VERSION" --recursive https://github.com/espressif/esp-idf.git "$IDF_PATH"
fi

export IDF_TOOLS_PATH
"$IDF_PATH/install.sh" esp32

cat <<EOF

ESP-IDF instalado.

Para activar el entorno en esta terminal:
  source "$IDF_PATH/export.sh"

Para usarlo desde este proyecto:
  ./tools/check-esp-env.sh
  ./tools/idf.sh set-target esp32
  ./tools/idf.sh build
EOF

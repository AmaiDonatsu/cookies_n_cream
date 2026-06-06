#!/usr/bin/env bash

set -euo pipefail

echo "Puertos serie candidatos para ESP32:"

found=0
for port in /dev/ttyUSB* /dev/ttyACM* /dev/serial/by-id/*; do
    if [[ -e "$port" ]]; then
        printf -- '- %s\n' "$port"
        found=1
    fi
done

if [[ "$found" -eq 0 ]]; then
    echo "No encontre /dev/ttyUSB*, /dev/ttyACM* ni /dev/serial/by-id/*."
    echo "Conecta la placa y revisa permisos de usuario sobre el puerto serie."
fi

#!/usr/bin/env bash
# tools/import-mqtt-certs.sh

set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "Uso: $0 <ruta/al/archivo_certs.json>"
    exit 1
fi

JSON_PATH="$1"

if [ ! -f "$JSON_PATH" ]; then
    echo "Error: El archivo JSON no existe: $JSON_PATH"
    exit 1
fi

# Extract directory of script
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
CERTS_DIR="$PROJECT_ROOT/main/certs"
MQTT_CONFIG_PATH="$PROJECT_ROOT/main/mqtt_config.h"

# Create certs directory if not exists
mkdir -p "$CERTS_DIR"

# Parse JSON fields using node (since we know node/npm is installed on the developer machine)
NODE_CMD="node -e \"
const fs = require('fs');
const data = JSON.parse(fs.readFileSync('$JSON_PATH', 'utf8'));
if (!data.deviceKey || !data.deviceCert || !data.caCert) {
    console.error('El JSON debe contener deviceKey, deviceCert y caCert.');
    process.exit(1);
}
fs.writeFileSync('$CERTS_DIR/device.key', data.deviceKey);
fs.writeFileSync('$CERTS_DIR/device.crt', data.deviceCert);
fs.writeFileSync('$CERTS_DIR/ca.pem', data.caCert);
\""

if ! eval "$NODE_CMD"; then
    echo "Error al procesar el archivo JSON de certificados."
    exit 1
fi

echo "Certificados MQTT importados en: $CERTS_DIR"
echo "- device.key"
echo "- device.crt"
echo "- ca.pem"

# Extract device ID from JSON filename (e.g. certs_DEV-12345678.json)
FILE_NAME=$(basename "$JSON_PATH")
if [[ $FILE_NAME =~ ^certs_(DEV-[A-Z0-9]+)\.json$ ]]; then
    DEVICE_ID="${BASH_REMATCH[1]}"
    echo ""
    echo "Device ID detectado: $DEVICE_ID"
    echo "Actualiza main/mqtt_config.h con este MQTT_DEVICE_ID si aún no lo has hecho."
    
    # Proactively update main/mqtt_config.h if it exists
    if [ -f "$MQTT_CONFIG_PATH" ]; then
        # Use sed to update MQTT_DEVICE_ID
        sed -i -E "s/#define MQTT_DEVICE_ID \".*\"/#define MQTT_DEVICE_ID \"$DEVICE_ID\"/" "$MQTT_CONFIG_PATH"
        echo "¡Se actualizó automáticamente el MQTT_DEVICE_ID a \"$DEVICE_ID\" en main/mqtt_config.h!"
    fi
fi

if [ -f "$MQTT_CONFIG_PATH" ]; then
    echo ""
    echo "Configuración actual en main/mqtt_config.h:"
    cat "$MQTT_CONFIG_PATH"
fi

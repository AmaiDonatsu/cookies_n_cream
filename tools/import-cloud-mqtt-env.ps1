param(
    [string]$EnvPath = ".env",
    [string]$DeviceId
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-EnvMap {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Path
    )

    $map = @{}

    foreach ($line in Get-Content -LiteralPath $Path) {
        $trimmed = $line.Trim()

        if ([string]::IsNullOrWhiteSpace($trimmed) -or $trimmed.StartsWith("#")) {
            continue
        }

        $parts = $trimmed -split "=", 2
        if ($parts.Count -ne 2) {
            continue
        }

        $key = $parts[0].Trim()
        $value = $parts[1].Trim()

        if (($value.StartsWith('"') -and $value.EndsWith('"')) -or ($value.StartsWith("'") -and $value.EndsWith("'"))) {
            $value = $value.Substring(1, $value.Length - 2)
        }

        $map[$key] = $value
    }

    return $map
}

$projectRoot = Split-Path -Parent $PSScriptRoot
$resolvedEnv = Resolve-Path (Join-Path $projectRoot $EnvPath) -ErrorAction Stop
$mqttConfigPath = Join-Path $projectRoot "main\\mqtt_config.h"
$envMap = Get-EnvMap -Path $resolvedEnv

$requiredKeys = @(
    "MQTT_HOST_CLOUD",
    "MQTT_OVER_TLS_PORT",
    "USER_NAME",
    "PASSWORD"
)

foreach ($key in $requiredKeys) {
    if (-not $envMap.ContainsKey($key) -or [string]::IsNullOrWhiteSpace($envMap[$key])) {
        throw "Falta la clave requerida '$key' en $resolvedEnv."
    }
}

if ([string]::IsNullOrWhiteSpace($DeviceId)) {
    if (Test-Path $mqttConfigPath) {
        $existingDeviceLine = Select-String -Path $mqttConfigPath -Pattern '^#define MQTT_DEVICE_ID "(.+)"$' | Select-Object -First 1
        if ($existingDeviceLine -and $existingDeviceLine.Matches.Count -gt 0) {
            $DeviceId = $existingDeviceLine.Matches[0].Groups[1].Value
        }
    }
}

if ([string]::IsNullOrWhiteSpace($DeviceId)) {
    $DeviceId = "DEV-ESP32C6-001"
}

$brokerUri = "mqtts://{0}:{1}" -f $envMap["MQTT_HOST_CLOUD"], $envMap["MQTT_OVER_TLS_PORT"]

$content = @"
#ifndef MQTT_CONFIG_H
#define MQTT_CONFIG_H

#define MQTT_AUTH_MODE_CLOUD_TLS 1
#define MQTT_AUTH_MODE_LOCAL_MTLS 2

#define MQTT_BROKER_URI "$brokerUri"
#define MQTT_DEVICE_ID "$DeviceId"
#define MQTT_PUBLISH_INTERVAL_MS 5000
#define MQTT_AUTH_MODE MQTT_AUTH_MODE_CLOUD_TLS
#define MQTT_TLS_USE_CRT_BUNDLE 1
#define MQTT_USERNAME "$($envMap["USER_NAME"])"
#define MQTT_PASSWORD "$($envMap["PASSWORD"])"

#endif // MQTT_CONFIG_H
"@

Set-Content -LiteralPath $mqttConfigPath -Value $content -Encoding utf8

Write-Host "Configuracion MQTT en nube generada en: $mqttConfigPath"
Write-Host "Broker: $brokerUri"
Write-Host "Device ID: $DeviceId"
Write-Host "Usuario MQTT: $($envMap["USER_NAME"])"

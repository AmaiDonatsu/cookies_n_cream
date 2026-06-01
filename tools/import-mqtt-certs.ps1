param(
    [Parameter(Mandatory = $true)]
    [string]$JsonPath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$projectRoot = Split-Path -Parent $PSScriptRoot
$resolvedJson = Resolve-Path $JsonPath -ErrorAction Stop
$certsDir = Join-Path $projectRoot "main\\certs"
$mqttConfigPath = Join-Path $projectRoot "main\\mqtt_config.h"

$certsJson = Get-Content $resolvedJson -Raw | ConvertFrom-Json

if (-not $certsJson.deviceKey -or -not $certsJson.deviceCert -or -not $certsJson.caCert) {
    throw "El JSON debe contener deviceKey, deviceCert y caCert."
}

$null = New-Item -ItemType Directory -Path $certsDir -Force

Set-Content -LiteralPath (Join-Path $certsDir "device.key") -Value $certsJson.deviceKey -Encoding utf8
Set-Content -LiteralPath (Join-Path $certsDir "device.crt") -Value $certsJson.deviceCert -Encoding utf8
Set-Content -LiteralPath (Join-Path $certsDir "ca.pem") -Value $certsJson.caCert -Encoding utf8

$deviceId = $null
$match = [regex]::Match((Split-Path $resolvedJson -Leaf), '^certs_(DEV-[A-Z0-9]+)\.json$')
if ($match.Success) {
    $deviceId = $match.Groups[1].Value
}

Write-Host "Certificados MQTT importados en: $certsDir"
Write-Host "- device.key"
Write-Host "- device.crt"
Write-Host "- ca.pem"

if ($deviceId) {
    Write-Host ""
    Write-Host "Device ID detectado: $deviceId"
    Write-Host "Actualiza main\\mqtt_config.h con ese MQTT_DEVICE_ID si aun no lo hiciste."
}

if (Test-Path $mqttConfigPath) {
    Write-Host ""
    Write-Host "Configuracion actual en main\\mqtt_config.h:"
    Get-Content $mqttConfigPath
}

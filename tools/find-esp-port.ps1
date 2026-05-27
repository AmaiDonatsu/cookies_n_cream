Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

$ports = @([System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object)

Write-Host "=== Puertos COM detectados ==="
if ($ports.Count -eq 0) {
    Write-Host "No se detecto ningun puerto COM."
    Write-Host "Si tu ESP32 esta conectado, revisa cable, drivers y Administrador de dispositivos."
    exit 1
}

$ports | ForEach-Object {
    Write-Host ("- {0}" -f $_)
}

Write-Host ""
Write-Host "Usa ese valor con:"
Write-Host ".\tools\idf.ps1 -Action flash-monitor -Port COMx"

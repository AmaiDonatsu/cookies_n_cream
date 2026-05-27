Set-StrictMode -Version Latest
$ErrorActionPreference = "SilentlyContinue"

function Get-EnvOrDefault {
    param(
        [AllowNull()]
        [string]$Value,
        [Parameter(Mandatory = $true)]
        [string]$DefaultValue
    )

    if ([string]::IsNullOrWhiteSpace($Value)) {
        return $DefaultValue
    }

    return $Value
}

function Show-CommandStatus {
    param(
        [Parameter(Mandatory = $true)]
        [string]$Name
    )

    $cmd = Get-Command $Name -ErrorAction SilentlyContinue
    if ($cmd) {
        Write-Host ("[OK]     {0} -> {1}" -f $Name, $cmd.Source)
    }
    else {
        Write-Host ("[FALTA]  {0}" -f $Name)
    }
}

Write-Host "=== Revision rapida del entorno ESP-IDF ==="
Write-Host ""

Show-CommandStatus -Name "idf.py"
Show-CommandStatus -Name "git"
Show-CommandStatus -Name "cmake"
Show-CommandStatus -Name "ninja"
Show-CommandStatus -Name "py"

Write-Host ""
Write-Host "Variables de entorno:"
Write-Host ("IDF_PATH={0}" -f (Get-EnvOrDefault -Value $env:IDF_PATH -DefaultValue "<sin definir>"))
Write-Host ("IDF_TOOLS_PATH={0}" -f (Get-EnvOrDefault -Value $env:IDF_TOOLS_PATH -DefaultValue "<sin definir>"))

Write-Host ""
Write-Host "Launchers de Python detectados:"
try {
    & py -0p
}
catch {
    Write-Host "No fue posible consultar el launcher 'py'."
}

Write-Host ""
Write-Host "Puertos serie detectados:"
$ports = @([System.IO.Ports.SerialPort]::GetPortNames() | Sort-Object)
if ($ports.Count -gt 0) {
    $ports | ForEach-Object { Write-Host ("- {0}" -f $_) }
}
else {
    Write-Host "No se detectaron puertos COM desde PowerShell."
}

Write-Host ""
if (-not (Get-Command "idf.py" -ErrorAction SilentlyContinue)) {
    Write-Host "Siguiente paso: instala ESP-IDF con EIM y abre 'ESP-IDF PowerShell'."
}

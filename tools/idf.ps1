param(
    [ValidateSet("set-target", "build", "flash", "monitor", "flash-monitor", "menuconfig", "fullclean")]
    [string]$Action = "build",
    [string]$Port,
    [string]$Target = "esp32"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

. "$PSScriptRoot\esp-idf-env.ps1"

if (-not (Import-EspIdfEnvironment -Quiet)) {
    Write-Error "No encuentro 'idf.py'. Instala ESP-IDF con EIM o abre este proyecto desde 'ESP-IDF PowerShell'."
}

switch ($Action) {
    "set-target" {
        & idf.py set-target $Target
    }
    "build" {
        & idf.py build
    }
    "flash" {
        if (-not $Port) {
            throw "Debes indicar -Port COMx para flashear."
        }
        & idf.py -p $Port flash
    }
    "monitor" {
        if (-not $Port) {
            throw "Debes indicar -Port COMx para abrir el monitor."
        }
        & idf.py -p $Port monitor
    }
    "flash-monitor" {
        if (-not $Port) {
            throw "Debes indicar -Port COMx para flashear y monitorear."
        }
        & idf.py -p $Port flash monitor
    }
    "menuconfig" {
        & idf.py menuconfig
    }
    "fullclean" {
        & idf.py fullclean
    }
}

param(
    [ValidateSet("set-target", "build", "flash", "monitor", "flash-monitor", "menuconfig", "fullclean")]
    [string]$Action = "build",
    [string]$Port,
    [string]$Target = "esp32"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

if (-not (Get-Command "idf.py" -ErrorAction SilentlyContinue)) {
    Write-Error "No encuentro 'idf.py'. Abre este proyecto desde 'ESP-IDF PowerShell' despues de instalar ESP-IDF."
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

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-EspIdfActivationScript {
    $candidates = @(
        "C:\Espressif\tools\eim_idf.json",
        "$HOME\.espressif\tools\eim_idf.json"
    )

    foreach ($candidate in $candidates) {
        if (-not (Test-Path $candidate)) {
            continue
        }

        $config = Get-Content $candidate -Raw | ConvertFrom-Json
        $selectedId = $config.idfSelectedId
        $selected = $config.idfInstalled | Where-Object { $_.id -eq $selectedId } | Select-Object -First 1

        if (-not $selected) {
            $selected = $config.idfInstalled | Select-Object -First 1
        }

        if ($selected -and (Test-Path $selected.activationScript)) {
            return $selected.activationScript
        }
    }

    return $null
}

function Import-EspIdfEnvironment {
    param(
        [switch]$Quiet
    )

    if ($env:IDF_PATH -and $env:IDF_TOOLS_PATH -and (Get-Command "idf.py" -ErrorAction SilentlyContinue)) {
        return $true
    }

    $activationScript = Get-EspIdfActivationScript
    if (-not $activationScript) {
        if (-not $Quiet) {
            Write-Host "No encontre el script de activacion de ESP-IDF."
        }
        return $false
    }

    $rawEnv = & pwsh -NoProfile -File $activationScript -e
    $espPath = $null
    $systemPath = $null

    foreach ($line in $rawEnv) {
        if ($line -notmatch "^[A-Z0-9_]+=.*$") {
            continue
        }

        $name, $value = $line -split "=", 2
        switch ($name) {
            "PATH" { $espPath = $value }
            "SYSTEM_PATH" { $systemPath = $value }
            default { Set-Item -Path ("env:{0}" -f $name) -Value $value }
        }
    }

    if ($espPath -and $systemPath) {
        $env:PATH = "{0};{1}" -f $espPath, $systemPath
    }
    elseif ($espPath) {
        $env:PATH = $espPath
    }

    if (-not $Quiet) {
        Write-Host ("ESP-IDF activado desde {0}" -f $activationScript)
    }

    return [bool](Get-Command "idf.py" -ErrorAction SilentlyContinue)
}

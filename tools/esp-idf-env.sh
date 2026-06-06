#!/usr/bin/env bash

set -euo pipefail

find_idf_path() {
    local candidates=(
        "${IDF_PATH:-}"
        "$HOME/esp/v6.0.1/esp-idf"
        "$HOME/esp/esp-idf"
        "$HOME/esp-idf"
    )

    local candidate
    for candidate in "${candidates[@]}"; do
        if [[ -n "$candidate" && -f "$candidate/export.sh" ]]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done

    return 1
}

import_esp_idf_environment() {
    if command -v idf.py >/dev/null 2>&1 && [[ -n "${IDF_PATH:-}" ]]; then
        return 0
    fi

    local idf_path
    if ! idf_path="$(find_idf_path)"; then
        return 1
    fi

    # shellcheck source=/dev/null
    source "$idf_path/export.sh" >/dev/null
}

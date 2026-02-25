#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_notification_count() {
    local file_path="$1"
    if [[ ! -f "$file_path" ]]; then
        echo "0"
        return
    fi
    wc -l <"$file_path" | tr -d ' '
}

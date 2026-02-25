#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_input_probe_value() {
    local key="$1"
    local file_path="$2"
    sed -n "s/^${key}=//p" "$file_path" | head -n 1
}

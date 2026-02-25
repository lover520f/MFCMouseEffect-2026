#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_input_write_permission_sim_state() {
    local file_path="$1"
    local trusted="$2"
    printf 'trusted=%s\n' "$trusted" >"$file_path"
}

#!/usr/bin/env bash

set -euo pipefail

_mfx_wasm_selfcheck_common_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$_mfx_wasm_selfcheck_common_dir/../../regression/lib/wasm_fixture_helpers.sh"
source "$_mfx_wasm_selfcheck_common_dir/wasm_selfcheck_parse_helpers.sh"
source "$_mfx_wasm_selfcheck_common_dir/wasm_selfcheck_http_helpers.sh"
source "$_mfx_wasm_selfcheck_common_dir/wasm_selfcheck_assert_helpers.sh"

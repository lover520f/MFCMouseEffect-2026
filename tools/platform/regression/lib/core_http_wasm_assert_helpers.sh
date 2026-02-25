#!/usr/bin/env bash

set -euo pipefail

_mfx_core_http_wasm_assert_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$_mfx_core_http_wasm_assert_dir/core_http_wasm_runtime_assert_helpers.sh"
source "$_mfx_core_http_wasm_assert_dir/core_http_wasm_transfer_assert_helpers.sh"
source "$_mfx_core_http_wasm_assert_dir/core_http_wasm_dispatch_assert_helpers.sh"

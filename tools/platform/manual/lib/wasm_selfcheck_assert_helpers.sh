#!/usr/bin/env bash

set -euo pipefail

_mfx_wasm_selfcheck_assert_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$_mfx_wasm_selfcheck_assert_dir/wasm_selfcheck_runtime_assert_helpers.sh"
source "$_mfx_wasm_selfcheck_assert_dir/wasm_selfcheck_transfer_assert_helpers.sh"
source "$_mfx_wasm_selfcheck_assert_dir/wasm_selfcheck_dispatch_assert_helpers.sh"

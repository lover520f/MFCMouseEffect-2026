#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/lib/common.sh"

mfx_reject_option_in_args \
    "--core-automation-check-scope" \
    "run-posix-wasm-regression-suite.sh enforces --core-automation-check-scope wasm; do not pass --core-automation-check-scope" \
    "$@"

exec "$SCRIPT_DIR/run-posix-regression-suite.sh" \
    --core-automation-check-scope wasm \
    --skip-macos-automation-injection-selfcheck \
    --skip-macos-automation-app-scope-selfcheck \
    --skip-macos-vm-suppression-selfcheck \
    --skip-macos-effects-tuning-selfcheck \
    "$@"

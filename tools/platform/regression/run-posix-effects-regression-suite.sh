#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/lib/common.sh"

mfx_reject_option_in_args \
    "--core-automation-check-scope" \
    "run-posix-effects-regression-suite.sh enforces --core-automation-check-scope effects; do not pass --core-automation-check-scope" \
    "$@"

exec "$SCRIPT_DIR/run-posix-regression-suite.sh" \
    --core-automation-check-scope effects \
    "$@"

#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/lib/common.sh"

mfx_reject_option_in_args \
    "--check-scope" \
    "run-posix-core-effects-contract-regression.sh enforces --check-scope effects; do not pass --check-scope" \
    "$@"

exec "$SCRIPT_DIR/run-posix-core-automation-contract-regression.sh" --check-scope effects "$@"

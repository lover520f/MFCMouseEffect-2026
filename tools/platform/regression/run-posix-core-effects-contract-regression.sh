#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/lib/common.sh"

mfx_reject_option_in_args \
    "--check-scope" \
    "run-posix-core-effects-contract-regression.sh enforces --check-scope effects; do not pass --check-scope" \
    "$@"

for arg in "$@"; do
    if [[ "$arg" == "-h" || "$arg" == "--help" ]]; then
        cat <<'USAGE'
Usage: run-posix-core-effects-contract-regression.sh [options]
  Wrapper over run-posix-core-automation-contract-regression.sh with --check-scope effects.

Defaults injected by this wrapper (override by exporting env before run):
  MFX_TEST_EFFECTS_DURATION_SCALE=0.5
  MFX_TEST_EFFECTS_SIZE_SCALE=1.2
  MFX_TEST_EFFECTS_OPACITY_SCALE=0.78
  MFX_TEST_EFFECTS_TRAIL_THROTTLE_SCALE=0.6
  MFX_EXPECT_EFFECTS_* defaults to corresponding MFX_TEST_EFFECTS_* value

Forwarded options:
  --platform <auto|macos|linux>
  --build-dir <path>
USAGE
        exit 0
    fi
done

export MFX_TEST_EFFECTS_DURATION_SCALE="${MFX_TEST_EFFECTS_DURATION_SCALE:-0.5}"
export MFX_TEST_EFFECTS_SIZE_SCALE="${MFX_TEST_EFFECTS_SIZE_SCALE:-1.2}"
export MFX_TEST_EFFECTS_OPACITY_SCALE="${MFX_TEST_EFFECTS_OPACITY_SCALE:-0.78}"
export MFX_TEST_EFFECTS_TRAIL_THROTTLE_SCALE="${MFX_TEST_EFFECTS_TRAIL_THROTTLE_SCALE:-0.6}"

export MFX_EXPECT_EFFECTS_DURATION_SCALE="${MFX_EXPECT_EFFECTS_DURATION_SCALE:-$MFX_TEST_EFFECTS_DURATION_SCALE}"
export MFX_EXPECT_EFFECTS_SIZE_SCALE="${MFX_EXPECT_EFFECTS_SIZE_SCALE:-$MFX_TEST_EFFECTS_SIZE_SCALE}"
export MFX_EXPECT_EFFECTS_OPACITY_SCALE="${MFX_EXPECT_EFFECTS_OPACITY_SCALE:-$MFX_TEST_EFFECTS_OPACITY_SCALE}"
export MFX_EXPECT_EFFECTS_TRAIL_THROTTLE_SCALE="${MFX_EXPECT_EFFECTS_TRAIL_THROTTLE_SCALE:-$MFX_TEST_EFFECTS_TRAIL_THROTTLE_SCALE}"

exec "$SCRIPT_DIR/run-posix-core-automation-contract-regression.sh" --check-scope effects "$@"

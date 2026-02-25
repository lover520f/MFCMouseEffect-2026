#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

source "$SCRIPT_DIR/lib/common.sh"
source "$SCRIPT_DIR/lib/linux_gate.sh"

MFX_BUILD_DIR="/tmp/mfx-platform-linux-build"
MFX_INCLUDE_CORE_RUNTIME=1

while [[ $# -gt 0 ]]; do
    case "$1" in
        --build-dir)
            MFX_BUILD_DIR="${2:-}"
            shift 2
            ;;
        --jobs)
            MFX_BUILD_JOBS="${2:-}"
            shift 2
            ;;
        --skip-core-runtime)
            MFX_INCLUDE_CORE_RUNTIME=0
            shift
            ;;
        --include-core-runtime)
            MFX_INCLUDE_CORE_RUNTIME=1
            shift
            ;;
        -h|--help)
            cat <<'USAGE'
Usage: run-posix-linux-compile-gate.sh [options]
  --build-dir <path>      linux package build directory (default: /tmp/mfx-platform-linux-build)
  --jobs <n>              build jobs override (default: env MFX_BUILD_JOBS or 8)
  --skip-core-runtime     only build default lane (MFX_ENABLE_POSIX_CORE_RUNTIME=OFF)
  --include-core-runtime  build both default lane and core-runtime lane (default)
USAGE
            exit 0
            ;;
        *)
            mfx_fail "unknown argument: $1"
            ;;
    esac
done

mfx_require_cmd cmake

mfx_info "repo root: $REPO_ROOT"
mfx_info "host platform: $(uname -s)"
mfx_info "linux gate build dir: $MFX_BUILD_DIR"
mfx_info "linux gate include core runtime lane: $MFX_INCLUDE_CORE_RUNTIME"

mfx_run_linux_compile_gate "$REPO_ROOT" "$MFX_BUILD_DIR" "$MFX_INCLUDE_CORE_RUNTIME"

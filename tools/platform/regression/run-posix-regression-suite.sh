#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

source "$SCRIPT_DIR/lib/common.sh"

MFX_PLATFORM="auto"
MFX_SCAFFOLD_BUILD_DIR=""
MFX_CORE_BUILD_DIR=""
MFX_CORE_AUTOMATION_BUILD_DIR=""
MFX_LINUX_BUILD_DIR="/tmp/mfx-platform-linux-build"
MFX_BUILD_JOBS_VALUE=""

MFX_SKIP_SCAFFOLD=0
MFX_SKIP_CORE_SMOKE=0
MFX_SKIP_CORE_AUTOMATION=0
MFX_SKIP_MACOS_AUTOMATION_INJECTION_SELFCHECK=0
MFX_SKIP_LINUX_GATE=0
MFX_LINUX_SKIP_CORE_RUNTIME=0
MFX_SKIP_AUTOMATION_TEST=0
MFX_SKIP_MACOS_WASM_SELFCHECK=0
MFX_SCAFFOLD_SKIP_SMOKE=0
MFX_SCAFFOLD_SKIP_HTTP=0

while [[ $# -gt 0 ]]; do
    case "$1" in
        --platform)
            MFX_PLATFORM="${2:-}"
            shift 2
            ;;
        --scaffold-build-dir)
            MFX_SCAFFOLD_BUILD_DIR="${2:-}"
            shift 2
            ;;
        --core-build-dir)
            MFX_CORE_BUILD_DIR="${2:-}"
            shift 2
            ;;
        --core-automation-build-dir)
            MFX_CORE_AUTOMATION_BUILD_DIR="${2:-}"
            shift 2
            ;;
        --linux-build-dir)
            MFX_LINUX_BUILD_DIR="${2:-}"
            shift 2
            ;;
        --jobs)
            MFX_BUILD_JOBS_VALUE="${2:-}"
            shift 2
            ;;
        --skip-scaffold)
            MFX_SKIP_SCAFFOLD=1
            shift
            ;;
        --skip-core-smoke)
            MFX_SKIP_CORE_SMOKE=1
            shift
            ;;
        --skip-core-automation)
            MFX_SKIP_CORE_AUTOMATION=1
            shift
            ;;
        --skip-macos-automation-injection-selfcheck)
            MFX_SKIP_MACOS_AUTOMATION_INJECTION_SELFCHECK=1
            shift
            ;;
        --skip-linux-gate)
            MFX_SKIP_LINUX_GATE=1
            shift
            ;;
        --linux-skip-core-runtime)
            MFX_LINUX_SKIP_CORE_RUNTIME=1
            shift
            ;;
        --skip-automation-test)
            MFX_SKIP_AUTOMATION_TEST=1
            shift
            ;;
        --skip-macos-wasm-selfcheck)
            MFX_SKIP_MACOS_WASM_SELFCHECK=1
            shift
            ;;
        --scaffold-skip-smoke)
            MFX_SCAFFOLD_SKIP_SMOKE=1
            shift
            ;;
        --scaffold-skip-http)
            MFX_SCAFFOLD_SKIP_HTTP=1
            shift
            ;;
        -h|--help)
            cat <<'USAGE'
Usage: run-posix-regression-suite.sh [options]
  --platform <auto|macos|linux>   scaffold package platform (default: auto)
  --scaffold-build-dir <path>     scaffold regression build dir override
  --core-build-dir <path>         core-lane smoke build dir override
  --core-automation-build-dir <path> core automation HTTP contract build dir override
  --linux-build-dir <path>        linux compile gate build dir (default: /tmp/mfx-platform-linux-build)
  --jobs <n>                      build jobs override (default: env MFX_BUILD_JOBS or 8)
  --skip-scaffold                 skip scaffold regression phase
  --skip-core-smoke               skip core-lane smoke phase
  --skip-core-automation          skip core automation HTTP contract phase
  --skip-macos-automation-injection-selfcheck skip macOS automation injection selfcheck phase
  --skip-linux-gate               skip linux compile gate phase
  --linux-skip-core-runtime       forward: linux gate skips core-runtime lane compile
  --skip-automation-test          skip webui automation platform semantic tests
  --skip-macos-wasm-selfcheck     skip macOS wasm runtime selfcheck phase
  --scaffold-skip-smoke           forward: skip scaffold smoke checks
  --scaffold-skip-http            forward: skip scaffold HTTP checks
USAGE
            exit 0
            ;;
        *)
            mfx_fail "unknown argument: $1"
            ;;
    esac
done

if [[ -n "$MFX_BUILD_JOBS_VALUE" ]]; then
    export MFX_BUILD_JOBS="$MFX_BUILD_JOBS_VALUE"
fi

mfx_info "repo root: $REPO_ROOT"
mfx_info "host platform: $(uname -s)"

if command -v pgrep >/dev/null 2>&1; then
    if pgrep -f mfx_entry_posix_host >/dev/null 2>&1; then
        mfx_info "detected running mfx_entry_posix_host; phase scripts will handle cleanup under mfx-entry-posix-host lock"
    fi
fi

if [[ "$MFX_SKIP_SCAFFOLD" -eq 0 ]]; then
    local_scaffold_args=("--platform" "$MFX_PLATFORM")
    if [[ -n "$MFX_SCAFFOLD_BUILD_DIR" ]]; then
        local_scaffold_args+=("--build-dir" "$MFX_SCAFFOLD_BUILD_DIR")
    fi
    if [[ "$MFX_SCAFFOLD_SKIP_SMOKE" -eq 1 ]]; then
        local_scaffold_args+=("--skip-smoke")
    fi
    if [[ "$MFX_SCAFFOLD_SKIP_HTTP" -eq 1 ]]; then
        local_scaffold_args+=("--skip-http")
    fi

    mfx_info "run scaffold regression phase"
    "$SCRIPT_DIR/run-posix-scaffold-regression.sh" "${local_scaffold_args[@]}"
else
    mfx_info "skip scaffold regression phase"
fi

if [[ "$MFX_SKIP_CORE_SMOKE" -eq 0 ]]; then
    local_core_args=("--platform" "$MFX_PLATFORM")
    if [[ -n "$MFX_CORE_BUILD_DIR" ]]; then
        local_core_args+=("--build-dir" "$MFX_CORE_BUILD_DIR")
    fi

    mfx_info "run core-lane smoke phase"
    "$SCRIPT_DIR/run-posix-core-smoke.sh" "${local_core_args[@]}"
else
    mfx_info "skip core-lane smoke phase"
fi

if [[ "$MFX_SKIP_CORE_AUTOMATION" -eq 0 ]]; then
    local_core_automation_args=("--platform" "$MFX_PLATFORM")
    if [[ -n "$MFX_CORE_AUTOMATION_BUILD_DIR" ]]; then
        local_core_automation_args+=("--build-dir" "$MFX_CORE_AUTOMATION_BUILD_DIR")
    elif [[ -n "$MFX_CORE_BUILD_DIR" ]]; then
        local_core_automation_args+=("--build-dir" "$MFX_CORE_BUILD_DIR")
    fi

    mfx_info "run core automation contract phase"
    "$SCRIPT_DIR/run-posix-core-automation-contract-regression.sh" "${local_core_automation_args[@]}"
else
    mfx_info "skip core automation contract phase"
fi

if [[ "$MFX_SKIP_MACOS_AUTOMATION_INJECTION_SELFCHECK" -eq 0 ]]; then
    if [[ "$(uname -s)" == "Darwin" ]]; then
        local_automation_inject_selfcheck_args=("--skip-build" "--dry-run")
        if [[ -n "$MFX_CORE_AUTOMATION_BUILD_DIR" ]]; then
            local_automation_inject_selfcheck_args+=("--build-dir" "$MFX_CORE_AUTOMATION_BUILD_DIR")
        elif [[ -n "$MFX_CORE_BUILD_DIR" ]]; then
            local_automation_inject_selfcheck_args+=("--build-dir" "$MFX_CORE_BUILD_DIR")
        fi

        mfx_info "run macos automation injection selfcheck phase"
        "$REPO_ROOT/tools/platform/manual/run-macos-automation-injection-selfcheck.sh" \
            "${local_automation_inject_selfcheck_args[@]}"
    else
        mfx_info "skip macos automation injection selfcheck phase (non-macos host)"
    fi
else
    mfx_info "skip macos automation injection selfcheck phase"
fi

if [[ "$MFX_SKIP_MACOS_WASM_SELFCHECK" -eq 0 ]]; then
    if [[ "$(uname -s)" == "Darwin" ]]; then
        local_wasm_selfcheck_args=("--skip-build")
        if [[ -n "$MFX_CORE_AUTOMATION_BUILD_DIR" ]]; then
            local_wasm_selfcheck_args+=("--build-dir" "$MFX_CORE_AUTOMATION_BUILD_DIR")
        elif [[ -n "$MFX_CORE_BUILD_DIR" ]]; then
            local_wasm_selfcheck_args+=("--build-dir" "$MFX_CORE_BUILD_DIR")
        fi

        mfx_info "run macos wasm runtime selfcheck phase"
        "$REPO_ROOT/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh" "${local_wasm_selfcheck_args[@]}"
    else
        mfx_info "skip macos wasm runtime selfcheck phase (non-macos host)"
    fi
else
    mfx_info "skip macos wasm runtime selfcheck phase"
fi

if [[ "$MFX_SKIP_LINUX_GATE" -eq 0 ]]; then
    local_linux_args=("--build-dir" "$MFX_LINUX_BUILD_DIR")
    if [[ -n "$MFX_BUILD_JOBS_VALUE" ]]; then
        local_linux_args+=("--jobs" "$MFX_BUILD_JOBS_VALUE")
    fi
    if [[ "$MFX_LINUX_SKIP_CORE_RUNTIME" -eq 1 ]]; then
        local_linux_args+=("--skip-core-runtime")
    fi

    mfx_info "run linux compile gate phase"
    "$SCRIPT_DIR/run-posix-linux-compile-gate.sh" "${local_linux_args[@]}"
else
    mfx_info "skip linux compile gate phase"
fi

if [[ "$MFX_SKIP_AUTOMATION_TEST" -eq 0 ]]; then
    mfx_require_cmd pnpm
    mfx_info "run webui semantic test phase"
    (
        cd "$REPO_ROOT"
        pnpm --dir MFCMouseEffect/WebUIWorkspace run test:automation-platform
        pnpm --dir MFCMouseEffect/WebUIWorkspace run test:effects-profile-model
        pnpm --dir MFCMouseEffect/WebUIWorkspace run test:wasm-error-model
    )
else
    mfx_info "skip webui semantic test phase"
fi

mfx_ok "posix regression suite passed"

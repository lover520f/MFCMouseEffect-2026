#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"

source "$SCRIPT_DIR/lib/common.sh"
source "$SCRIPT_DIR/lib/posix_suite_options.sh"
source "$SCRIPT_DIR/lib/posix_suite_phases.sh"

mfx_posix_suite_init_defaults
mfx_posix_suite_parse_args "$@"
mfx_posix_suite_export_build_jobs

mfx_info "repo root: $REPO_ROOT"
mfx_info "host platform: $(uname -s)"
mfx_info "suite lock: mfx-posix-regression-suite"

mfx_run_posix_regression_suite_workflow() {
    mfx_posix_suite_run_objcxx_gate_phase "$REPO_ROOT"
    mfx_posix_suite_run_macos_objcxx_surface_gate_phase "$SCRIPT_DIR"
    mfx_posix_suite_log_entry_host_presence
    mfx_posix_suite_run_scaffold_phase "$SCRIPT_DIR"
    mfx_posix_suite_run_core_smoke_phase "$SCRIPT_DIR"
    mfx_posix_suite_run_core_automation_phase "$SCRIPT_DIR"
    mfx_posix_suite_run_macos_automation_injection_selfcheck_phase "$REPO_ROOT"
    mfx_posix_suite_run_macos_automation_app_scope_selfcheck_phase "$REPO_ROOT"
    mfx_posix_suite_run_macos_vm_suppression_selfcheck_phase "$REPO_ROOT"
    mfx_posix_suite_run_macos_effects_tuning_selfcheck_phase "$REPO_ROOT"
    mfx_posix_suite_run_macos_wasm_selfcheck_phase "$REPO_ROOT"
    mfx_posix_suite_run_linux_gate_phase "$SCRIPT_DIR"
    mfx_posix_suite_run_webui_semantic_phase "$REPO_ROOT"
}

suite_lock_timeout_seconds="${MFX_POSIX_SUITE_LOCK_TIMEOUT_SECONDS:-600}"
mfx_with_lock "mfx-posix-regression-suite" "$suite_lock_timeout_seconds" mfx_run_posix_regression_suite_workflow

mfx_ok "posix regression suite passed"

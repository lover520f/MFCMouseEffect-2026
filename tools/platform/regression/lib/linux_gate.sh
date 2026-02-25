#!/usr/bin/env bash

set -euo pipefail

_mfx_run_linux_compile_mode() {
    local repo_root="$1"
    local build_dir="$2"
    local core_runtime_flag="$3"
    local mode_label="$4"

    local targets=("mfx_shell_linux" "mfx_entry_posix")

    mfx_info "configure linux package ($mode_label, core_runtime=$core_runtime_flag)"
    cmake -S "$repo_root/MFCMouseEffect/Platform" -B "$build_dir" \
        -DMFX_PACKAGE_PLATFORM=linux \
        -DMFX_ENABLE_CROSS_HOST_PACKAGES=ON \
        -DMFX_ENABLE_ENTRY_RUNTIME_TARGETS=ON \
        -DMFX_ENABLE_POSIX_CORE_RUNTIME="$core_runtime_flag"

    mfx_info "build targets ($mode_label): ${targets[*]}"
    cmake --build "$build_dir" --target "${targets[@]}" -j"${MFX_BUILD_JOBS:-8}"
}

mfx_run_linux_compile_gate() {
    local repo_root="$1"
    local build_dir="$2"
    local include_core_runtime="${3:-1}"

    _mfx_run_linux_compile_mode "$repo_root" "$build_dir" "OFF" "default-lane"
    if [[ "$include_core_runtime" == "1" ]]; then
        _mfx_run_linux_compile_mode "$repo_root" "${build_dir}-core-runtime" "ON" "core-runtime-lane"
    fi
    mfx_ok "linux compile gate passed"
}

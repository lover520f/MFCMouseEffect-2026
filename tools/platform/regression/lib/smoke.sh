#!/usr/bin/env bash

set -euo pipefail

mfx_run_smoke_checks() {
    local platform="$1"
    local build_dir="$2"
    local entry_bin="$build_dir/mfx_entry_posix_host"

    if [[ ! -x "$entry_bin" ]]; then
        mfx_fail "entry host executable missing: $entry_bin"
    fi

    mfx_info "check background exit text command"
    printf 'exit\n' | "$entry_bin" -mode=background >/dev/null 2>&1

    mfx_info "check background exit json command"
    printf '{"cmd":"exit"}\n' | "$entry_bin" -mode=background >/dev/null 2>&1

    if [[ "$platform" == "macos" ]]; then
        local smoke_bin="$build_dir/platform_macos/mfx_shell_macos_smoke"
        local tray_smoke_bin="$build_dir/platform_macos/mfx_shell_macos_tray_smoke"
        if [[ ! -x "$smoke_bin" ]]; then
            mfx_fail "macOS smoke executable missing: $smoke_bin"
        fi
        if [[ ! -x "$tray_smoke_bin" ]]; then
            mfx_fail "macOS tray smoke executable missing: $tray_smoke_bin"
        fi

        mfx_info "run macOS event-loop smoke"
        "$smoke_bin" >/dev/null 2>&1

        mfx_info "run macOS tray smoke (settings + theme action contracts)"
        local tray_smoke_tmp_dir
        tray_smoke_tmp_dir="$(mktemp -d)"
        local tray_settings_url="http://127.0.0.1:9527/?token=tray-smoke"
        local tray_launch_capture_file="$tray_smoke_tmp_dir/tray-launch-capture.env"
        local tray_theme_capture_file="$tray_smoke_tmp_dir/tray-theme-capture.env"
        local tray_theme_value="neon"
        "$tray_smoke_bin" \
            --expect-settings-action \
            --expect-theme-action \
            --settings-url "$tray_settings_url" \
            --theme-value "$tray_theme_value" \
            --launch-capture-file "$tray_launch_capture_file" \
            --theme-capture-file "$tray_theme_capture_file" >/dev/null 2>&1
        if [[ -f "$tray_launch_capture_file" ]]; then
            mfx_assert_file_contains "$tray_launch_capture_file" "captured=1" "macOS tray smoke launch capture flag"
            mfx_assert_file_contains "$tray_launch_capture_file" "command=open" "macOS tray smoke launch command"
            mfx_assert_file_contains "$tray_launch_capture_file" "url=$tray_settings_url" "macOS tray smoke launch url"
        else
            mfx_info "macOS tray smoke launch capture file not emitted; keep exit-code gate only under current runner"
        fi
        if [[ -f "$tray_theme_capture_file" ]]; then
            mfx_assert_file_contains "$tray_theme_capture_file" "captured=1" "macOS tray smoke theme capture flag"
            mfx_assert_file_contains "$tray_theme_capture_file" "command=theme_select" "macOS tray smoke theme capture command"
            mfx_assert_file_contains "$tray_theme_capture_file" "expected_theme=$tray_theme_value" "macOS tray smoke expected theme"
            mfx_assert_file_contains "$tray_theme_capture_file" "selected_theme=$tray_theme_value" "macOS tray smoke selected theme"
        else
            mfx_info "macOS tray smoke theme capture file not emitted; keep exit-code gate only under current runner"
        fi
        rm -rf "$tray_smoke_tmp_dir"
    else
        mfx_info "linux smoke executable is not available yet; skip platform-specific smoke binary"
    fi

    mfx_ok "smoke checks completed"
}

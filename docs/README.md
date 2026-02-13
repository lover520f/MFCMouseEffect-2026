# MFCMouseEffect Documentation

Language: [English](README.md) | [中文](README.zh-CN.md)

## Doc Index
- Marketing: `docs/marketing/readme_language_switch.md` (README language toggle rendering)
- Issues: `docs/issues/emoji-support.md` (emoji rendering in settings + text click effect)
- Issues: `docs/issues/text-effect-motion.md` (floating text motion parity after DWrite switch)
- Issues: `docs/issues/settings-emoji-preview.md` (settings emoji color preview overlay)
- Issues: `docs/issues/web-settings-server-lifecycle.md` (web settings idle restart crash + token rotation)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage1.md` (archive Dawn takeover route and switch GPU final-present strategy to D3D11+DirectComposition)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage2.md` (wire D3D11+DComp presenter lifecycle into overlay host while keeping layered present path)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage3_observability.md` (expose D3D11+DComp host readiness into web settings state for diagnosis)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage4_probe_target.md` (validate hidden DComp target/root-visual commit chain and expose takeover readiness flags)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage5_local_state_snapshot.md` (persist latest `/api/state` to local diag file for deterministic post-test analysis)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage6_takeover_file_switch.md` (add local file takeover switch and expose takeover control source)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage7_safe_takeover_trial.md` (add one-shot takeover trial entry and fallback counters while keeping layered present path)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage8_auto_fuse_off.md` (auto-write takeover off marker after fallback and reuse it as next-run safety fuse)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage9_auto_off_freshness.md` (ignore stale auto-off marker after newer build and expose control decision detail)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage10_stale_marker_archive.md` (archive stale auto-off marker after it is ignored by newer build)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage11_probe_swapchain_trial.md` (validate hidden DComp swapchain content path with one-shot probe present)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage12_visible_trial_gate.md` (wire hard-gated visible-host trial path while keeping layered fallback authoritative)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage13_trial_result_snapshot.md` (persist structured takeover trial outcome snapshot for fast post-test diagnosis)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage14_rearm_file_retry.md` (consume local rearm file to clear auto-off marker and allow controlled retry)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage15_control_module_split.md` (extract takeover control/file-env policy into dedicated GPU control module)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage16_trial_frame_upload.md` (upload per-frame BGRA buffers into DComp trial swapchain and expose submit counters)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage17_trial_submit_counter_hygiene.md` (separate trial submit skips from true failures for cleaner diagnostics)
- Issues: `docs/issues/gpu_route_switch_dawn_to_d3d11_dcomp_stage18_trial_upload_fast_gate.md` (add fast gate to avoid per-frame trial upload calls when trial path is disabled)
- Architecture: `docs/architecture/tray-and-appcontroller-refactor.md` (tray menu table-driven + AppController cleanup)
- Architecture: `docs/architecture/settingswnd-emoji-split.md` (SettingsWnd emoji logic split)
- Architecture: `docs/architecture/ui-folder-structure.md` (UI folder layout refolder)
- Architecture: `docs/architecture/mousefx-folder-structure.md` (MouseFx folder layout refolder)
- Architecture: `docs/architecture/trail-effects-differentiation.md` (trail profiles + renderer split)
- Architecture: `docs/architecture/trail-profiles-config.md` (config.json trail_profiles + reload_config)
- Architecture: `docs/architecture/trail-tuning-settings-ui.md` (settings UI: presets + trail tuning)
- Architecture: `docs/architecture/web-settings-ui.md` (browser settings via loopback HTTP server)
- Architecture: `docs/architecture/gpu-final-present-d3d11-dcomp-plan.md` (new GPU final-present roadmap based on D3D11 + DirectComposition)
- Install: `docs/install/installer-packaging-20260204.md` (Inno Setup packaging updates, 2026-02-04)

## What It Is
- Global mouse click visualization for Windows: low-level hook (`WH_MOUSE_LL`) + GDI+ layered ripple windows.
- Click-through: does not block the underlying app.

## Build & Run
1. Open `MFCMouseEffect.slnx` (or the generated `.sln`) in Visual Studio.
2. Choose `x64` + `Debug` (recommended for testing).
3. Build and run `MFCMouseEffect` — output goes to `x64\Debug\MFCMouseEffect.exe` (Release: `x64\Release\...`).
4. On launch (Debug), a one-shot self-test ripple should appear at the cursor within ~250 ms. Then every mouse click should ripple.
5. If clicking admin/elevated windows, run the app as Administrator for matching integrity.

## Release: Tray-Only (No Window)
Release builds no longer create the main frame window. A hidden host window is used solely for the tray icon, so there is no startup flash.
- Entry: `x64\Release\MFCMouseEffect.exe`
- Tray icon: right-click menu “Exit” (double-click also exits)
- Debug still shows a main window for convenience
Note: Tray menu labels now follow the selected UI language (Chinese or English), instead of bilingual mixed text.

## Settings (Browser UI, Non-background Mode)
Tray menus are fine for quick toggles. For full configuration (including advanced tuning), the tray **Settings...** opens a browser page served by a local loopback HTTP server.
- Persistence: saved to `config.json`
- Live apply: changes take effect immediately
- Details: `docs/architecture/web-settings-ui.md`

## Customizing the Look
- File: `MFCMouseEffect/MouseFx/Styles/RippleStyle.h` and `MFCMouseEffect/MouseFx/Windows/RippleWindow.cpp`.
- Key knobs:
  - Duration: `RippleStyle::durationMs`
  - Radii: `startRadius`, `endRadius`
  - Window size: `windowSize`
  - Colors per button: `RippleWindow::StartAt(...)` switch (fill/stroke/glow)

## Operational Notes (Macro-Level)
- **UAC / admin windows:** hooks may not fully work inside elevated apps unless this app is also run elevated.
- **Tray vs background mode:** tray mode is interactive; background mode is IPC-only (no tray UI).
- **IPC control:** background mode is designed to be controlled by a parent process via stdin JSON; it exits when stdin closes.
- **Persistence:** `config.json` lives next to the exe; theme and active effects are persisted there when changed via tray/IPC.
- **Security software:** some endpoint/security tools may block global hooks or layered windows.

## Troubleshooting
- **No ripple at all (Debug):** the self-test ripple did not show. Start likely failed. Check the dialog for `Stage/Error/Message`.
  - Stage `dispatch window`, Error `1400 (invalid window handle)`: fixed in code; rebuild and run `x64\Debug\MFCMouseEffect.exe`.
  - Other errors: see the error text; often permissions or system policies.
- **Hook errors:** The dialog or VS Output window prints `MouseFx: global hook start failed. GetLastError=...`. If you click elevated windows, run this app elevated. Security software may also block hooks.
- **Ripple off-position at >100% DPI:** DPI awareness is enabled at startup; rebuild and run the newest binary.
- **Running wrong binary:** There was a duplicate output under `MFCMouseEffect\x64\Debug\...`. Current project outputs to `x64\Debug\...`. Clean + Rebuild to ensure you run the right one.
- **Virtual secondary display offset:** some tablet/virtual display drivers can cause coordinate-space mismatch (DPI mapping). See: `docs/issues/virtual-display-coordinates.md`.
  - Jan 2026: coordinate normalization is enabled by default; most virtual/ tablet display offsets should now be gone. If it still offsets, please report with driver/app name and DPI settings.

## SDI / Single-Window Notes
- The app now uses an SDI frame: one top-level window hosts the view; ripples are still rendered in separate transparent layered windows, so UI and effect remain decoupled.
- If you need multiple windows, create multiple top-level frames (no MDI children/tabs), or run multiple instances.

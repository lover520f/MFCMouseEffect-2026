# MFCMouseEffect Documentation

Language: [English](README.md) | [中文](README.zh-CN.md)

## Doc Index
- Marketing: `docs/marketing/readme_language_switch.md` (README language toggle rendering)
- Issues: `docs/issues/emoji-support.md` (emoji rendering in settings + text click effect)
- Issues: `docs/issues/text-effect-motion.md` (floating text motion parity after DWrite switch)
- Issues: `docs/issues/settings-emoji-preview.md` (settings emoji color preview overlay)
- Issues: `docs/issues/web-settings-server-lifecycle.md` (web settings idle restart crash + token rotation)
- Issues: `docs/issues/tray_unnamed_menu_entries_root_cause.md` (tray submenu `(Unnamed)` entries caused by unsequenced metadata count argument evaluation)
- Issues: `docs/issues/dawn_native_stage1_hold_neon_gpu_v2_entry.md` (introduce independent `hold_neon3d_gpu_v2` effect id as Dawn-native route entry)
- Issues: `docs/issues/dawn_native_stage2_cpu_fallback_notice.md` (auto CPU fallback + one-time notice + local route status snapshot for unsupported Dawn-native hold effect)
- Issues: `docs/issues/dawn_native_stage3_runtime_dll_probe.md` (restore Dawn runtime DLLs and wire real runtime load probe into fallback reasoning)
- Issues: `docs/issues/dawn_native_stage4_keep_gpu_route_when_runtime_ready.md` (keep `hold_neon3d_gpu_v2` selected when Dawn runtime is loadable; fallback only when runtime probe fails)
- Issues: `docs/issues/dawn_native_stage5_hold_route_snapshot_scope.md` (limit gpu route snapshot to hold category to avoid non-hold overwrite noise)
- Issues: `docs/issues/dawn_native_stage6_repo_runtime_probe_path.md` (add repo runtime DLL probe path so dev layout can load Dawn binaries)
- Issues: `docs/issues/dawn_native_stage7_postbuild_copy_dawn_runtime.md` (post-build copy Dawn runtime DLLs beside exe to avoid missing runtime at launch)
- Issues: `docs/issues/dawn_native_stage8_fluxfield_dual_route_entry.md` (add complex FluxField hold effect with explicit CPU/GPU-v2 route IDs for A/B testing)
- Issues: `docs/issues/dawn_native_stage9_fluxfield_gpu_v2_d2d_backend.md` (switch FluxField GPU-v2 from placeholder to real D2D GPU backend with runtime gate + CPU fallback)
- Issues: `docs/issues/dawn_native_stage10_fluxfield_gpu_transform_fix.md` (fix FluxField GPU-v2 transform to follow mouse-local coordinates instead of screen origin)
- Issues: `docs/issues/dawn_native_stage11_fluxfield_gpu_placeholder_rollback.md` (rollback FluxField GPU-v2 renderer to stable placeholder after COM-error/no-effect regression)
- Issues: `docs/issues/dawn_native_stage12_hold_state_bridge.md` (add normalized hold-state bridge for GPU-v2 routes while keeping stable CPU render fallback)
- Issues: `docs/issues/dawn_native_stage13_fluxfield_d2d_experimental_gate.md` (reintroduce D2D GPU render path behind explicit opt-in gate with safe fallback default)
- Issues: `docs/issues/dawn_native_stage14_fluxfield_d2d_ui_switch.md` (move FluxField D2D experimental gate into web settings UI; no manual marker file required)
- Issues: `docs/issues/dawn_native_stage15_fluxfield_d2d_com_error_fuse_and_settings_only_gate.md` (add COM-error fuse for FluxField D2D path and remove legacy gate interference)
- Issues: `docs/issues/dawn_native_stage16_hold_runtime_diag_write_throttle_fix.md` (remove high-frequency hold-update diagnostic disk writes to fix long-press stutter/CPU rise)
- Issues: `docs/issues/dawn_native_stage17_fluxfield_d2d_binddc_clip_transform_fix.md` (fix D2D BindDC clip rect to match world transform translation and avoid fully clipped invisible rendering)
- Issues: `docs/issues/dawn_native_stage18_fluxfield_gpu_v2_cursor_state_anchor_fix.md` (anchor FluxField GPU-v2 D2D placement by hold-state cursor coordinates in overlay-local space)
- Issues: `docs/issues/dawn_native_stage19_fluxfield_gpu_v2_multisurface_offscreen_guard.md` (guard GPU-v2 D2D from offscreen monitor-surface BindDC failures and clamp clip rect per-surface)
- Issues: `docs/issues/dawn_native_stage20_fluxfield_gpu_v2_d3d11_compute_pivot.md` (pivot FluxField GPU-v2 to stable D3D11 compute workload route with visible lightweight overlay)
- Issues: `docs/issues/dawn_native_stage21_fluxfield_gpu_v2_visual_decouple.md` (replace temporary charge-ring visuals with dedicated FluxField GPU-v2 visual renderer while keeping D3D11 compute route)
- Issues: `docs/issues/dawn_native_stage22_neon_gpu_v2_d3d11_compute_route.md` (implement real Neon HUD3D GPU-v2 route with D3D11 compute workload and hold-end diagnostics snapshot)
- Issues: `docs/issues/dawn_native_stage23_neon_gpu_v2_full_visual_gpu_path.md` (promote Neon HUD3D GPU-v2 to GPU visual rendering path via D2D backend with CPU fallback only as safety net)
- Issues: `docs/issues/dawn_native_stage24_neon_gpu_v2_direct_runtime_full_gpu.md` (bypass overlay GDI loop for `hold_neon3d_gpu_v2` and run direct D3D11+DComp runtime for performance-first full-GPU hold rendering)
- Issues: `docs/issues/dawn_native_stage25_neon_gpu_v2_direct_runtime_visibility_fix.md` (fix no-visible-image issue by COM init on direct runtime worker and presenter window style compatibility adjustment)
- Issues: `docs/issues/dawn_native_stage26_neon_gpu_v2_render_loop_and_coord_fix.md` (fix persistent no-render by adding runtime thread message pump and hardening hold-start coordinates with cursor fallback)
- Issues: `docs/issues/dawn_native_stage27_neon_gpu_v2_presenter_error_telemetry.md` (add API-level presenter failure telemetry and propagate detailed reason into runtime diagnostics)
- Issues: `docs/issues/dawn_native_stage28_neon_gpu_v2_swapchain_prereq_order_fix.md` (fix RenderFrame precheck order so lazy swapchain creation can execute instead of failing at first frame)
- Issues: `docs/issues/dawn_native_stage29_neon_gpu_v2_full_d3d11_presenter.md` (replace D2D surface bridge with pure D3D11 shader presenter on DComp swapchain to eliminate `CreateBitmapFromDxgiSurface` hard-fail path)
- Issues: `docs/issues/dawn_native_stage30_neon_gpu_v2_hardware_adapter_and_alpha_cleanup.md` (prefer explicit hardware adapter creation and tighten shader alpha footprint to reduce WARP probe noise and dark background artifact)
- Issues: `docs/issues/dawn_native_stage31_neon_gpu_v2_formal_hud3d_shader.md` (replace temporary test ring shader with formal Neon HUD3D layered visual while preserving full-GPU presenter path)
- Issues: `docs/issues/dawn_native_stage32_neon_gpu_v2_angular_wrap_fix.md` (fix white sector artifact by correcting angular distance wrap math in shader and preventing exponential over-bright blow-up)
- Issues: `docs/issues/dawn_native_stage33_quantum_halo_gpu_v2_rename_and_high_fidelity_shader.md` (rename Neon GPU-v2 to Quantum Halo GPU-v2, add old-id compatibility aliasing, and upgrade to a higher-fidelity multi-layer shader)
- Issues: `docs/issues/dawn_native_stage34_gpu_fallback_notice_webui_and_false_positive_fix.md` (remove blocking fallback popup, fix alias-triggered false-positive fallback notification, and surface real fallback notices in Web settings status)
- Issues: `docs/issues/dawn_native_stage35_gpu_display_name_normalization.md` (normalize user-facing GPU/CPU labels across settings metadata, tray, and Web UI while keeping internal route ids compatible)
- Issues: `docs/issues/dawn_native_stage36_remove_fluxfield_gpu_d2d_experimental_switch.md` (remove obsolete FluxField GPU D2D experimental toggle from UI/config/runtime path to avoid misleading controls)
- Issues: `docs/issues/dawn_native_stage37_fluxfield_gpu_single_route_with_cpu_fallback.md` (enforce single-route rendering for FluxField GPU path: GPU visual priority with CPU fallback only on failure, plus explicit option labels)
- Issues: `docs/issues/dawn_native_stage38_fluxfield_single_option_in_settings.md` (remove CPU-only FluxField from settings metadata so users see only one FluxField option with GPU-first auto-fallback behavior)
- Issues: `docs/issues/dawn_native_stage39_fluxfield_gpu_visibility_matrix_anchor_fallback.md` (fix invisible FluxField GPU frames by replacing off-surface cursor no-op with matrix-anchor fallback rendering)
- Issues: `docs/issues/dawn_native_stage40_follow_mode_and_flux_label_copy_cleanup.md` (rename hold follow-mode copy to cursor-priority semantics and normalize FluxField GPU label to CPU fallback wording)
- Issues: `docs/issues/dawn_native_stage41_text_click_font_size_configurable.md` (add configurable click-text font size in Web settings, with end-to-end state/apply/persist wiring)
- Issues: `docs/issues/dawn_native_stage42_quantum_halo_gpu_driver_seh_guard.md` (add SEH guard around Quantum Halo GPU submit path to prevent hard crash on driver-side access violations and trigger controlled failure fallback)
- Issues: `docs/issues/mfc-shell-stage59-dispatch-feature-routing-extraction.zh-CN.md` (extract WASM/automation/input-indicator dispatch details into dedicated feature handlers while preserving behavior)
- Issues: `docs/issues/vm_foreground_effect_suppression.md` (suppress host-side effects while VM window is foreground; auto resume after returning to host focus)
- Issues: `docs/issues/scroll_twinkle_starfield_effect.md` (add new scroll `twinkle` star-particle renderer and integrate into factory/settings/tray)
- Issues: `docs/issues/input-automation-mouse-gesture-keyboard-mapping.zh-CN.md` (add core pipeline for mouse action/gesture -> keyboard mapping, default-off and backward-compatible)
- Issues: `docs/input_indicator_refactor.md` (Refactor: generic input indicator supporting keyboard and enhanced mouse actions)
- Issues: `docs/issues/web-settings-hci-refresh.zh-CN.md` (refresh Web settings UX using HCI principles: clearer IA, navigation, feedback, and responsive usability)
- Issues: `docs/issues/web-settings-svelte-migration.zh-CN.md` (progressive Svelte migration checklist for Web settings UI, stage-by-stage with verification gates)
- Issues: `docs/issues/web-settings-stage4-form-coordinator.zh-CN.md` (stage 4.1: extract Web settings form orchestration from app.js into dedicated module)
- Issues: `docs/issues/web-settings-stage4-i18n-extraction.zh-CN.md` (stage 4.2: extract Web settings i18n catalog from app.js into standalone WebUI asset)
- Issues: `docs/issues/web-settings-stage4-web-api-extraction.zh-CN.md` (stage 4.3: extract Web settings API access and health-check transport from app.js)
- Issues: `docs/issues/web-settings-stage4-i18n-runtime-extraction.zh-CN.md` (stage 4.4: extract Web settings i18n runtime behavior from app.js)
- Issues: `docs/issues/web-settings-stage5-shell-svelteization.zh-CN.md` (stage 5.1: migrate Web settings shell layout from static HTML to Svelte components, JS-only)
- Issues: `docs/issues/web-settings-stage5-dialog-svelteization.zh-CN.md` (stage 5.2: migrate Web settings dialog runtime from legacy DOM script to Svelte, JS-only)
- Issues: `docs/issues/web-settings-stage5-shell-action-bridge.zh-CN.md` (stage 5.3: bridge top actions and status banner state through Svelte shell runtime APIs)
- Issues: `docs/issues/web-settings-stage6-input-indicator-stateful-svelte.zh-CN.md` (stage 6.1: migrate Input Indicator dynamic form/runtime behavior to Svelte state layer with compatibility fallback)
- Issues: `docs/issues/web-settings-stage6-core-sections-stateful-svelte.zh-CN.md` (stage 6.2: migrate General/Effects/Text/Trail form rendering and readback to Svelte section state APIs)
- Issues: `docs/issues/webui-generated-bundles-untracked.zh-CN.md` (stop tracking WebUI generated `*.svelte.js` artifacts; keep bundles build-local)
- Issues: `docs/issues/web-settings-entry-layout-and-iss-preflight.zh-CN.md` (consolidate WebUI multi-entry scripts under `src/entries` and add installer preflight checks for required WebUI bundles)
- Issues: `docs/issues/web-settings-automation-zh-i18n-fix.zh-CN.md` (fix mixed English text in automation mapping when UI language is Chinese)
- Issues: `docs/issues/webui-pnpm-migration.zh-CN.md` (switch WebUIWorkspace package manager from npm to pnpm, including lockfile migration)
- Issues: `docs/issues/webui-build-minify.zh-CN.md` (enable WebUI minified build output and reduce packaged web asset size)
- Issues: `docs/issues/web-settings-automation-editor.zh-CN.md` (add Web settings automation editor for mouse action/gesture to keyboard mappings)
- Issues: `docs/issues/web-settings-automation-chain-shortcut-capture.zh-CN.md` (upgrade automation mapping UX with direct shortcut capture and chained action triggers)
- Issues: `docs/issues/wasm-phase2-runtime-render-landing.md` (Phase2 runtime render landing checklist for WASM click commands)
- Issues: `docs/issues/wasm-runtime-bridge-self-build.md` (land first-party self-build pipeline for `mfx_wasm_runtime.dll` and wire installer packaging)
- Issues: `docs/issues/wasm-click-mixed-text-image-access-violation-fix.md` (fix execute access violation in complex `Demo Click Mixed Text Image (0.1.1)` runtime path by replacing variadic wasm3 calls, hardening memory copy, and adding SEH recovery)
- Issues: `docs/issues/wasm-plugin-import-export-phase3g.md` (WASM plugin panel: import selected plugin into primary root and export all discovered plugins)
- Issues: `docs/issues/wasm-plugin-folder-import-dialog.md` (WASM plugin panel: import by selecting plugin folder via native picker, validate plugin.json/entry wasm, then copy to primary root)
- Issues: `docs/issues/wasm-plugin-path-label-clarity.md` (WASM panel naming cleanup: WASM Plugin -> Effect Plugins, Plugin Catalog -> Plugin Info, and path-label simplification)
- Issues: `docs/issues/wasm-plugin-button-clarity-tooltips.md` (WASM panel action wording cleanup and hover tooltips for all core buttons)
- Issues: `docs/issues/wasm-plugin-catalog-root-config.md` (WASM plugin panel: configurable catalog root path persisted in config and applied to scan/export flows)
- Issues: `docs/issues/wasm-panel-initial-i18n-order-fix.md` (fix first-load WASM panel mixed-language labels caused by pre-mount i18n timing)
- Issues: `docs/issues/wasm-image-assets-gif-and-static-support.md` (WASM `spawn_image` now supports plugin file assets: png/jpg/bmp/gif/tiff with built-in fallback)
- Issues: `docs/issues/wasm-plugin-template-full-sample-matrix-phase4b.md` (expand wasm-plugin-template to full sample matrix and add bilingual user-facing template docs)
- Issues: `docs/issues/wasm-plugin-template-assets-all-formats-phase4c.md` (bundle real downloaded sample images across all supported formats and auto-copy `image_assets` into sample build outputs)
- Issues: `docs/issues/wasm-spawn-image-kinematics-fix.md` (fix static-looking WASM image effects by honoring `vx/vy/ax/ay/delayMs` in host runtime and retuning sample trajectories)
- Issues: `docs/issues/mouse-action-indicator-overlay.md` (Legacy: add cursor-adjacent mouse action indicator)
- Architecture: `docs/architecture/tray-and-appcontroller-refactor.md` (tray menu table-driven + AppController cleanup)
- Architecture: `docs/architecture/settingswnd-emoji-split.md` (SettingsWnd emoji logic split)
- Architecture: `docs/architecture/ui-folder-structure.md` (UI folder layout refolder)
- Architecture: `docs/architecture/mousefx-folder-structure.md` (MouseFx folder layout refolder)
- Architecture: `docs/architecture/trail-effects-differentiation.md` (trail profiles + renderer split)
- Architecture: `docs/architecture/trail-profiles-config.md` (config.json trail_profiles + reload_config)
- Architecture: `docs/architecture/trail-tuning-settings-ui.md` (settings UI: presets + trail tuning)
- Architecture: `docs/architecture/web-settings-ui.md` (browser settings via loopback HTTP server)
- Architecture: `docs/architecture/custom-effects-wasm-route.md` (user-custom effects route: WASM logic plugins + C++ host batched rendering)
- Architecture: `docs/architecture/wasm-plugin-template-quickstart.md` (official WASM plugin template quick start and local compile flow)
- Architecture: `docs/architecture/wasm-plugin-compatibility.md` (ABI/manifest compatibility policy for WASM plugin upgrades)
- Architecture: `docs/architecture/wasm-plugin-troubleshooting.md` (WASM plugin diagnostics and troubleshooting playbook)
- Architecture: `docs/architecture/dawn-native-effects-route.md` (postmortem + new Dawn-native effect route from clean main)
- Refactoring: `docs/refactoring/phase6-dispatchrouter-boundary.md` (remove `DispatchRouter` friend/private-field coupling by introducing explicit dispatch-facing APIs)
- Refactoring: `docs/refactoring/phase7-effectfactory-registry.md` (pivot `EffectFactory` from branch-based creation to category registry with typed and fallback creators)
- Refactoring: `docs/refactoring/phase8-appcontroller-config-activation-dedup.md` (deduplicate active-effect apply/normalize flow between startup and reload paths)
- Refactoring: `docs/refactoring/phase9-appcontroller-active-category-table.md` (use a single active-category descriptor table to remove repeated per-category branching in `AppController`)
- Refactoring: `docs/refactoring/phase10-appcontroller-reapply-helpers.md` (centralize active-slot lookup/enabled-check/reapply helpers and reuse them across config-update paths)
- Refactoring: `docs/refactoring/phase11-commandhandler-encoding-cleanup.md` (remove non-ASCII source character causing `C4819` warning in default code-page builds)
- Refactoring: `docs/refactoring/phase12-theme-reapply-metadata.md` (move theme-reapply category selection into active-category metadata to avoid hard-coded method branching)
- Refactoring: `docs/refactoring/phase13-commandhandler-command-routing-table.md` (replace `CommandHandler::Handle` if/else chain with command routing table and dedicated per-command handlers)
- Refactoring: `docs/refactoring/phase14-apply-settings-active-route-table.md` (replace hard-coded apply-settings active effect calls with category/key route table iteration)
- Refactoring: `docs/refactoring/phase15-input-indicator-payload-dedup.md` (deduplicate input-indicator payload parsing across new/legacy schema branches while preserving legacy scope)
- Refactoring: `docs/refactoring/phase16-trail-profile-route-table.md` (replace manual trail profile apply calls with route-table iteration in apply_settings parsing)
- Refactoring: `docs/refactoring/phase17-commandhandler-applysettings-file-split.md` (split `CommandHandler` into routing file and dedicated `apply_settings` parsing file to reduce per-file complexity)
- Refactoring: `docs/refactoring/phase18-applysettings-helper-extraction.md` (extract focused helper functions from `HandleApplySettings` while keeping payload processing order unchanged)
- Refactoring: `docs/refactoring/phase19-appcontroller-configupdates-file-split.md` (split config update/apply methods from `AppController.cpp` into dedicated implementation unit)
- Refactoring: `docs/refactoring/phase20-effectconfig-file-split.md` (split `EffectConfig` by responsibility into shared helpers + load + save units and keep public entry points minimal)
- Refactoring: `docs/refactoring/phase21-effectconfig-json-codec-boundary.md` (introduce dedicated JSON codec boundary so `Load/Save` focus on file lifecycle while schema mapping stays isolated)
- Refactoring: `docs/refactoring/phase22-effectconfig-parse-multiunit-split.md` (split config JSON parse path into root/input-trail/effects units with a parser-internal boundary)
- Refactoring: `docs/refactoring/phase23-effectconfig-serialize-multiunit-split.md` (split config JSON serialize path into root/input-trail/effects units with a serialize-internal boundary)
- Refactoring: `docs/refactoring/phase24-effectconfig-json-keys-centralization.md` (centralize EffectConfig JSON schema keys and replace codec string literals with constants)
- Refactoring: `docs/refactoring/phase25-effectconfig-json-keys-subheaders.md` (split centralized EffectConfig JSON key definitions into root/active/input/trail/effects subheaders)
- Refactoring: `docs/refactoring/phase26-effect-color-mapping-helper-extraction.md` (extract shared helpers for ripple/icon color field parse/serialize mapping to remove duplicate branches)
- Refactoring: `docs/refactoring/phase27-parse-input-trail-unit-split.md` (split parse input/trail codec into dedicated units and remove mixed InputTrail implementation file)
- Refactoring: `docs/refactoring/phase28-serialize-input-trail-unit-split.md` (split serialize input/trail codec into dedicated units and remove mixed InputTrail implementation file)
- Refactoring: `docs/refactoring/phase29-core-config-folderization.md` (move `EffectConfig*` domain into `MouseFx/Core/Config` to reduce `Core` flatness and clarify ownership boundaries)
- Refactoring: `docs/refactoring/phase30-core-control-folderization.md` (move AppController/command/dispatch/ipc control domain into `MouseFx/Core/Control` and retarget project/include boundaries)
- Refactoring: `docs/refactoring/phase31-core-overlay-folderization.md` (move overlay domain units into `MouseFx/Core/Overlay` and harden moved-header include dependencies)
- Refactoring: `docs/refactoring/phase32-core-system-folderization.md` (move OS/runtime-facing units into `MouseFx/Core/System` and retarget dependent include/project mappings)
- Refactoring: `docs/refactoring/phase33-configpathresolver-to-core-config.md` (move config path resolver into `MouseFx/Core/Config` and retarget dependent include/project mappings)
- Refactoring: `docs/refactoring/phase34-core-protocol-folderization.md` (move JsonLite and MouseFxMessages into `MouseFx/Core/Protocol` and retarget protocol include/project mappings)
- Refactoring: `docs/refactoring/phase35-effectfactory-to-core-control.md` (move EffectFactory into `MouseFx/Core/Control` and align header dependencies with config/control boundaries)
- Refactoring: `docs/refactoring/phase36-websettingsserver-multiunit-split.md` (split WebSettingsServer by lifecycle/routing/token-monitor responsibilities into separate implementation units)
- Refactoring: `docs/refactoring/phase37-httpserver-protocol-split.md` (split HttpServer transport loop and HTTP protocol parse/response logic into dedicated implementation units)
- Refactoring: `docs/refactoring/phase38-quantum-halo-presenter-backend-pluggable.md` (extract Quantum Halo GPU presentation into pluggable backend architecture with registry + host orchestration)
- Refactoring: `docs/refactoring/phase39-hold-gpu-route-catalog-and-presenter-policy.md` (centralize hold GPU routes, add presenter backend strategy config/env entry, and enhance presenter diagnostics)
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

## WASM Plugin Quick Check
If you want to verify custom WASM click effects end-to-end quickly:

1. Build template artifact in `examples/wasm-plugin-template`.
2. Open settings -> `WASM Plugin`.
3. To import a downloaded plugin package, click `Import Plugin Folder`, select the folder that contains `plugin.json`, then confirm.
4. Click `Refresh Catalog`, pick a plugin, then `Load Selected`.
5. Click `Enable`, then click on desktop a few times.

Success indicators:
- `Plugin loaded = Yes`
- `Rendered by WASM = Yes`
- `Last call metrics`: `duration/output/commands` are all non-zero

If catalog is empty:
- Check `Catalog roots` in the same panel.
- Ensure `plugin.json` + `effect.wasm` exist under one of those roots.
- See: `docs/architecture/wasm-plugin-template-quickstart.md`
- See: `docs/architecture/wasm-plugin-troubleshooting.md`

If you use custom image files in `spawn_image`:
- add `image_assets` into `plugin.json` (paths relative to manifest);
- supported extensions: `.png/.jpg/.jpeg/.bmp/.gif/.tif/.tiff`.

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



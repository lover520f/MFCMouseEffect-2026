# Agent Current Context (2026-03-01)

## Scope and Priority
- Primary dev host: macOS.
- Primary target: macOS usable loop first.
- Constraints: no Windows regression; Linux follows compile + contract coverage.

## Latest Delta (2026-03-01)
- macOS active source ownership is now fully promoted to main paths under `Platform/macos/{Effects,Overlay,Shell,System,Wasm}`; `Platform/macos/legacy` no longer contains active source files.
- macOS build wiring now uses explicit ObjC++ allowlist compilation in `Platform/macos/CMakeLists.txt` (replacing directory wildcard matching) so migration scope is mechanically auditable and prunable per-file.
- macOS Swift bridge build now uses explicit latest-stable language mode policy via `MFX_SWIFT_LANGUAGE_MODE` (`auto|5|6`, default `auto`; auto resolves to Swift 6 on Swift 6 toolchains, else Swift 5).
- macOS native folder picker is now Swift-owned end-to-end (`OpenPanel -> AppleScript fallback`) via `MacosNativeFolderPickerBridge.swift`; `PlatformNativeFolderPicker` no longer calls Objective-C++ fallback, and `mfx_entry_runtime_common` no longer links legacy folder-picker `.mm` sources.
- macOS automation app-catalog scan path is now Swift-owned (`MacosApplicationCatalogBridge.swift` + callback bridge), and legacy Objective-C++ scan workflow files are removed from main `mfx_shell_macos` build wiring.
- macOS foreground-process resolver is now Swift-owned (`MacosForegroundProcessBridge.swift` + `MacosForegroundProcessService.cpp`), and legacy `MacosForegroundProcessService*.mm` is removed from main `mfx_shell_macos` build wiring.
- macOS input-permission resolver is now Swift-owned (`MacosInputPermissionBridge.swift` + `MacosInputPermissionState.cpp`), and legacy `MacosInputPermissionState*.mm` is removed from main `mfx_shell_macos` build wiring.
- macOS Objective-C++ dependent `.cpp` sources now compile with ObjC++ mode (`-x objective-c++`) via an explicit file allowlist in `Platform/macos/CMakeLists.txt` (no `legacy/*` or directory wildcard selectors), reducing residual migration drift.
- macOS tray menu localization is now Swift-owned (`MacosTrayMenuLocalizationBridge.swift` + `MacosTrayMenuLocalization.cpp`), and mac build wiring no longer requires legacy `MacosTrayMenuLocalization.mm`.
- macOS legacy tree now has `0` source files under `Platform/macos/legacy`; migration policy is "no new legacy-source resurrection" and no compile wiring depends on that tree.
- macOS tray menu creation/release/auto-trigger/terminate path is now Swift-owned (`MacosTrayMenuBridge.swift` + `MacosTrayMenuSwiftBridge.h` + `MacosTrayMenuFactory.cpp` thin C++ adapter); `MacosTrayService.cpp` is now pure C++ call-through and main build wiring no longer depends on legacy `MacosTrayMenuFactory.ActionBridge.cpp`, `MacosTrayMenuFactory.Items.cpp`, or `MacosTrayRuntimeHelpers.cpp`.
- Obsolete tray legacy files were removed from repository to keep single-source ownership clear: `legacy/Shell/MacosTrayMenuFactory.ActionBridge.cpp`, `legacy/Shell/MacosTrayMenuFactory.Items.cpp`, `legacy/Shell/MacosTrayRuntimeHelpers.cpp`, `legacy/Shell/MacosTrayMenuLocalization.cpp`, and unused headers `Shell/MacosTrayMenuFactory.Internal.h`, `Shell/MacosTrayRuntimeHelpers.h`.
- macOS event-loop AppKit bridge is now Swift-owned (`MacosEventLoopBridge.swift` + `MacosEventLoopSwiftBridge.h`), and C++ side calls through a thin wrapper (`Shell/MacosEventLoopBridge.cpp`).
- Stable Shell adapters are now promoted from `legacy/Shell` to `Shell` main path (`MacosTrayService.cpp`, `MacosTrayMenuFactory.cpp`, `MacosEventLoopBridge.cpp`, `MacosTraySmokeMain.cpp`); `legacy/Shell` no longer owns active build sources.
- Input pipeline adapters are now promoted from `legacy/System` to `System` main path (global input hook, keyboard injector, key resolver/tables, virtual-key mapper, input-event utils), with `Platform/macos/CMakeLists.txt` updated to consume main-path sources.
- Obsolete legacy System fallback implementations were removed after Swift bridge cutover (app-catalog scan workflow, foreground-process resolver, input-permission resolver, native-folder-picker fallback chain), and unused legacy-only headers in `Platform/macos/System` were deleted to keep single-source ownership.
- Overlay runtime implementation is now promoted from `legacy/Overlay` to `Overlay` main path (`MacosInputIndicatorOverlay*`, `MacosOverlayCoordSpace*`), with source wiring switched to main-path files.
- WASM overlay/runtime implementation is now promoted from `legacy/Wasm` to `Wasm` main path (`MacosWasmCommandRenderer*`, `MacosWasmOverlayState*`, `MacosWasmTextOverlay*`, `MacosWasmImageOverlayRenderer*`), with source wiring switched to main-path files.
- Effects runtime implementation is now promoted from `legacy/Effects` to `Effects` main path (`MacosClick/Trail/Scroll/Hover/Hold*`, `MacosOverlayRenderSupport*`, `MacosTextEffectFallback*`), with source wiring switched to main-path files.
- `/api/state.wasm` now exposes macOS overlay throttle-policy diagnostics (`overlay_max_inflight`, `overlay_min_image_interval_ms`, `overlay_min_text_interval_ms`), and core HTTP state contracts assert these fields on macOS.
- core automation regression now sets deterministic macOS overlay-policy test env defaults (`MFX_MACOS_WASM_OVERLAY_MAX_INFLIGHT=77`, `MFX_MACOS_WASM_IMAGE_MIN_INTERVAL_MS=9`, `MFX_MACOS_WASM_TEXT_MIN_INTERVAL_MS=11`) and asserts `/api/state.wasm` value-level parity via `MFX_EXPECT_MACOS_WASM_*`.
- `/api/state.wasm` and `/api/wasm/state` now expose lifetime WASM dispatch diagnostics (`lifetime_invoke_*`, `lifetime_render_dispatches`, `lifetime_executed_*`, `lifetime_throttled_*`, `lifetime_dropped_render_commands`) in addition to `last_*` snapshot fields.
- core WASM dispatch contract checks (regression + manual selfcheck) now enforce lifetime counter invariants (`invoke_success + invoke_failed == invoke_calls`, throttle subtotal consistency, and lifetime counters not below dispatch snapshot), and schema/state checks assert `lifetime_*` key exposure.
- WebUI semantic phase now includes `pnpm --dir MFCMouseEffect/WebUIWorkspace run test:wasm-state-model` to gate `normalizeWasmState` passthrough for lifetime WASM diagnostics.
- WebUI WASM diagnostics panel now renders both `last_*` and `lifetime_*` summaries (invoke/render), and WebUI semantic phase now includes `pnpm --dir MFCMouseEffect/WebUIWorkspace run test:wasm-diagnostics-model` to gate label coverage and formatting contracts.
- POSIX regression suite now enforces `no objcxx edits` by default (`MFX_ENFORCE_NO_OBJCXX_EDITS=1`), with explicit per-run override `--allow-objcxx-edits` for approval-only exceptions; this keeps macOS mainline aligned with `Swift-first / no new .mm surface` policy.
- macOS ObjC++ surface gate is now explicit-scripted (`tools/platform/regression/run-macos-objcxx-surface-regression.sh`) and wired into `run-posix-regression-suite.sh`; gate now asserts: `.mm` count is zero, ObjC++ compile wiring is allowlist-based (not directory wildcard), and allowlist entries resolve to real files under `Platform/macos/{Effects,Overlay,Wasm}`.
- macOS input-indicator panel lifecycle/style is now Swift-owned (`MacosInputIndicatorBridge.swift` + `MacosInputIndicatorSwiftBridge.h`), and C++ overlay files now call opaque bridge APIs (`CreatePanel/ReleasePanel/HidePanel/ApplyPanelPresentation`) instead of direct `NSPanel/NSTextField` message sends; overlay internals also removed `NSString` helper dependency and now use `pthread_main_np` for main-thread checks.
- macOS overlay coordinate conversion (`Quartz -> Cocoa`) is now Swift-owned (`MacosOverlayCoordSpaceBridge.swift` + `MacosOverlayCoordSpaceSwiftBridge.h`), and `MacosOverlayCoordSpaceConversion.cpp` is reduced to bridge invocation without direct `NSScreen/CGDisplayBounds` AppKit logic.
- macOS overlay render-support core window/contents-scale path is now Swift-owned (`MacosOverlayRenderSupportBridge.swift` + `MacosOverlayRenderSupportSwiftBridge.h`); `MacosOverlayRenderSupport.cpp` now delegates overlay-window creation and per-point content-scale application to Swift bridge APIs, further shrinking direct AppKit message-send surface in effect runtime support code.
- macOS overlay render-support bridge now also owns unified overlay-window release (`mfx_macos_overlay_release_window_v1`), and macOS effects window lifecycle call-sites (click/trail/scroll registries + renderer close paths + line-trail/hover/hold close paths) now use `macos_overlay_support::ReleaseOverlayWindow(...)` instead of scattered direct `orderOut/release` message sends.
- macOS overlay render-support bridge now also owns unified overlay-window front/show and per-point screen-frame resolve (`mfx_macos_overlay_show_window_v1`, `mfx_macos_overlay_resolve_screen_frame_v1`); effect renderers now route window fronting through `macos_overlay_support::ShowOverlayWindow(...)`, and `MacosLineTrailOverlay.cpp` removed local `NSScreen` resolve logic in favor of `macos_overlay_support::ResolveScreenFrameForPoint(...)` with cached frame-size state.
- macOS text-effect fallback panel UI path is now Swift-owned (`MacosTextEffectFallbackBridge.swift` + `MacosTextEffectFallbackSwiftBridge.h`); `MacosTextEffectFallback.cpp` keeps only animation-timing/spec computation and delegates panel create/show/frame/style/release to Swift bridge APIs, reducing direct AppKit dependency in text-click fallback runtime.
- macOS line-trail overlay path is now Swift-owned (`MacosLineTrailOverlayBridge.swift` + `MacosLineTrailOverlaySwiftBridge.h`); `MacosLineTrailOverlay.cpp` is reduced to a pure C++ bridge wrapper (`ScreenToOverlayPoint` + C ABI forwarding), and the file was removed from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`.
- macOS effects bridge dedup is now tightened: `MacosLineTrailOverlayBridge.swift` reuses shared overlay bridge ABI for window/frame/scale operations (no local duplicate path), and `MacosTextEffectFallback.cpp` reuses `macos_overlay_support::RunOnMainThread*` instead of private duplicate helpers.
- macOS click/trail/scroll window registries now detach from ObjC++ compile dependency by consuming `MacosOverlayRenderSupportSwiftBridge.h` C ABI directly, and all three `*WindowRegistry.cpp` files are removed from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`.
- macOS effects pure compute/registry modules are now detached from ObjC++ allowlist (`MacosEffectCreatorRegistry*`, `MacosEffectComputeProfileAdapter.cpp`, `MacosEffectRenderProfile*`), keeping allowlist focused on true AppKit/ObjC dependent runtime paths.
- macOS overlay coordinate conversion/service modules are now also detached from ObjC++ allowlist (`MacosOverlayCoordSpaceConversion.cpp`, `MacosOverlayCoordSpaceService.cpp`) since both paths are pure C++ wrappers over Swift bridge/C++ logic.
- macOS ObjC++ allowlist is further pruned by compiler-level probe (`compile_commands` per entry with `-x c++ -fsyntax-only`), removing 26 additional pure-C++ wrappers across effects entry files, input-indicator overlays, and WASM command/state wrappers while preserving build/regression pass gates.
- macOS effects plan/state headers now expose C++-compatible forward-declaration boundaries (AppKit imports kept in ObjC++ implementation files), enabling 4 additional files (`Click/Trail/Scroll Core.Plan` + `Hold Core.State`) to be removed from ObjC++ allowlist with full scaffold/core-automation/surface-gate regression pass.
- macOS WASM planning/layout headers now use C++-compatible forward-declaration boundaries (with AppKit/Foundation imports moved to ObjC++ implementation files), enabling `MacosWasmImageOverlayRendererCore.Plan.cpp` and `MacosWasmTextOverlay.Layout.cpp` removal from ObjC++ allowlist while preserving scaffold/core-automation/surface-gate passes.
- `MacosHoldPulseOverlayRendererCore.cpp` is now also removed from ObjC++ allowlist after null-sentinel cleanup (`nil -> nullptr`) confirmed pure C++ compatibility and full scaffold/core-automation/surface-gate regression pass.
- macOS effects wrapper entry files (`MacosClick/Trail/Scroll/Hover/HoldPulseOverlayRenderer.cpp`) now use C++ callback-based main-thread bridge APIs (no Objective-C block literals), and all five wrappers are removed from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST` with scaffold/core-automation/surface-gate regression pass.
- `MacosTextEffectFallback.cpp` now uses pure C++ callback scheduling (`RunOnMainThread*` callback overload + `dispatch_after_f` animation tick context) and is removed from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST` with full scaffold/core-automation/surface-gate regression pass.
- `MacosWasmOverlayRuntime.cpp` now uses pure C++ main-thread dispatch (`pthread_main_np` + `dispatch_sync_f/dispatch_async_f`) and unified overlay-window release via `macos_overlay_support::ReleaseOverlayWindow(...)`, and the file is removed from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST` with full scaffold/core-automation/surface-gate regression pass.
- macOS `click/trail/scroll/hover` style modules are now color-decoupled from `NSColor/AppKit` contracts (style layer keeps normalize/path helpers only), scroll renderer-support now consumes command-resolved ARGB directly, and four style files are removed from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST` with full scaffold/core-automation/surface-gate regression pass.
- `MacosWasmOverlayRenderMath.cpp` is now pure math-only (no `AppKit`/`NSColor` contract), with ARGB->`NSColor` conversion moved to wasm renderer leaf files (`ImageOverlayRendererCore.Window`, `TextOverlay.Style`), and the file is removed from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST` with full scaffold/core-automation/surface-gate regression pass.
- `MacosWasmImageOverlayRendererSupport.cpp` now exposes pure C++ UTF-8 path helpers (`Utf8PathFromWide`) instead of Objective-C `NSString*` contracts, with `NSString` conversion moved to `ImageOverlayRendererCore.Window` leaf rendering path, and the file is removed from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST` with full scaffold/core-automation/surface-gate regression pass.
- `MacosHoverPulseOverlayRendererCore.Plan.cpp` is now constrained to pure plan-building logic (ring-layer configuration moved to `Core.Layers.cpp`), hover internal header now supports non-ObjC compile aliases (`NSView`/`CAShapeLayer`/`NSRect`) with `CGRectZero` init, and `Core.Plan.cpp` is removed from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST` with full scaffold/core-automation/surface-gate regression pass.
- `MacosHoldPulseOverlayStyle.cpp` is now pure compute-only (`NormalizeHoldType`/`ResolveHoldStyle`), while `NSColor` conversion and hold accent layer rendering are moved to `MacosHoldPulseOverlayRendererCore.Start.cpp` (already ObjC++ leaf). `MacosHoldPulseOverlayStyle.cpp` is removed from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`; current allowlist count is `16`.
- `MacosHoldPulseOverlayStyle.Accent.cpp` is now pure path-plan compute (`BuildSpecialHoldAccentPath`: path + line width + fill mode). Layer color/application is executed in `MacosHoldPulseOverlayRendererCore.Start.cpp`, and `MacosHoldPulseOverlayStyle.Accent.cpp` is removed from `MFX_MACOS_OBJCXX_SOURCE_ALLOWLIST`; current allowlist count is `15`.
- Validation passed:
  - `cmake -S MFCMouseEffect/Platform -B /tmp/mfx-platform-macos-legacy-mm-build -DMFX_PACKAGE_PLATFORM=macos -DMFX_ENABLE_POSIX_CORE_RUNTIME=ON -DMFX_ENABLE_ENTRY_RUNTIME_TARGETS=ON`
  - `cmake --build /tmp/mfx-platform-macos-legacy-mm-build --target mfx_shell_macos mfx_entry_posix_host -j8`
  - `./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
  - `./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
  - `./tools/platform/regression/run-posix-wasm-regression-suite.sh --platform auto`
  - `./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build`
  - `./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
  - Runtime API spot check: `POST /api/automation/app-catalog` returns `ok=true` with non-zero `count` (manual host session probe)

## Current Program State
- Branch baseline has completed Phase 50 -> Phase 55zj code slices.
- POSIX dual-lane guardrail exists:
  - scaffold lane remains default stable lane.
  - core lane is gated and iteratively enabled.
  - macOS core lane now includes:
  - global input capture/degraded handling
  - input indicator + click-first visible effect
  - baseline scroll visible effect (`MacosScrollPulseEffect`)
  - automation mapping foundation
  - WASM runtime backend (`wasm3_static`) with renderer strategy path
  - WASM plugin catalog/import/export HTTP APIs in WebSettings
  - WASM plugin folder import dialog path on macOS (native picker)
  - import dialog supports `probe_only=true` for non-interactive regression checks
  - test-gated `/api/wasm/test-dispatch-click` enables non-interactive invoke/render contract checks
  - test-gated `/api/automation/test-app-scope-match` enables non-interactive app-scope alias contract checks (`code/.exe/.app`)
  - test-gated `/api/automation/test-binding-priority` enables non-interactive priority contract checks (`process > all`, `longest chain > shorter chain`)
  - test-gated `/api/automation/test-match-and-inject` enables non-interactive matcher+injector integration checks (`history -> selected binding -> inject`)
  - test-gated `/api/automation/test-shortcut-from-mac-keycode` enables non-interactive `Cmd+V/Cmd+Tab` mapping contract checks
  - shortcut test API now has explicit invalid-keycode contract (`supported=false`, `vk_code=0`, empty `shortcut`, reason code) with regression guard
  - WASM runtime action routes (`load-manifest`/`reload`) now expose structured `error_code`, and contracts now cover `manifest_path required` + `reload ok` semantics
  - `wasm_reload` command path is now truly cross-platform (no Windows-only no-op), with deterministic `reload_target_missing` contract validated via test-gated runtime-reset probe
  - WASM reload missing-module path is now contract-gated via deterministic fixture flow (`load -> remove entry wasm -> reload => module_load_failed/load_module`)
  - WASM reload manifest-api mismatch path is now contract-gated via fixture mutation (`load -> api_version=2 -> reload => manifest_api_unsupported/manifest_api_version`)
  - WASM fixture orchestration is now helperized in one shared script module used by regression and manual selfcheck, reducing fixture-flow drift
  - WebUI WASM action error model now maps runtime load/reload error codes (`reload_target_missing`, `module_load_failed`, `manifest_api_unsupported`, etc.) to stable user-facing messages, and legacy WebUI EN/ZH i18n keys are now kept in parity for those runtime codes
  - WebUI WASM error-model test now asserts i18n parity (`action-error-model` keys must exist in `WebUI/i18n.js` for both `en-US` and `zh-CN`), preventing future mapping-vs-dictionary drift
  - WASM route path contracts now explicitly gate `load-manifest` `manifest_path` trim/blank semantics and folder-dialog probe `initial_path` trim semantics (`"  path  "` succeeds and roundtrips as canonical path; `"   "` keeps required-path failure where expected)
  - test-gated `/api/automation/test-inject-shortcut` enables non-interactive injector call-path checks (`Cmd+C`) under dry-run mode
  - real injection manual acceptance (`left_click -> Cmd+C`) is user-verified on macOS via one-command selfcheck
  - Phase 53 automation mapping scope is now explicitly closed for M1 in `phase53ai-automation-mapping-phase-closure.md`
  - unified POSIX suite now includes macOS automation injection selfcheck (`--dry-run`) by default
  - WebSettings test-only routes are now isolated in `WebSettingsServer.TestApiRoutes.*`, reducing main route file coupling
  - WebSettings production WASM routes are now isolated in `WebSettingsServer.WasmRoutes.*`, reducing main route file coupling further
  - WebSettings production automation routes are now isolated in `WebSettingsServer.AutomationRoutes.*`, reducing main route file coupling further
  - WebSettings core settings routes are now isolated in `WebSettingsServer.CoreApiRoutes.*`, and main routing file now focuses on request gate + top-level delegation
  - WebSettings request-entry gateway (`token/fallback/exception mapping`) is now isolated in `WebSettingsServer.RequestGateway.*`
  - WebSettings WebUI path discovery is now isolated in `WebSettingsServer.WebUiPathResolver.*`
  - WebSettings WASM routes are now split into `WasmRuntimeRoutes.*`, `WasmCatalogRoutes.*`, and `WasmRouteUtils.*` with delegating entry file
  - WebSettings test routes are now split into `TestAutomationApiRoutes.*`, `TestWasmInputApiRoutes.*`, and `TestRouteCommon.*` with delegating `TestApiRoutes.cpp`
  - WebSettings automation test routes are now further split into `scope/injection/shortcut` layers with shared `TestAutomationRouteUtils.*`, keeping `TestAutomationApiRoutes.cpp` as delegator
  - WebSettings runtime automation routes are now split into `AutomationShortcutCaptureRoutes.*` and `AutomationCatalogRoutes.*` with shared `AutomationRouteUtils.*`, keeping `AutomationRoutes.cpp` as delegator
  - WebSettings WASM catalog routes are now split into `WasmCatalogQueryRoutes.*`, `WasmImportRoutes.*`, and `WasmExportRoutes.*`, keeping `WasmCatalogRoutes.cpp` as delegator
  - WebSettings WASM runtime routes are now split into `WasmRuntimeStateRoutes.*` and `WasmRuntimeActionRoutes.*`, keeping `WasmRuntimeRoutes.cpp` as delegator
  - WebSettings WASM route utils are now split into `WasmRouteParseUtils.cpp`, `WasmRoutePathUtils.cpp`, and `WasmRouteResponseUtils.cpp`, removing monolithic utility implementation coupling
  - WebSettings WASM import routes are now split into `WasmImportSelectedRoute.*` and `WasmImportFolderDialogRoute.*`, keeping `WasmImportRoutes.cpp` as delegator
  - WebSettings WASM runtime state/action internals are now split into endpoint-level route modules (`WasmRuntimeToggleRoutes.*`, `WasmRuntimePolicyRoute.*`, `WasmReloadRoute.*`, `WasmLoadManifestRoute.*`), keeping state/action files as delegators
  - Automation matcher/executor flow is now isolated in `InputAutomationDispatch.*`, while `InputAutomationEngine` remains focused on input event orchestration and state; gesture direction quantization/serialization is further split into `GestureRecognizer.Direction.cpp` to keep recognizer session flow and direction algorithm boundaries separate
  - test-gated `/api/input-indicator/test-keyboard-labels` now verifies keyboard indicator label rendering contract (`A`, `Cmd+K9`, `K6`) in core automation regression
  - `/api/state` `input_capture` now exposes `effects_suspended`, and core automation contracts assert suspension/resume transitions during permission revoke/regrant
  - AppController input-capture runtime/state orchestration is now isolated in `AppController.InputCapture.cpp`, with both CMake and Visual Studio build wiring updated to keep POSIX/Windows lanes aligned
  - AppController lifecycle orchestration (`Start/Stop/CreateDispatchWindow/DestroyDispatchWindow`) is now isolated in `AppController.Lifecycle.cpp`, with both CMake and Visual Studio build wiring updated to keep POSIX/Windows lanes aligned
  - AppController effect orchestration + VM suppression runtime (`SetEffect/ApplyConfiguredEffects/SetTheme/UpdateVmSuppressionState` etc.) is now isolated in `AppController.Effects.cpp`, with both CMake and Visual Studio build wiring updated to keep POSIX/Windows lanes aligned
  - AppController dispatch-state runtime helpers (`OnGlobalKey`, shortcut session lifecycle, hover/hold timers/state) are now isolated in `AppController.DispatchState.cpp`, with both CMake and Visual Studio build wiring updated to keep POSIX/Windows lanes aligned
  - macOS effect routing now covers click/trail/scroll/hold/hover categories (GPU hold routes remain excluded), with pluggable creator registry + shared overlay render support (`main-thread dispatch`/`window setup`) and config-driven render profile mapping (`EffectConfig -> mac click/trail/scroll/hold/hover profile`) to reduce renderer duplication and magic constants drift; type normalization now includes Windows-origin/legacy alias parity for hold (`hold_fluxfield_*`, `hold_neon3d_gpu_v2`, `scifi3d`) and trail/scroll/hover (`stream/stardust/direction/suspension` etc.), trail alias normalization is now shared by style/profile/throttle resolution, and profile tempo/intensity mapping has been tuned (trail/hold/hover + click/scroll) to keep mac visual pacing and visibility closer to Windows defaults without touching GPU route behavior; non-GPU overlays now share frame clamp + per-screen contents-scale + animation curve + alpha policy + geometry metric scaling + size-normalized duration helper policies across click/trail/scroll/hold/hover to reduce edge/multi-screen/mixed-DPI/curve/size/tempo drift, and suite-level policy gate now supports explicit no-ObjC++ edit enforcement; test-gated `/api/effects/test-render-profiles` now exposes resolved profile contracts for regression; settings schema reports `capabilities.effects.trail/scroll/hold/hover=true` on macOS
  - hold route constants now include `hold_fluxfield_cpu` as shared catalog key (`HoldRouteCatalog`), and both metadata/tray matching paths consume catalog constants for FluxField CPU/GPU variants to prevent cross-platform type drift
  - click effect path now has explicit compute/render split: shared core module `ClickEffectCompute` emits `ClickEffectRenderCommand` (normalized type/size/duration/opacity/palette/label), and macOS click renderer consumes that command as execution input; legacy renderer API remains as compatibility wrapper
  - macOS effect mainline now applies the same compute->command pattern to all 5 categories (`click/trail/scroll/hover/hold`): renderer entry points consume shared commands, and `/api/effects/test-render-profiles` now exposes `command_samples` (including `trail_emission`) so settings->command changes are contract-observable
  - macOS `render-profile -> compute-profile` conversion is now centralized in `MacosEffectComputeProfileAdapter` (click/trail/scroll/hover/hold + trail-throttle), and duplicated local builders were removed from effect + overlay + effects-profile-builder paths to reduce multi-point drift
  - Windows `EffectFactory` now applies shared normalize entry by category (click/trail/scroll/hold/hover) before registry/fallback selection, so win/mac effect-type semantic input uses the same normalization source while keeping Windows renderer backends unchanged
  - effects test-profile `command_samples` now also includes cross-category `alias_matrix` contracts (input->normalized), and regression asserts representative alias mappings (`TEXT/stream/stardust/suspension/hold_neon3d_gpu_v2`) to prevent silent type-semantic drift
  - effects test-profile `command_samples` now includes `effective_timing` baseline (click/trail/scroll/hover/hold timing fields), and effects contract regression asserts those fields to support command-level behavior-alignment tracking before pixel-level tuning
  - macOS scroll effect runtime now applies Windows-style input shaper semantics (type-based emit interval + pending-delta merge + duration cap), and `command_samples.effective_timing` now includes `scroll_emit_interval_ms` / `scroll_max_duration_ms` for parity observability
  - shared scroll render command now carries continuous `strength_scalar` + `intensity` (Windows-style), and macOS scroll body geometry now consumes that intensity to smooth high-delta transitions while preserving type/direction semantics
  - shared trail render command now carries continuous `speed_px` + `intensity`, and macOS trail layer styling (stroke/opacity) consumes that intensity to smooth fast-vs-slow movement response while preserving type semantics
  - shared scroll strength quantization now uses non-zero `1..6` levels (`abs(delta)/120` clamp), replacing coarse `0..3` thresholds for closer Windows parity in strength-driven sizing/timing
  - click shared normalization now canonicalizes `icon/icon_star -> star` and `textclick/text_click -> text`; hold route canonicalization now also normalizes `scifi3d -> hologram` and `neon3d -> hold_neon3d` (with lowercase-first alias resolution), and alias-matrix contracts assert these mappings
  - hold follow-mode normalization is now unified via `config_internal::TryNormalizeHoldFollowMode/NormalizeHoldFollowMode` across core apply path, hold compute parser, and scaffold state patch; aliases (`cursor_priority`, `performance_first`, `cpu_saver`, etc.) now canonicalize to (`smooth`/`efficient`) consistently, and effects alias-matrix contracts now expose/assert follow-mode alias mappings
  - effect-type normalization is now unified as single-source in runtime/config/profile paths: `AppController::ResolveRuntimeEffectType` now canonicalizes all five categories (not hold-only), `EffectFactory` now consumes normalized types on both macOS and Windows, and trail alias normalization (`scifi/sci-fi/sci_fi -> tubes`) is shared by `TrailEffectCompute`, `EffectConfig::GetTrailHistoryProfile`, and macOS trail profile/throttle resolution; effects command samples now expose `sample_input.active_raw` + `active_normalized` for active/effective alignment checks
  - macOS effect runtime now canonicalizes effect type once at effect-constructor boundary for all five categories (click/trail/scroll/hover/hold), and registry hold creator no longer does separate pre-normalization; runtime execution now consistently uses canonical ids before render command execution
  - effects contract checks now assert additional alias normalization samples (`click none`, `trail default/scifi`, `scroll none`, `hover none`) and sample-input normalized visibility (`active_raw`/`active_normalized`), and a one-command macOS manual selfcheck is available for five-category type parity (`run-macos-effects-type-parity-selfcheck.sh`)
  - hold-follow normalization contract is now header-inline (`EffectConfigInternal.h`) with local ASCII trim/lower processing, so scaffold/core lanes share one normalization source without introducing linker dependencies in scaffold builds
  - repo-root macOS shortcut launcher `./mfx` is now available for daily loops (`start`/`fast`/`effects`), reducing long manual command-path copy/paste during local testing
  - macOS tray exit path now actively removes status item and calls `NSApp terminate` in `MacosTrayService::RequestExit`, preventing stale menu-bar icon/process residue when clicking tray `Exit`
  - macOS trail `line` rendering now draws from previous emission point to current point (instead of fixed short dash segments), and trail overlay frame expands by `deltaX/deltaY` to avoid clipping at high cursor speed; this closes visible discontinuity where line trail looked like detached short bars
  - macOS `click=text` now routes to shared `TextEffect` (same semantic path as Windows) and uses platform fallback `MacosTextEffectFallback` for transparent floating text animation; this removes pulse-ring `LEFT/RIGHT/MIDDLE` text rendering from text-click mode
  - macOS trail `line` segmented emission path has been removed (back to single-command continuous emission), and `line` path shaping now uses real delta length + tighter throttle (`5ms/1.5px`) to reduce detached "matchstick" segments
- macOS trail `line` path shaping now uses real delta length (no short fixed `14..28px` clamp), and line throttle is tightened to `5ms/1.5px` to reduce dotted/matchstick artifacts
- macOS trail `line` now interpolates multi-segment emissions on large cursor deltas (line-only), so long sparse moves no longer render as isolated "matchsticks"; macOS text-click fallback now renders via `CATextLayer` (no `NSTextField` background), with per-frame font/color updates for Windows-like floating text
- macOS trail `line` now uses a persistent screen-sized path overlay (`MacosLineTrailOverlay`) to accumulate recent points and render a continuous stroke with idle fade, replacing per-pulse windows for `line` only to match Windows behavior under fast movement
- macOS line trail and text-click fallback now convert Quartz input points to Cocoa coords inside the overlay/fallback (avoids overlay-origin drift), and line overlay keeps the window alive briefly when only one point is present to prevent disappearing trails on slow movement
- macOS text-click fallback now exposes `/api/state.effects_runtime.text_effect` diagnostics (click/fallback/panel/error/last text) and prebuilds `NSString` before async dispatch to guard against lifetime-related no-show cases
- macOS text-click fallback now sets `CATextLayer` font via font-name string (valid `CFTypeRef`) to avoid invisible text when a non-bridged font object is rejected by Core Animation
- macOS line trail now clears historical points when the screen-sized overlay window is recreated (avoids stray long lines), uses config-driven line width for thickness, and non-line trail pulses now interpolate large deltas into multiple emissions to reduce matchstick gaps
- Trail 调参页新增 `line_width` 调节（通过 WebUI 直接设置并持久化），无需手改 config.json
- trail 类型 `none` 现在不会再被归一化为 `line`（真正关闭拖尾），并提升非 line 拖尾在快速移动时的分段密度以进一步降低“火柴棍”间隙
- apply_settings 现在会按元数据解析 effect 选项输入（value/alias/中英文 label + Trim），避免“无/None”被误判为未知并回落到 `line`
- macOS `line` 拖尾坐标链路已统一到 `ScreenToOverlayPoint`，并在单点采样时绘制最小可见点；`streamer` 分段长度/插值密度已调优，降低“火柴棍”断续感
- macOS `line` 拖尾分支已增加稀疏 move 事件插值补点（按轨迹分段写入 line overlay），避免输入合并时仅出现零散点/几乎无拖尾
- WebUI “Effects Runtime Profile” 区块改为默认隐藏；仅当 URL 包含 `effects_profile_debug=1`（或 `debug=1`）时显示，避免污染常规设置界面
- macOS move 路由在收到 (0,0) 事件坐标时改用光标查询结果兜底，避免偶发“左上角到鼠标”的直线；macOS trail 在 `none` 时直接返回防止误渲染
  - click routing now forces built-in path when active click type normalizes to `text` (skip wasm click render interception for that type), preventing `click=text` from becoming no-op when WASM route is enabled
  - macOS overlay frame clamp policy now preserves real input anchor (no origin retreat to keep full frame in screen); near-edge effects may clip partially but no longer drift inward
  - `TextEffect` click text path now has macOS-first fallback execution (`MacosTextEffectFallback`) plus empty-text-pool default-label guard (`LEFT/RIGHT/MIDDLE`), and text settings hot-reapply now uses normalized click type check (`NormalizeClickEffectType(config_.active.click) == text`)
  - macOS click overlay color path is now also config-driven (`EffectConfig.ripple.left/right/middle`) via `ClickRenderProfile`, replacing hardcoded per-button click colors to keep user-customized color behavior aligned with Windows
  - macOS scroll/hover overlay color paths are now profile-driven from runtime config/theme resolution (`ScrollRenderProfile` direction colors + `HoverRenderProfile` glow/tubes colors), replacing hardcoded constants while preserving direction/type visual differences
  - macOS trail/hold overlay color paths are now profile-driven from runtime config resolution (`TrailRenderProfile` type colors + `HoldRenderProfile` base/style colors), replacing hardcoded constants while preserving existing effect type/style semantics
  - effects profile contracts now include color fields in both test route (`/api/effects/test-render-profiles`) and state diagnostics (`/api/state.effects_profile`), and core automation contracts assert representative color keys to guard parity drift
  - macOS trail/scroll/hover render plans now include type-aware tempo/size scaling, so runtime motion pacing differs by effect type instead of relying on mostly uniform defaults
  - trail/scroll/hover tempo scales are now surfaced in both test-route and state diagnostics contracts, and core automation regression asserts representative tempo fields to guard against silent parity drift
  - effects profile contract checks now assert cross-API value parity (`/api/effects/test-render-profiles` vs `/api/state.effects_profile`) for representative tempo/color fields
  - `/api/state` now exposes non-test diagnostics `effects_profile` (`active` + `config_basis` + resolved click/trail/scroll/hold/hover profile fields), effects settings UI now renders that snapshot in a read-only diagnostics card with one-click JSON copy, and core state regression asserts those fields to guard profile-mapping drift
  - `SettingsStateMapper` effects diagnostics are now isolated in `SettingsStateMapper.EffectsDiagnostics.cpp` (runtime overlay counters + profile snapshot), keeping `SettingsStateMapper.Diagnostics.cpp` focused on GPU/WASM/input-capture diagnostics while preserving `/api/state.effects_runtime` and `/api/state.effects_profile` contracts
  - test-only effects routes are now split by capability (`WebSettingsServer.TestEffectsProfileApiRoute.*` + `WebSettingsServer.TestEffectsOverlayApiRoute.*`) with `WebSettingsServer.TestEffectsApiRoutes.cpp` as delegator, reducing route-level coupling while preserving `/api/effects/test-render-profiles` and `/api/effects/test-overlay-windows` contracts
  - macOS hold overlay renderer now delegates hold-style normalization/color/path-accent construction to `MacosHoldPulseOverlayStyle.*`, keeping `MacosHoldPulseOverlayRenderer.mm` lifecycle/state-focused without changing public hold overlay API contracts
  - macOS trail overlay renderer now delegates trail-style normalization/color/path construction to `MacosTrailPulseOverlayStyle.*`, keeping `MacosTrailPulseOverlayRenderer.mm` lifecycle/animation-focused without changing public trail overlay API contracts
  - macOS click overlay renderer now delegates click-type normalization/star-path construction to `MacosClickPulseOverlayStyle.*`, keeping `MacosClickPulseOverlayRenderer.mm` lifecycle/animation-focused without changing public click overlay API contracts
  - macOS scroll overlay renderer now delegates scroll-type normalization to `MacosScrollPulseOverlayStyle.*`, keeping `MacosScrollPulseOverlayRenderer.mm` lifecycle/animation-focused without changing public scroll overlay API contracts
  - macOS hover overlay renderer now delegates hover-type normalization and glow/tubes palette constants to `MacosHoverPulseOverlayStyle.*`, keeping `MacosHoverPulseOverlayRenderer.mm` lifecycle/animation-focused without changing public hover overlay API contracts
  - effects profile diagnostics assembly is now isolated in `SettingsStateMapper.EffectsProfileStateBuilder.*`, keeping `SettingsStateMapper.EffectsDiagnostics.cpp` focused on runtime counters and delegation while preserving `/api/state.effects_profile` contracts
  - macOS effect profile resolution is now split by category (`MacosEffectRenderProfile.ClickTrail.cpp` + `MacosEffectRenderProfile.ScrollHoldHover.cpp` + shared helper header), and state diagnostics are now split by concern (`SettingsStateMapper.WasmDiagnostics.cpp` + `SettingsStateMapper.InputCaptureDiagnostics.cpp`), keeping contracts unchanged while reducing single-file coupling
  - macOS click/scroll renderers are now split into thin wrappers plus dedicated core units (`MacosClickPulseOverlayRendererCore.*`, `MacosScrollPulseOverlayRendererCore.*`), reducing renderer-file coupling while preserving behavior and API contracts
  - macOS hold/trail renderers are now split into thin wrappers plus dedicated core units (`MacosHoldPulseOverlayRendererCore.*`, `MacosTrailPulseOverlayRendererCore.*`), and hold core start flow is further isolated in `MacosHoldPulseOverlayRendererCore.Start.mm`, reducing renderer-file coupling while preserving behavior and API contracts
  - macOS effect probe route now accepts per-category type arguments (`click/trail/scroll/hold/hover`), and core automation contracts exercise non-default type matrix (`text/electric/helix/hold_quantum_halo_gpu_v2/tubes`)
  - Linux compile gate now validates both default lane and core-runtime lane by default (`MFX_ENABLE_POSIX_CORE_RUNTIME=OFF/ON`) with optional fast-path skip flag
  - Phase 54 Linux follow scope is now explicitly closed for compile+contract boundary in `phase54i-linux-follow-phase-closure.md`
  - `SettingsStateMapper` is now split into `BaseSections.*` and `Diagnostics.*` with top-level composition kept in `SettingsStateMapper.cpp`
  - `SettingsSchemaBuilder` is now split into `OptionsSections.*` and `CapabilitiesSections.*` with top-level composition kept in `SettingsSchemaBuilder.cpp`
  - `HttpServer` is now split into lifecycle/session/protocol layers (`HttpServer.Lifecycle.cpp`, `HttpServer.ClientSession.cpp`, `HttpServer.Protocol.cpp`) with thin `HttpServer.cpp` entry
  - test-gated `/api/input-indicator/test-mouse-labels` enables non-interactive mac indicator label contract checks (`L/R/M`)
  - `/api/automation/active-process` now guarantees non-empty `process` on macOS via foreground-query fallback chain
  - schema capability `capabilities.input.keyboard_injector` now reports true on macOS (aligned with runtime injector wiring)
  - core automation contract now also guards WebUI shell asset loading (`/settings-shell.svelte.js?token=<token>`) and shell settings-launcher URL call-path (probe/capture files)
  - macOS tray smoke now guards tray `Settings` action callback -> settings launcher URL call-path using test-only auto-trigger and launch-capture probes
  - core automation contract now simulates startup/runtime permission transitions (`trusted=0/1`) and asserts degraded/recovery + startup notify dedup
  - core automation contract now guards app-catalog refresh (`force=true/false`) and selected-app scope state roundtrip
  - unified POSIX suite now runs macOS WASM runtime selfcheck by default (with explicit skip flag), reducing manual acceptance drift
  - WASM diagnostics now expose machine-readable load-failure fields (`last_load_failure_stage`, `last_load_failure_code`) and invalid-manifest selfcheck now asserts `manifest_load/manifest_io_error`
  - macOS WASM selfcheck now covers full manifest load-failure classification (`manifest_io_error`, `manifest_json_parse_error`, `manifest_invalid`) to lock failure-code semantics in regression
  - macOS WASM selfcheck now also covers stage-level load failures (`manifest_api_version`, `load_module`) and asserts load-failure field reset after a valid reload
  - macOS WASM selfcheck helper stack is now modularized (`parse/http/runtime-assert/transfer-assert/dispatch-assert` modules) with compatibility loader entry retained in `tools/platform/manual/lib/wasm_selfcheck_common.sh`
  - core HTTP contract regression now asserts WASM load-failure diagnostics semantics (`last_load_failure_stage/code`) for success, invalid-manifest failure, and reload-clear paths
  - core HTTP contract regression now also asserts WASM transfer semantics (`import-selected` success+failure and `export-all` success with minimum count guard)
  - core HTTP contract regression now also asserts WASM export filesystem consistency (`export_path` existence and exported directory count == response count)
  - core HTTP contract regression now also asserts WASM export manifest integrity (`plugin.json` count/exists/non-empty under export path)
  - WASM transfer APIs now expose stable `error_code` fields (`import-selected`, `export-all`, folder-dialog import), and regression asserts import failure code semantics
  - shared Svelte WASM UI now surfaces transfer `error_code` in operation feedback, backed by dedicated error-code mapping module
  - transfer failure-code regression matrix now covers `manifest_path_required/not_file/load_failed/not_found`, and WebUI prefers translated error-code mapping before raw backend error text
  - POSIX suite webui semantic phase now includes `test:automation-platform` + `test:effects-profile-model` + `test:wasm-error-model`, keeping platform semantics, effects profile mapping, and WASM error-code behavior gated by default
  - shared Svelte WASM diagnostics panel now surfaces `last_load_failure_stage/code` with EN/ZH i18n labels and warning-state linkage
  - shared Svelte WASM state normalization is now deduplicated via `WebUIWorkspace/src/wasm/state-model.js`, reducing cross-file drift risk
  - core HTTP WASM regression helper stack is now modularized (`parse/http/runtime-assert/transfer-assert/dispatch-assert`) with compatibility loader entry retained in `tools/platform/regression/lib/core_http_wasm_helpers.sh`
  - core HTTP WASM contract checks are now split by scenario (catalog/path/runtime/transfer/fixture/dispatch/platform) with `core_http_wasm_contract_checks.sh` as orchestrator
  - core HTTP automation contract checks are now split by scenario (basic/app-scope/priority/match-inject/shortcut/indicator/effects/platform) with `core_http_automation_contract_checks.sh` as orchestrator
  - core HTTP orchestrator is now helper-split (probe/entry/state checks) with `core_http.sh` as the top-level workflow
  - core HTTP regression now supports scoped checks (`all` default, `wasm` focused) so WASM closure can run a dedicated gate without full automation/input contracts
  - WASM plugin transfer service is now split by responsibility (`Common` helpers + `Import` flow + `Export` flow + thin delegator entry), reducing transfer-path coupling while preserving import/export error-code contracts
  - macOS WASM selfcheck now also covers transfer/error-code contracts (`catalog`, folder-dialog probe, import-selected success/failure codes, export-all consistency) in the same one-command flow
  - core HTTP WASM contract execution is now isolated in `tools/platform/regression/lib/core_http_wasm_contract_checks.sh`, and `core_http.sh` now focuses on lifecycle + non-WASM orchestration boundaries
  - core HTTP non-WASM contracts are now isolated in `tools/platform/regression/lib/core_http_input_contract_checks.sh` and `tools/platform/regression/lib/core_http_automation_contract_checks.sh`, and input-capture helpers are split by concern (parse/permission/notification/state/steps) to reduce cross-domain coupling in `core_http.sh`
  - core regression entry scripts (`core-smoke`, `core-automation`, `core-wasm`) now share lock-guarded host resource scheduling (`mfx-entry-posix-host`) to avoid concurrent local run interference
  - macOS hover renderer and effect creator registry are now split by responsibility (`MacosHoverPulseOverlayRendererCore.*`, `MacosEffectCreatorRegistry.Table.cpp` + `MacosEffectCreatorRegistry.Internal.h`), keeping wrapper/entry files small while preserving effect creation/render contracts
  - macOS manual core-entry scripts (`run-macos-core-websettings-manual.sh`, `run-macos-automation-injection-selfcheck.sh`, `run-macos-wasm-runtime-selfcheck.sh`) now share the same host lock guard with regression scripts
  - POSIX suite preflight is now detect-only for `mfx_entry_posix_host`; cleanup stays phase-local under `mfx-entry-posix-host` lock (no suite-level force kill)
  - stale entry-host cleanup now uses one shared helper (`mfx_terminate_stale_entry_host`) across core regression workflows and macOS manual host startup
  - manual entry-lock acquire path is now helperized (`mfx_manual_acquire_entry_host_lock`), and keep-running stop hints are PID-scoped (`kill -TERM <pid>`) to avoid broad process-pattern termination
  - regression file-content checks now use shared `rg`-preferred with `grep` fallback helpers, and entry scripts no longer hard-require `rg`
  - POSIX regression entry scripts now share helperized host-platform detection and `--platform` resolution to keep cross-host guard semantics centralized
  - core regression entry scripts now share helperized workflow preparation and lock execution (`mfx_prepare_core_entry_runtime`, `mfx_run_with_entry_lock`)
  - wasm test-dispatch assertions in regression/manual selfchecks now use bounded retries to reduce transient invoke/render readiness flakiness
  - wasm test-dispatch checks now also assert diagnostics consistency against `/api/state` (`throttled total == capacity+interval`, and dispatch vs state counters/error snapshot match)
  - WASM plugin manifest path is now split into `Load` and `Validate` units (`WasmPluginManifest.Load.cpp`, `WasmPluginManifest.Validate.cpp`), reducing parse-vs-rule coupling while preserving manifest contracts
  - macOS `MacosGlobalInputHook` implementation is now split by responsibility (`MacosGlobalInputHook.mm`, `.EventTap.mm`, `.RunLoop.mm`) to lower file coupling without behavior changes
  - macOS input-indicator overlay path is now split into thin entry (`MacosInputIndicatorOverlay.mm`), lifecycle (`MacosInputIndicatorOverlay.Lifecycle.mm`), display (`MacosInputIndicatorOverlay.Display.mm`), probe/event-entry (`MacosInputIndicatorOverlay.Probes.mm`), and shared internals (`MacosInputIndicatorOverlayInternals.*`)
  - macOS trail pulse core is now split by concern (`render plan` + `layers/animation` + orchestration entry), reducing trail-effect render coupling while preserving effect contracts
  - macOS wasm text/image overlays now share render math boundary (`MacosWasmOverlayRenderMath.*`) for clamp/color/lifetime rules, reducing duplicated tuning logic while preserving overlay behavior contracts
  - macOS wasm command dispatch is now split into entry + text + image/affine handlers (`MacosWasmCommandRenderDispatch.mm`, `.Text.mm`, `.Image.mm`) with shared internal contract, reducing single-file command-path coupling while preserving runtime behavior contracts
  - macOS keyboard injector key tables are now split by mapping domain (`Printable`, `Function`, `Special`, `Modifier`) and no longer share one monolithic implementation file, reducing shortcut-mapping change blast radius
  - macOS AppleScript folder picker is now split into entry/thread dispatch, string/path helper, and execute pipeline modules (`MacosAppleScriptFolderPicker.Script/Execution/StringUtils.*`), reducing picker change coupling while preserving folder-selection behavior
  - macOS global input hook event-tap path is now split into callback routing (`MacosGlobalInputHook.EventTap.mm`) and per-event dispatch handlers (`MacosGlobalInputHook.EventTapDispatch.mm`), reducing input-ingress coupling while preserving runtime event semantics
  - macOS input-indicator overlay is now split into lifecycle/orchestration (`MacosInputIndicatorOverlay.mm`) and style/layout helper module (`MacosInputIndicatorOverlay.Style.*`), reducing overlay UI tuning blast radius
  - macOS keyboard injector now splits dry-run/event-post internals into dedicated module (`MacosKeyboardInjector.EventPost.mm`), reducing chord orchestration vs low-level posting coupling while preserving injection contracts
  - macOS app-catalog bundle resolve path now splits process/display resolution helpers into dedicated module (`MacosApplicationCatalogScanWorkflow.BundleResolve.Helpers.mm`) while keeping bundle-resolve file focused on entry orchestration
  - macOS WASM image overlay renderer now splits render-plan (`MacosWasmImageOverlayRendererCore.Plan.mm`) and window composition (`MacosWasmImageOverlayRendererCore.Window.mm`) from orchestration entry (`MacosWasmImageOverlayRendererCore.mm`), reducing scheduling-vs-render coupling while preserving image overlay contracts
  - macOS WASM overlay state now splits admission/reset internals (`MacosWasmOverlayState.Admission.mm`) and window-set ownership operations (`MacosWasmOverlayState.Windows.mm`) from API wrappers, reducing admission-vs-window-state coupling while preserving throttle/admission contracts
  - macOS input-indicator probe path now splits common probe setup/capture logic into helper module (`MacosInputIndicatorOverlay.ProbeHelpers.mm`), reducing duplicated probe logic while preserving probe contracts
  - macOS overlay coordinate conversion now splits service/origin state from Quartz->Cocoa conversion internals (`MacosOverlayCoordSpaceService` vs `MacosOverlayCoordSpaceConversion.*`), reducing coord-space coupling while preserving conversion behavior contracts
  - macOS global-input-hook runloop path now splits init/simulation/mask helpers into dedicated module (`MacosGlobalInputHook.RunLoopHelpers.mm`), reducing runloop core coupling while preserving input-hook runtime behavior contracts
  - macOS keyboard-injector resolver now splits non-modifier fallback chain into dedicated helper module (`MacosKeyboardInjectorKeyResolver.NonModifier.mm`), reducing resolver branching coupling while preserving key-resolution contracts
  - macOS virtual-key mapper now splits key-pair table ownership into dedicated module (`MacosVirtualKeyMapper.KeyPairs.mm`), reducing table-vs-entry coupling while preserving keycode normalization contracts
  - macOS global-input event-tap dispatch now splits tap-disabled recovery and mouse/key event dispatch into dedicated modules (`MacosGlobalInputHook.EventTapDispatch*.mm`), reducing input-ingress coupling while preserving dispatch semantics
  - macOS user notification service now splits AppleScript/test-capture helpers into dedicated module (`MacosUserNotificationService.AppleScript.cpp`), reducing warn-entry vs notification-backend coupling while preserving degraded-warning contracts
  - macOS warning notification Swift bridge now uses native notification-center delivery first (`NSUserNotificationCenter`) with AppleScript fallback only when native path is unavailable, reducing script-process side effects while preserving existing warning contracts
  - macOS event-loop service now splits runloop resource lifecycle into dedicated module (`MacosEventLoopService.RunLoop.cpp`), reducing service-flow vs runloop-resource coupling while preserving shell event-loop contracts
  - `PosixSettingsLauncher` and `ScaffoldSettingsRuntime` are now split by concern (`capture/spawn` and `runtime-start orchestration` modules), reducing shell runtime coupling while preserving URL launch and scaffold server contracts
  - `AppController` VM suppression path is now isolated in `AppController.VmSuppression.cpp`, reducing suppression-vs-effects coupling while preserving suppression behavior contracts
  - macOS effect overlay lifecycle now exposes 5-category active-window diagnostics (`click/trail/scroll/hold/hover`) in `/api/state.effects_runtime`, and probe coverage (`/api/effects/test-overlay-windows`) now exercises all 5 categories with persistent-overlay close control
  - macOS effects profile resolution now supports explicit test-only tuning overrides (`MFX_TEST_EFFECTS_DURATION_SCALE`, `MFX_TEST_EFFECTS_SIZE_SCALE`, `MFX_TEST_EFFECTS_OPACITY_SCALE`, `MFX_TEST_EFFECTS_TRAIL_THROTTLE_SCALE`) with bounded clamps and cross-API diagnostics parity (`/api/effects/test-render-profiles` + `/api/state.effects_profile`); automation contracts can assert non-default override values via `MFX_EXPECT_EFFECTS_*`
  - posix regression suite now includes macOS effects tuning selfcheck phase by default (`run-macos-effects-profile-tuning-selfcheck.sh`), with skip switch `--skip-macos-effects-tuning-selfcheck`
  - core HTTP automation regression now supports effects-only scope (`--check-scope effects`) and dedicated fast entry (`run-posix-core-effects-contract-regression.sh`) for effects overlay/profile contract checks
  - posix suite now forwards core automation check scope via `--core-automation-check-scope <all|wasm|effects>`, enabling effects-focused suite passes without switching entry scripts
  - effects-focused suite shortcut entry is now available: `tools/platform/regression/run-posix-effects-regression-suite.sh` (pins `--core-automation-check-scope effects` and skips macOS wasm selfcheck by default)
  - wasm-focused suite shortcut entry is now available: `tools/platform/regression/run-posix-wasm-regression-suite.sh` (pins `--core-automation-check-scope wasm` and skips non-wasm mac selfchecks)
  - scope-pinned shortcut entries now reject conflicting scope arguments (`run-posix-core-effects-contract-regression.sh`, `run-posix-effects-regression-suite.sh`, `run-posix-wasm-regression-suite.sh`) for both `--arg value` and `--arg=value` forms, preventing accidental scope override drift
  - conflicting-scope rejection in shortcut entries is now implemented via shared helper (`mfx_reject_option_in_args`), reducing duplicate shell logic and keeping failure semantics consistent
  - manual macOS core-host selfcheck teardown now uses bounded staged stop (`TERM` with timeout, then `KILL` fallback via `MFX_MANUAL_STOP_TIMEOUT_SECONDS`) to reduce leftover host-process interference between repeated selfchecks
  - macOS manual selfcheck scripts now acquire `mfx-entry-posix-host` lock before optional build phase (not only before host startup), reducing build/start races under concurrent manual/suite execution
  - macOS manual selfcheck scripts now reuse a single host-binary prepare helper (`mfx_manual_prepare_core_host_binary`), unifying core-build flags and executable validation across automation/effects/wasm/websettings checks
  - macOS manual selfcheck scripts now also share common numeric option validation helpers (`mfx_manual_apply_build_jobs_env`, `mfx_manual_validate_non_negative_integer`) for consistent `--jobs` / `--auto-stop-seconds` contracts
  - `cmake` dependency checks for macOS manual selfchecks are now centralized in host-binary prepare helper and only enforced when build is requested (`--skip-build=0`), avoiding unnecessary dependency gating in skip-build flows
  - `/api/effects/test-render-profiles` now reuses `BuildEffectsProfileStateJson` as the single profile/config-basis assembly source, reducing duplicate resolver logic and test-route/state-route drift risk
  - effects test-profile route now also reuses `effects_profile.active` payload (including `hold`/`hover`), and effects contract checks assert active hold/hover visibility to guard route/state shape parity
  - effects contract parity now includes value-level active matching (`click/trail/scroll/hold/hover`) between `/api/effects/test-render-profiles` and `state.effects_profile`, not only section existence checks
  - effects contract/profile-tuning selfchecks now use float-tolerance assertions for test tuning scales (`duration/size/opacity/trail_throttle`), avoiding false negatives caused by JSON float string formatting differences
  - effects contract parity now also validates per-section base-opacity consistency (`click/trail/scroll/hold/hover`) between `/api/effects/test-render-profiles` and `state.effects_profile`
  - WebUI effects profile model test coverage now includes `config_basis.test_tuning.opacity_scale/opacity_overridden` and section `base_opacity` (`click/trail/scroll/hold/hover`) passthrough checks
  - core HTTP regression startup now tolerates missing core probe file by recovering `url/token` from launch-probe URL when available, reducing false negatives caused by probe write timing gaps
  - effects contract shortcut entry (`run-posix-core-effects-contract-regression.sh`) now defaults to non-default test tuning values (`duration=0.5`, `size=1.2`, `opacity=0.78`, `trail_throttle=0.6`) and auto-aligns `MFX_EXPECT_EFFECTS_*`, so each effects run validates override paths by default
  - core HTTP startup requirement is now scope-aware: launch-probe is required only for `all` scope (input-launch contracts), while `effects/wasm` scopes no longer fail on missing launch-probe artifacts
  - POSIX core regression probe emission now treats launch-open failure as non-fatal for core probe output (launch result remains observable in launch-probe), and effects/wasm scope startup no longer injects launch-probe env variables
  - core HTTP startup now captures probe diagnostics file (`MFX_CORE_WEB_SETTINGS_PROBE_DIAGNOSTICS_FILE`) and prints `status/reason` on startup failure, improving root-cause visibility for probe emission failures
  - WebSettings startup diagnostics now include HTTP server start stage/code (`websettings_start_failed(stage=<n>,code=<err>)`) from `HttpServer`, so probe-start failures can be triaged by socket/bind/listen/thread stage
  - core HTTP startup failure logging now adds explicit hint for `websettings_start_failed(stage=2,code=1)` (bind `EACCES`) to distinguish environment/sandbox permission blocks from product regressions
  - effects contract shortcut entry now has dedicated `--help` output with its injected default test tuning values and forwarded options, reducing wrapper-vs-core usage ambiguity
  - effects contract shortcut now defaults `MFX_CORE_HTTP_ALLOW_BIND_EACCES_SKIP=1`; when startup diagnostics detect `websettings_start_failed(stage=2,code=1)`, effects scope exits as explicit `skipped` instead of hard failure (to avoid environment-sandbox false negatives)
  - wasm contract shortcut now also defaults `MFX_CORE_HTTP_ALLOW_BIND_EACCES_SKIP=1`, aligning constrained-runtime skip behavior with effects contract shortcut
  - macOS effects-profile JSON assembly is split into dedicated builder module (`SettingsStateMapper.EffectsProfileStateBuilder.Macos.cpp`), while `SettingsStateMapper.EffectsProfileStateBuilder.cpp` keeps platform-neutral envelope logic only
  - regression startup/stop numeric env tuning is now tolerant to invalid inputs (falls back to defaults in `http_entry_helpers`/`core_http_entry_helpers`), reducing accidental shell-env misconfiguration failures during local loops
  - core automation scope values are now normalized by shared helper (`mfx_normalize_core_automation_check_scope`) across suite/core-http/core-automation entry points, keeping scope contract parsing consistent (supports mixed-case inputs like `EFFECTS`)
  - regression `mfx_fail` output now writes to stderr, so invalid-option diagnostics remain visible even when failures occur inside command-substitution call paths
  - core HTTP regression stop/HTTP helpers are now timeout-hardened (`MFX_CORE_HTTP_STOP_TIMEOUT_SECONDS`, `MFX_HTTP_CONNECT_TIMEOUT_SECONDS`, `MFX_HTTP_MAX_TIME_SECONDS`) to avoid indefinite hangs in wasm/effects/all scopes
  - core HTTP regression stop path now uses staged shutdown (`stdin-exit -> TERM -> KILL`) with dedicated waits (`MFX_CORE_HTTP_GRACEFUL_STOP_WAIT_SECONDS`, `MFX_CORE_HTTP_TERM_WAIT_SECONDS`) to reduce unnecessary tail-latency in repeated wasm/effects loops
  - scaffold HTTP entry startup now includes bounded early-exit retry (`MFX_HTTP_ENTRY_START_RETRIES`, default `1`) with per-attempt fifo cleanup to reduce transient false-negative regressions
  - scaffold regression (`run-posix-scaffold-regression.sh`) now runs smoke/HTTP checks under shared entry-host lock (`mfx-entry-posix-host`) to prevent concurrent suite runs from causing startup false negatives
  - posix full suite (`run-posix-regression-suite.sh`) now runs under suite-level lock (`mfx-posix-regression-suite`, timeout via `MFX_POSIX_SUITE_LOCK_TIMEOUT_SECONDS`) so concurrent invocations serialize instead of cross-interfering
  - shared WebUI effects profile model now preserves backend `config_basis` diagnostics (including `test_tuning`) instead of dropping it during normalization

## Known Stable Gates
Run these as first-line regression checks:
```bash
./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto
./tools/platform/regression/run-posix-core-smoke.sh --platform auto
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto
./tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto
./tools/platform/regression/run-posix-core-wasm-contract-regression.sh --platform auto
./tools/platform/regression/run-posix-core-wasm-path-contract-regression.sh --platform auto
./tools/platform/regression/run-posix-linux-compile-gate.sh --build-dir /tmp/mfx-platform-linux-build --jobs 8
./tools/platform/regression/run-posix-regression-suite.sh --platform auto
./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build
```

## macOS Manual Runner
Use this one-command entry for manual WebSettings verification on macOS core lane:
```bash
./tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60
```

Use this one-command entry for WASM runtime invoke/render/fallback selfcheck:
```bash
./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build
```

Use this one-command entry for automation injection selfcheck (`left_click -> Cmd+C` path):
```bash
./tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build
```

Use this one-command entry for effects profile tuning selfcheck (test-only `duration/size/opacity/trail-throttle` overrides):
```bash
./tools/platform/manual/run-macos-effects-profile-tuning-selfcheck.sh --skip-build
```

## Current Next Slice
- Continue M2 with macOS-first WASM quality and contract hardening.
- Keep platform abstraction reusable for Linux follow-up.
- Keep Windows behavior unchanged unless explicit approved scope.

## Behavior Contracts to Preserve
1. Permission loss on macOS should degrade safely, keep process alive, and notify clearly.
2. Permission restore should recover without requiring process restart.
3. Scaffold lane contracts must remain unchanged.
4. Core lane API contracts must stay backward-compatible.
5. Web settings must remain Svelte-based and shared across platforms.
6. WASM plugin catalog/import/export API semantics should stay aligned across Windows and POSIX core lane.

## High-Value Docs (Read on Demand)
- Roadmap status: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
- Doc governance: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/agent-doc-governance.md`
- Key WASM hardening docs:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55o-macos-wasm-closure.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55q-posix-wasm-load-failure-diagnostics-contract.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zd-wasm-transfer-error-code-regression-matrix-and-i18n.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55ze-webui-wasm-error-model-test-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zf-wasm-focused-contract-gate-and-selfcheck-expansion.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zo-posix-platform-arg-helper-consolidation.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zp-doc-index-compaction-p0-p1.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zq-core-regression-workflow-helper-consolidation.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zr-wasm-dispatch-readiness-retry-hardening.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzc-macos-app-catalog-workflow-secondary-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzd-macos-wasm-overlay-runtime-state-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zze-macos-scroll-pulse-overlay-internals-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzp-wasm-fixture-helper-consolidation.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzq-webui-wasm-runtime-error-code-mapping.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzr-webui-wasm-error-i18n-parity-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzs-wasm-manifest-path-trim-contract-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzt-wasm-selfcheck-helper-modularization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzu-core-http-wasm-helper-modularization-and-lock-race-hardening.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzv-core-http-wasm-contract-check-modularization.md`
- Phase closure docs: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53ai-automation-mapping-phase-closure.md`, `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54i-linux-follow-phase-closure.md`
## AI-IDE Context Loading Rule
- Read this file first; read only one targeted phase/issue doc per task; avoid bulk historical lists.
## Update Checklist (per capability change)
1. Update targeted phase/issue doc.
2. If behavior/contract changed, update this file.
3. If navigation changed, update docs README indexes.

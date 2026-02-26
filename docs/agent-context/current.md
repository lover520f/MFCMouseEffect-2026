# Agent Current Context (2026-02-25)

## Scope and Priority
- Primary dev host: macOS.
- Primary target: macOS usable loop first.
- Constraints: no Windows regression; Linux follows compile + contract coverage.

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
  - Automation matcher/executor flow is now isolated in `InputAutomationDispatch.*`, while `InputAutomationEngine` remains focused on input event orchestration and state
  - test-gated `/api/input-indicator/test-keyboard-labels` now verifies keyboard indicator label rendering contract (`A`, `Cmd+K9`, `K6`) in core automation regression
  - `/api/state` `input_capture` now exposes `effects_suspended`, and core automation contracts assert suspension/resume transitions during permission revoke/regrant
  - AppController input-capture runtime/state orchestration is now isolated in `AppController.InputCapture.cpp`, with both CMake and Visual Studio build wiring updated to keep POSIX/Windows lanes aligned
  - AppController lifecycle orchestration (`Start/Stop/CreateDispatchWindow/DestroyDispatchWindow`) is now isolated in `AppController.Lifecycle.cpp`, with both CMake and Visual Studio build wiring updated to keep POSIX/Windows lanes aligned
  - AppController effect orchestration + VM suppression runtime (`SetEffect/ApplyConfiguredEffects/SetTheme/UpdateVmSuppressionState` etc.) is now isolated in `AppController.Effects.cpp`, with both CMake and Visual Studio build wiring updated to keep POSIX/Windows lanes aligned
  - AppController dispatch-state runtime helpers (`OnGlobalKey`, shortcut session lifecycle, hover/hold timers/state) are now isolated in `AppController.DispatchState.cpp`, with both CMake and Visual Studio build wiring updated to keep POSIX/Windows lanes aligned
  - macOS effect routing now covers click/trail/scroll/hold/hover categories (GPU hold routes remain excluded), with pluggable creator registry + shared overlay render support (`main-thread dispatch`/`window setup`) and config-driven render profile mapping (`EffectConfig -> mac click/trail/scroll/hold/hover profile`) to reduce renderer duplication and magic constants drift; test-gated `/api/effects/test-render-profiles` now exposes resolved profile contracts for regression; settings schema reports `capabilities.effects.trail/scroll/hold/hover=true` on macOS
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
  - macOS hold/trail renderers are now split into thin wrappers plus dedicated core units (`MacosHoldPulseOverlayRendererCore.*`, `MacosTrailPulseOverlayRendererCore.*`), reducing renderer-file coupling while preserving behavior and API contracts
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
  - macOS input-indicator overlay path is now split into render/lifecycle (`MacosInputIndicatorOverlay.mm`), probe/event-entry (`MacosInputIndicatorOverlay.Probes.mm`), and shared internals (`MacosInputIndicatorOverlayInternals.*`)
  - `DispatchRouter` is now split by responsibility (`DispatchRouter.cpp` + `DispatchRouter.Pointer.cpp` + shared helper boundary), reducing pointer/timer-vs-entry coupling while preserving dispatch behavior contracts
  - macOS wasm text/image overlays now share render math boundary (`MacosWasmOverlayRenderMath.*`) for clamp/color/lifetime rules, reducing duplicated tuning logic while preserving overlay behavior contracts
  - macOS wasm command dispatch is now split into entry + text + image/affine handlers (`MacosWasmCommandRenderDispatch.mm`, `.Text.mm`, `.Image.mm`) with shared internal contract, reducing single-file command-path coupling while preserving runtime behavior contracts
  - macOS keyboard injector key tables are now split by mapping domain (`Printable`, `Function`, `Special`, `Modifier`) and no longer share one monolithic implementation file, reducing shortcut-mapping change blast radius
  - macOS AppleScript folder picker is now split into entry/thread dispatch, string/path helper, and execute pipeline modules (`MacosAppleScriptFolderPicker.Script/Execution/StringUtils.*`), reducing picker change coupling while preserving folder-selection behavior
  - macOS global input hook event-tap path is now split into callback routing (`MacosGlobalInputHook.EventTap.mm`) and per-event dispatch handlers (`MacosGlobalInputHook.EventTapDispatch.mm`), reducing input-ingress coupling while preserving runtime event semantics
  - macOS input-indicator overlay is now split into lifecycle/orchestration (`MacosInputIndicatorOverlay.mm`) and style/layout helper module (`MacosInputIndicatorOverlay.Style.*`), reducing overlay UI tuning blast radius
  - macOS wasm image overlay renderer now splits utility helpers into support module (`MacosWasmImageOverlayRendererSupport.*`), reducing renderer-core utility coupling while preserving render behavior contracts
  - macOS app-catalog scan workflow now splits bundle-resolution and root-traversal responsibilities (`MacosApplicationCatalogScanWorkflow.BundleResolve.mm`, `MacosApplicationCatalogScanWorkflow.RootScan.mm`), reducing scan helper coupling while preserving app-catalog semantics
  - macOS hold effect core now splits shared state and update responsibilities (`MacosHoldPulseOverlayRendererCore.Internal/State/Update`), reducing hold-renderer core coupling while preserving visual behavior contracts
  - macOS scroll effect core now splits strength/timing/layer construction helpers into support module (`MacosScrollPulseOverlayRendererSupport.*`), reducing scroll-renderer core utility coupling while preserving visual behavior contracts
  - macOS input-indicator probe path now splits common probe setup/capture logic into helper module (`MacosInputIndicatorOverlay.ProbeHelpers.mm`), reducing duplicated probe logic while preserving probe contracts
  - macOS overlay coordinate conversion now splits service/origin state from Quartz->Cocoa conversion internals (`MacosOverlayCoordSpaceService` vs `MacosOverlayCoordSpaceConversion.*`), reducing coord-space coupling while preserving conversion behavior contracts
  - macOS global-input-hook runloop path now splits init/simulation/mask helpers into dedicated module (`MacosGlobalInputHook.RunLoopHelpers.mm`), reducing runloop core coupling while preserving input-hook runtime behavior contracts
  - macOS keyboard-injector resolver now splits non-modifier fallback chain into dedicated helper module (`MacosKeyboardInjectorKeyResolver.NonModifier.mm`), reducing resolver branching coupling while preserving key-resolution contracts
  - macOS virtual-key mapper now splits key-pair table ownership into dedicated module (`MacosVirtualKeyMapper.KeyPairs.mm`), reducing table-vs-entry coupling while preserving keycode normalization contracts
  - `WasmEffectHost` invoke path is now isolated in `WasmEffectHost.Invoke.cpp`, reducing lifecycle-vs-invoke coupling while preserving host execution contracts
  - `AppController` VM suppression path is now isolated in `AppController.VmSuppression.cpp`, reducing suppression-vs-effects coupling while preserving suppression behavior contracts
  - macOS click pulse overlay internals now isolate style/renderer/window-registry modules (`MacosClickPulseOverlayStyle.*`, `MacosClickPulseOverlayRenderer.*`, `MacosClickPulseWindowRegistry.*`) while `MacosClickPulseEffect.mm` remains lifecycle/event-entry facade
  - macOS hold overlay renderer now uses explicit type->style mapping (`charge/lightning/hex/tech/neon/hologram/quantum_halo/flux_field`) for differentiated visuals, and hover alias `suspension` is normalized to `tubes`
  - macOS effect overlay lifecycle now exposes 5-category active-window diagnostics (`click/trail/scroll/hold/hover`) in `/api/state.effects_runtime`, and probe coverage (`/api/effects/test-overlay-windows`) now exercises all 5 categories with persistent-overlay close control
  - effect overlay probe contracts now assert counter arithmetic from raw fields (`before/after total == click + scroll`), not only route-local invariant booleans

## Known Stable Gates
Run these as first-line regression checks:
```bash
./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto
./tools/platform/regression/run-posix-core-smoke.sh --platform auto
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto
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

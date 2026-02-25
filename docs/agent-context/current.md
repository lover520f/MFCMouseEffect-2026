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
  - settings schema now reports `capabilities.effects.scroll=true` on macOS (aligned with runtime scroll-effect mapping)
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
  - macOS WASM selfcheck `load-manifest` request/assert logic is now helper-split (`tools/platform/manual/lib/wasm_selfcheck_common.sh`), reducing script duplication while preserving checks
  - core HTTP contract regression now asserts WASM load-failure diagnostics semantics (`last_load_failure_stage/code`) for success, invalid-manifest failure, and reload-clear paths
  - core HTTP contract regression now also asserts WASM transfer semantics (`import-selected` success+failure and `export-all` success with minimum count guard)
  - core HTTP contract regression now also asserts WASM export filesystem consistency (`export_path` existence and exported directory count == response count)
  - core HTTP contract regression now also asserts WASM export manifest integrity (`plugin.json` count/exists/non-empty under export path)
  - WASM transfer APIs now expose stable `error_code` fields (`import-selected`, `export-all`, folder-dialog import), and regression asserts import failure code semantics
  - shared Svelte WASM UI now surfaces transfer `error_code` in operation feedback, backed by dedicated error-code mapping module
  - transfer failure-code regression matrix now covers `manifest_path_required/not_file/load_failed/not_found`, and WebUI prefers translated error-code mapping before raw backend error text
  - POSIX suite webui semantic phase now also includes `test:wasm-error-model`, keeping WASM error-code mapping behavior gated by default
  - shared Svelte WASM diagnostics panel now surfaces `last_load_failure_stage/code` with EN/ZH i18n labels and warning-state linkage
  - shared Svelte WASM state normalization is now deduplicated via `WebUIWorkspace/src/wasm/state-model.js`, reducing cross-file drift risk
  - core HTTP regression WASM load helpers are now split into `tools/platform/regression/lib/core_http_wasm_helpers.sh`, reducing coupling in `core_http.sh`
  - core HTTP regression now supports scoped checks (`all` default, `wasm` focused) so WASM closure can run a dedicated gate without full automation/input contracts
  - new dedicated WASM-focused gate `tools/platform/regression/run-posix-core-wasm-contract-regression.sh` is available for faster M2 iterations
  - macOS WASM selfcheck now also covers transfer/error-code contracts (`catalog`, folder-dialog probe, import-selected success/failure codes, export-all consistency) in the same one-command flow
  - core HTTP WASM contract execution is now isolated in `tools/platform/regression/lib/core_http_wasm_contract_checks.sh`, and `core_http.sh` now focuses on lifecycle + non-WASM orchestration boundaries
  - core HTTP non-WASM contracts are now isolated in `tools/platform/regression/lib/core_http_input_contract_checks.sh` and `tools/platform/regression/lib/core_http_automation_contract_checks.sh`, further reducing cross-domain coupling in `core_http.sh`
  - core regression entry scripts (`core-smoke`, `core-automation`, `core-wasm`) now share lock-guarded host resource scheduling (`mfx-entry-posix-host`) to avoid concurrent local run interference
  - macOS manual core-entry scripts (`run-macos-core-websettings-manual.sh`, `run-macos-automation-injection-selfcheck.sh`, `run-macos-wasm-runtime-selfcheck.sh`) now share the same host lock guard with regression scripts
  - POSIX suite preflight is now detect-only for `mfx_entry_posix_host`; cleanup stays phase-local under `mfx-entry-posix-host` lock (no suite-level force kill)
  - stale entry-host cleanup now uses one shared helper (`mfx_terminate_stale_entry_host`) across core regression workflows and macOS manual host startup
  - manual entry-lock acquire path is now helperized (`mfx_manual_acquire_entry_host_lock`), and keep-running stop hints are PID-scoped (`kill -TERM <pid>`) to avoid broad process-pattern termination
  - regression file-content checks now use shared `rg`-preferred with `grep` fallback helpers, and entry scripts no longer hard-require `rg`
  - POSIX regression entry scripts now share helperized host-platform detection and `--platform` resolution to keep cross-host guard semantics centralized
  - core regression entry scripts now share helperized workflow preparation and lock execution (`mfx_prepare_core_entry_runtime`, `mfx_run_with_entry_lock`)
  - wasm test-dispatch assertions in regression/manual selfchecks now use bounded retries to reduce transient invoke/render readiness flakiness
  - macOS global-input event mapping and permission-simulation parsing are now extracted to dedicated helper modules (`MacosInputEventUtils.*`, `MacosInputPermissionState.*`)
  - macOS `MacosGlobalInputHook` implementation is now split by responsibility (`MacosGlobalInputHook.mm`, `.EventTap.mm`, `.RunLoop.mm`) to lower file coupling without behavior changes
  - macOS input-indicator overlay path is now split into render/lifecycle (`MacosInputIndicatorOverlay.mm`), probe/event-entry (`MacosInputIndicatorOverlay.Probes.mm`), and shared internals (`MacosInputIndicatorOverlayInternals.*`)
  - macOS keyboard injection path now isolates shortcut key-resolution (`MacosKeyboardInjectorKeyResolver.*`) from event-post execution flow (`MacosKeyboardInjector.mm`)
  - macOS app-catalog scanner now isolates root-scan workflow in `MacosApplicationCatalogScanWorkflow.*` and keeps `MacosApplicationCatalogScanner` as thin entry façade
  - macOS wasm renderer now isolates command dispatch and resolver utilities (`MacosWasmCommandRenderDispatch.*`, `MacosWasmCommandRenderResolvers.*`) from top-level parse/orchestration
  - macOS scroll pulse effect now isolates overlay rendering internals in `MacosScrollPulseOverlayRenderer.*`, keeping `MacosScrollPulseEffect` as lifecycle + entry façade
  - macOS wasm transient image overlay now isolates rendering internals in `MacosWasmImageOverlayRenderer.*`, keeping `MacosWasmTransientOverlay` as API facade
  - macOS dispatch host now isolates lifecycle, messaging worker, and timer management into separate implementation units (`MacosDispatchMessageHost.cpp`, `.Messaging.cpp`, `.Timers.cpp`)
  - macOS tray service now isolates menu/action bridge creation and runtime helpers into `MacosTrayMenuFactory.*` and `MacosTrayRuntimeHelpers.*`, keeping `MacosTrayService` as lifecycle facade
  - macOS app-catalog workflow now isolates scan roots and entry-store logic (`MacosApplicationCatalogScanRoots.*`, `MacosApplicationCatalogEntryStore.*`) with traversal orchestration retained in `MacosApplicationCatalogScanWorkflow.mm`

## Known Stable Gates
Run these as first-line regression checks:
```bash
./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto
./tools/platform/regression/run-posix-core-smoke.sh --platform auto
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto
./tools/platform/regression/run-posix-core-wasm-contract-regression.sh --platform auto
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
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zs-macos-global-input-hook-helper-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zt-macos-global-input-hook-impl-split-eventtap-runloop.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zu-macos-input-indicator-overlay-impl-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zv-macos-keyboard-injector-key-resolver-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zw-macos-application-catalog-scan-workflow-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zx-macos-wasm-command-renderer-dispatch-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zy-macos-scroll-pulse-overlay-renderer-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zz-macos-wasm-image-overlay-renderer-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zza-macos-dispatch-message-host-impl-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzb-macos-tray-service-menu-factory-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzc-macos-app-catalog-workflow-secondary-split.md`
- Phase closure docs:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53ai-automation-mapping-phase-closure.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54i-linux-follow-phase-closure.md`
- For full Phase 55 sequence (`55h-55zzc`), read roadmap status doc above instead of loading all slice docs by default.

## AI-IDE Context Loading Rule
- Read this file first for active truth.
- Read only one targeted phase/issue doc per task.
- Avoid loading bulk historical lists unless needed for specific traceability.

## Update Checklist (per capability change)
1. Update targeted phase/issue doc.
2. If behavior/contract changed, update this file.
3. If navigation changed, update docs README indexes.

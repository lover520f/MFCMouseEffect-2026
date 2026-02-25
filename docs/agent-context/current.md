# Agent Current Context (2026-02-25)

## Scope and Priority
- Primary dev host: macOS.
- Primary target: macOS usable loop first.
- Constraints: no Windows regression; Linux follows compile + contract coverage.

## Current Program State
- Branch baseline has completed Phase 50 -> Phase 55y code slices.
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
  - shared Svelte WASM diagnostics panel now surfaces `last_load_failure_stage/code` with EN/ZH i18n labels and warning-state linkage
  - shared Svelte WASM state normalization is now deduplicated via `WebUIWorkspace/src/wasm/state-model.js`, reducing cross-file drift risk
  - core HTTP regression WASM load helpers are now split into `tools/platform/regression/lib/core_http_wasm_helpers.sh`, reducing coupling in `core_http.sh`

## Known Stable Gates
Run these as first-line regression checks:
```bash
./tools/platform/regression/run-posix-scaffold-regression.sh --platform auto
./tools/platform/regression/run-posix-core-smoke.sh --platform auto
./tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto
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
- Latest WASM crash/throttle context:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55h-macos-wasm-overlay-throttle-guardrail.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55i-macos-wasm-throttle-diagnostics-and-linux-roadmap.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55j-macos-wasm-throttle-cause-breakdown.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55k-macos-wasm-async-task-lifetime-crash-fix.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55q-posix-wasm-load-failure-diagnostics-contract.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55r-macos-wasm-load-failure-classification-selfcheck.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55s-macos-wasm-load-stage-selfcheck-expansion.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55t-macos-wasm-selfcheck-helper-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55u-core-http-wasm-load-failure-contract-assertions.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55v-webui-wasm-load-failure-diagnostics-surface.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55w-webui-wasm-state-model-dedup.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55x-core-http-wasm-helper-module-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55y-core-http-wasm-transfer-contract-assertions.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53ai-automation-mapping-phase-closure.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54i-linux-follow-phase-closure.md`

## AI-IDE Context Loading Rule
- Read this file first for active truth.
- Read only one targeted phase/issue doc per task.
- Avoid loading bulk historical lists unless needed for specific traceability.

## Update Checklist (per capability change)
1. Update targeted phase/issue doc.
2. If behavior/contract changed, update this file.
3. If navigation changed, update docs README indexes.

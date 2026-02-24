# Agent Current Context (2026-02-24)

## Scope and Priority
- Primary dev host: macOS.
- Primary target: macOS usable loop first.
- Constraints: no Windows regression; Linux follows compile + contract coverage.

## Current Program State
- Branch baseline has completed Phase 50 -> Phase 55k code slices.
- POSIX dual-lane guardrail exists:
  - scaffold lane remains default stable lane.
  - core lane is gated and iteratively enabled.
- macOS core lane now includes:
  - global input capture/degraded handling
  - input indicator + click-first visible effect
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
  - unified POSIX suite now includes macOS automation injection selfcheck (`--dry-run`) by default
  - WebSettings test-only routes are now isolated in `WebSettingsServer.TestApiRoutes.*`, reducing main route file coupling
  - WebSettings production WASM routes are now isolated in `WebSettingsServer.WasmRoutes.*`, reducing main route file coupling further
  - WebSettings production automation routes are now isolated in `WebSettingsServer.AutomationRoutes.*`, reducing main route file coupling further
  - WebSettings core settings routes are now isolated in `WebSettingsServer.CoreApiRoutes.*`, and main routing file now focuses on request gate + top-level delegation
  - test-gated `/api/input-indicator/test-mouse-labels` enables non-interactive mac indicator label contract checks (`L/R/M`)
  - `/api/automation/active-process` now guarantees non-empty `process` on macOS via foreground-query fallback chain
  - schema capability `capabilities.input.keyboard_injector` now reports true on macOS (aligned with runtime injector wiring)
  - core automation contract now also guards WebUI shell asset loading (`/settings-shell.svelte.js?token=<token>`) and shell settings-launcher URL call-path (probe/capture files)
  - macOS tray smoke now guards tray `Settings` action callback -> settings launcher URL call-path using test-only auto-trigger and launch-capture probes
  - core automation contract now simulates startup/runtime permission transitions (`trusted=0/1`) and asserts degraded/recovery + startup notify dedup
  - core automation contract now guards app-catalog refresh (`force=true/false`) and selected-app scope state roundtrip
  - unified POSIX suite now runs macOS WASM runtime selfcheck by default (with explicit skip flag), reducing manual acceptance drift

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

## AI-IDE Context Loading Rule
- Read this file first for active truth.
- Read only one targeted phase/issue doc per task.
- Avoid loading bulk historical lists unless needed for specific traceability.

## Update Checklist (per capability change)
1. Update targeted phase/issue doc.
2. If behavior/contract changed, update this file.
3. If navigation changed, update docs README indexes.

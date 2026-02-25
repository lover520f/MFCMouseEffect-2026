# MFCMouseEffect Documentation

Language: [English](README.md) | [中文](README.zh-CN.md)

## Why This Index
This file is intentionally compact for AI-IDE and human quick navigation.
Use it as a pointer index, not a full historical catalog.

## Read Order (Agent-First)
1. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/AGENTS.md`
2. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/current.md`
3. `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
4. One targeted issue/refactoring doc for current task

## High-Priority Docs
- Active context: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/current.md`
- Roadmap snapshot: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
- Doc governance: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/agent-doc-governance.md`
- POSIX regression suite workflow: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-regression-suite-workflow.md`
- POSIX scaffold workflow: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-scaffold-regression-workflow.md`
- POSIX core smoke workflow: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-lane-smoke-workflow.md`
- POSIX core automation contract workflow: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-automation-contract-workflow.md`
- POSIX Linux compile gate workflow: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-linux-compile-gate-workflow.md`

## Current macOS Mainline (Phases 50-55)
- Dual-lane guardrails: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase50-posix-core-runtime-dual-lane-guardrails.md`
- Core decoupling: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase51-core-win32-decoupling-and-posix-path-foundation.md`
- Permission and input convergence (52x):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52f-macos-runtime-permission-revocation-hardening.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52j-macos-startup-missing-permission-retry-and-notify-dedup.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52k-macos-permission-and-indicator-contract-automation.md`
- Automation and app-scope (53x):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53a-macos-automation-system-services-bootstrap.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53e-macos-automation-app-catalog-scan.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53c-posix-core-settings-webui-rewire.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53d-macos-shortcut-capture-keycode-normalization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53b-automation-appscope-cross-platform-normalization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53f-macos-automation-scope-ui-platform-semantics.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53g-appscope-match-test-endpoint-and-contract-closure.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53h-automation-binding-priority-contract-closure.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53i-macos-automation-injection-selfcheck-and-match-inject-contract.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53j-websettings-test-routes-module-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53k-websettings-wasm-routes-module-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53l-websettings-automation-routes-module-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53m-websettings-core-api-routes-module-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53n-websettings-request-gateway-module-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53o-websettings-webui-path-resolver-module-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53p-websettings-wasm-route-runtime-catalog-utils-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53q-websettings-test-routes-deep-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53r-settings-state-mapper-sections-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53s-settings-schema-builder-sections-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53t-http-server-lifecycle-session-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53u-websettings-test-automation-routes-layer-split.md`
- Linux follow gates (54x):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54c-posix-regression-suite-orchestrator.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54f-core-automation-http-contract-regression.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54g-macos-automation-injection-selfcheck-suite-gate.md`
- WASM runtime and diagnostics (55x):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55h-macos-wasm-overlay-throttle-guardrail.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55j-macos-wasm-throttle-cause-breakdown.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55k-macos-wasm-async-task-lifetime-crash-fix.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55l-posix-wasm-catalog-transfer-apis.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55m-macos-native-folder-picker-wasm-import-dialog.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55n-wasm-dispatch-test-contract-automation.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55o-macos-manual-websettings-runner.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55o-b-macos-wasm-runtime-selfcheck-script.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55p-macos-wasm-selfcheck-suite-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55o-c-macos-folder-picker-implementation-split.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55o-macos-wasm-closure.md`

## Targeted Retrieval Commands
```bash
# List latest refactoring docs
rg --files docs/refactoring | sort | tail -n 30

# Search by capability keyword
rg -n "permission|automation|app_scope|wasm|throttle" docs/refactoring docs/issues

# Token hygiene check (weekly)
./tools/docs/doc-hygiene-check.sh --strict

# macOS core manual runner (URL/token auto-resolved)
./tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60

# macOS wasm runtime selfcheck (load/invoke/render/fallback)
./tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build

# macOS automation injection selfcheck (real dispatch by default; add --dry-run for deterministic mode)
./tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build
```

## Archive
- Archive root: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/archive/README.md`
- For complete historical coverage, use targeted `rg` queries instead of expanding this top-level index.

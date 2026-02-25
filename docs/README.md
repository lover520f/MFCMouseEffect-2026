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
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52l-macos-scroll-baseline-effect.md`
- Automation and app-scope (53x):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53a-macos-automation-system-services-bootstrap.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53b-automation-appscope-cross-platform-normalization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53d-macos-shortcut-capture-keycode-normalization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53f-macos-automation-scope-ui-platform-semantics.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53i-macos-automation-injection-selfcheck-and-match-inject-contract.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53ai-automation-mapping-phase-closure.md`
  - Full split-chain detail (`53j-53ah`) is summarized in `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
- Linux follow gates (54x):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54c-posix-regression-suite-orchestrator.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54f-core-automation-http-contract-regression.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54g-macos-automation-injection-selfcheck-suite-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54h-linux-dual-lane-compile-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54i-linux-follow-phase-closure.md`
- WASM runtime and diagnostics (55x):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55o-macos-wasm-closure.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55p-macos-wasm-selfcheck-suite-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55q-posix-wasm-load-failure-diagnostics-contract.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zd-wasm-transfer-error-code-regression-matrix-and-i18n.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55ze-webui-wasm-error-model-test-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zf-wasm-focused-contract-gate-and-selfcheck-expansion.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zo-posix-platform-arg-helper-consolidation.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zp-doc-index-compaction-p0-p1.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zq-core-regression-workflow-helper-consolidation.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zr-wasm-dispatch-readiness-retry-hardening.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzs-wasm-manifest-path-trim-contract-gate.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzt-wasm-selfcheck-helper-modularization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzu-core-http-wasm-helper-modularization-and-lock-race-hardening.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzv-core-http-wasm-contract-check-modularization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzw-core-http-automation-contract-check-modularization.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzx-core-http-orchestrator-helper-split.md`
  - Full hardening sequence (`55h-55zzx`) is summarized in `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`

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

# macOS core wasm-focused HTTP contract gate (faster than full automation contract run)
./tools/platform/regression/run-posix-core-wasm-contract-regression.sh --platform auto

# macOS core wasm path-trim contract gate (fast route path-semantics check)
./tools/platform/regression/run-posix-core-wasm-path-contract-regression.sh --platform auto

# macOS automation injection selfcheck (real dispatch by default; add --dry-run for deterministic mode)
./tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build
```

## Archive
- Archive root: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/archive/README.md`
- For complete historical coverage, use targeted `rg` queries instead of expanding this top-level index.

# macOS Mainline Roadmap Snapshot (2026-03-01)

## Purpose
Single high-value status page for AI-IDE and reviewers.
This file intentionally excludes low-value historical step logs.

## Scope Baseline
- Primary host and priority: macOS.
- Hard constraints:
  - Windows behavior must not regress.
  - Linux follows compile + contract checks.
  - macOS new work is Swift-first; no Objective-C++ surface expansion.

## Runtime Lanes
- `scaffold` lane: default stable path.
- `core` lane (`mfx_entry_posix_host`): progressive parity path.
- Policy: ship new cross-platform capability through core lane while preserving scaffold stability.

## Phase Status (Plan vs Actual)
- Phase 50: done.
  - Dual-lane guardrails in place.
- Phase 51: done.
  - Core decoupled from Win32-only assumptions for POSIX compile/start baseline.
- Phase 52: done.
  - macOS input capture + permission degrade/recover + indicator baseline complete.
- Phase 53: done (M1 automation scope).
  - App-scope normalization, mapping/injection path, and test-route contracts complete.
- Phase 54: done (Linux follow scope).
  - Compile-level + contract-level gates complete.
- Phase 55: done (WASM runtime baseline in current scope).
  - macOS wasm runtime path, diagnostics, and selfcheck gates complete.
- Phase 56+: ongoing.
  - Behavior parity hardening and token-efficient docs governance.

## Current Capability State (macOS)
- Effects:
  - click/trail/scroll/hold/hover paths available in core lane.
  - Type normalization and command-based profile flow active.
  - `trail=none` hard-disable and anti-origin-connector guards active.
- Automation:
  - `process:code` / `code.app` / `code.exe` scope semantics normalized.
  - Injection and matcher contracts script-gated.
- WASM:
  - load/invoke/render/fallback path active.
  - schema vs state capability parity is contract-gated.
- Permissions:
  - runtime revoke -> degrade + notify.
  - runtime regrant -> hot recovery without restart.

## Regression Gates (Canonical)
- Scaffold regression:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- Effects contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
- Automation contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- WASM suite:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-wasm-regression-suite.sh --platform auto`
- macOS ObjC++ surface gate:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-macos-objcxx-surface-regression.sh`

## Manual Selfcheck Entrypoints (macOS)
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/mfx`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh --skip-build`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build`

## Active Supporting Docs (Small Set)
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52e-macos-coordinate-space-unification.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52f-macos-runtime-permission-revocation-hardening.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52g-macos-input-indicator-label-lifetime-fix.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52h-macos-startup-permission-degraded-convergence.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52i-macos-runtime-permission-hot-recovery-and-notify.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase52j-macos-startup-missing-permission-retry-and-notify-dedup.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase53ai-automation-mapping-phase-closure.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase54i-linux-follow-phase-closure.md`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbm-macos-user-notification-native-center.md`

## Completion Criteria for Current Milestone
- macOS behavior parity acceptable for main user-facing paths (effects/automation/wasm/permissions).
- Windows no-regression verified by existing gates.
- Linux compile + contract follow remains green.
- No uncontrolled growth in P0/P1 docs.

## Notes
- Historical granular phase logs were intentionally removed from this file to keep first-read token usage low.
- If deep history is required, use git history on this file and targeted surviving phase docs.

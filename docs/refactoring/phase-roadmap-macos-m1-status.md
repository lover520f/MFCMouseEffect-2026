# macOS Mainline Roadmap Snapshot (2026-03-02)

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
  - Effects contract now hard-checks trail mode runtime semantics on macOS overlay probe:
    - `trail=line` must produce observable trail overlay window increase.
    - `trail=none` must not produce trail overlay window increase.
  - Effects contract now hard-checks trail mode command semantics in render profile probe:
    - `trail=line` => command `normalized_type=line` and `emit=true`.
    - `trail=none` => command `normalized_type=none` and `emit=false`.
  - Effects contract now hard-checks command geometry diagnostics:
    - click text sample geometry (`font_size/float_distance`) must be positive.
    - for normalized `line` trail, command line-width must match profile line-width.
  - Effects contract now hard-checks command parity for scroll/hover/hold canonical modes:
    - `scroll=helix` => command emits with helix mode on.
    - `hover=tubes` => command resolves tubes mode on.
    - `hold=hologram` => hold start/update command remains active with normalized hologram semantics.
  - Effects contract now hard-checks alias and fallback command semantics:
    - legacy aliases (`textclick/scifi/stardust/scifi3d/suspension`) map to expected command normalized types and mode flags.
    - `none` fallbacks for click/scroll/hover map to (`ripple/arrow/glow`) with conflicting mode flags disabled.
  - Effects contract now hard-checks hold follow-mode update semantics:
    - smooth mode sample must output deterministic smoothed overlay coordinates on second update.
    - efficient mode sample must satisfy emit throttling cadence (first emit, short-interval suppress, long-interval resume).
  - Effects contract assertion script has been deduplicated to shared helper-based checks (active+type, mode flags, nested command fields) with no behavior contract change.
- Automation:
  - `process:code` / `code.app` / `code.exe` scope semantics normalized.
  - Non-Windows scope persistence now canonicalizes suffix variants to `process:<base>` and enforces legacy `app_scope == app_scopes[0]` parity in automation contract checks.
  - Binding-priority contract now verifies platform-aware normalized selected scope and alias-tie deterministic winner behavior.
  - macOS manual app-scope selfcheck now includes alias-dedupe persistence assertions.
  - App-scope JSON parse helpers are shared across regression/manual scripts for single-source contract parsing behavior.
  - Nested contract scalar parsing moved from regex extraction to JSON path parsing with compatible fallback, reducing false positives under expanding payloads.
  - Injection and matcher contracts script-gated.
- WASM:
  - load/invoke/render/fallback path active.
  - schema vs state capability parity is contract-gated.
  - wasm platform checks now also lock runtime backend expectation (`wasm3_static`) and schema/state invoke-render capability parity in wasm-only contract scope.
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
- Manual core-host startup now validates `/api/state` readiness and fails fast when a scaffold-lane binary is accidentally reused via `--skip-build`.

## Active Supporting Docs (Small Set)
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase55zzzzbm-macos-user-notification-native-center.md`

## Completion Criteria for Current Milestone
- macOS behavior parity acceptable for main user-facing paths (effects/automation/wasm/permissions).
- Windows no-regression verified by existing gates.
- Linux compile + contract follow remains green.
- No uncontrolled growth in P0/P1 docs.

## Notes
- Historical granular phase logs were intentionally removed from this file to keep first-read token usage low.
- Phase52-54 closure details are retained in git history and regression scripts; separate phase documents were removed to reduce stale-doc maintenance.
- If deep history is required, use git history on this file and prior commits.

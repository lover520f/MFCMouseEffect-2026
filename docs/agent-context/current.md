# Agent Current Context (2026-03-01)

## Scope and Priority
- Primary host: macOS.
- Primary objective: macOS mainline reaches stable Windows-equivalent behavior (behavior parity first, pixel parity later).
- Hard constraints:
  - No Windows regressions.
  - Linux follows compile + contract coverage.
  - New macOS features prefer Swift-first; do not expand `.mm` surface.

## Runtime Lanes
- Default stable lane: `scaffold`.
- Progressive lane: `core` (`mfx_entry_posix_host`).
- Policy: keep scaffold behavior stable while landing core-lane parity increments.

## Current Capability Status
- Effects:
  - macOS supports click/trail/scroll/hold/hover in core lane.
  - Shared compute-command model is active; renderer path is execution-focused.
  - Trail `none` hard-disable, line-trail diagnostics, and anti-origin-connector guards are in place.
  - Effects profile probe now exposes metadata-derived `catalog_values`; selfcheck asserts full five-category option coverage.
- Automation:
  - App-scope alias normalization (`process:code` / `code.app` / `code.exe`) is gated.
  - Injection selfcheck and app-scope selfcheck are part of POSIX suite phases.
- WASM:
  - macOS runtime path + diagnostics + fallback contracts are active.
  - Capability schema/state parity is gated.
  - Shared config resolvers (text/image/runtime value) are single-source.
- Input indicator / permissions:
  - Permission degrade + hot recovery behavior is implemented and script-gated.
  - VM suppression diagnostics/state exposure is present.
- Shell UX:
  - macOS tray now prefers icon rendering (project logo path fallback + SF Symbol fallback) instead of text-only `MFX`.
  - macOS warning notifications now initialize app icon before delivery to avoid generic default sender icon in unbundled runs.

## Build and Regression Gates
- macOS shell CMake Swift bridge registration has been normalized to one helper (`mfx_add_swift_bridge`) plus one source list (`MFX_MACOS_SWIFT_OBJECTS`), removing 20 duplicated compile blocks and reducing bridge drift risk.
- POSIX effects suite (core automation scope fixed to `effects`, now includes macOS WASM selfcheck by default):
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-effects-regression-suite.sh --platform auto`
- POSIX scaffold regression:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- POSIX core effects contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
- POSIX core automation contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- POSIX wasm suite:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-wasm-regression-suite.sh --platform auto`
- macOS ObjC++ surface gate:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-macos-objcxx-surface-regression.sh`
  - Gate now checks both macOS CMake rules and any existing `/tmp/mfx-platform-macos*/compile_commands.json` for hidden Objective-C++ flags.
- Core HTTP regression teardown now attempts `/api/stop` before TERM/KILL fallback to reduce forced-stop noise and flakiness.

## High-Value Manual Entrypoints (macOS)
- One-command launcher:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/mfx`
  - primary commands: `run`, `run-no-build`, `effects` (legacy aliases `start`, `fast` remain supported).
  - supports `--seconds` for quick auto-stop runs (e.g. `./mfx run-no-build --seconds 30`).
  - regression shortcuts: `verify-effects` (daily gate) and `verify-full` (full suite).
- Core web settings manual run:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60`
- Effects parity selfcheck:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh --skip-build`
- Automation injection selfcheck:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build`
- WASM runtime selfcheck:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build`
- Manual host helper teardown now prefers `/api/stop` and falls back to TERM/KILL (same policy used by auto-stop scheduler).

## Key Contracts (Do Not Break)
- `settings schema` vs `runtime diagnostics` capability parity.
- `effects active type` normalization consistency across core/runtime/web state.
- `trail=none` must remain no-render residual.
- Permission revoke/regrant must not require process restart.
- Windows behavior semantics must remain unchanged when touching shared/core paths.

## Docs Governance State
- Active structure:
  - `P0`: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/AGENTS.md`
  - `P1`: this file + `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
  - `P2`: targeted docs under `docs/architecture`, `docs/issues`, `docs/refactoring`
  - `P3`: `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/archive/README.md`
- Cleanup baseline (2026-03-01):
  - non-referenced historical docs in issues/refactoring/architecture/root were removed
  - first-read docs were compacted for low-token navigation
  - WASM architecture/operator docs were rewritten as contract/checklist format
  - phase52-phase54 standalone refactoring notes were merged back into roadmap + git history and removed from active docs
  - wasm phase3/4 issue-slice documents were removed from active docs; long-form chronology stays in git history
- Current inventory (after cleanup):
  - `docs/architecture`: 16
  - `docs/refactoring`: 2
  - `docs/issues`: 1
  - `docs` total markdown: 23

## Where to Read History
- For full chronological details, use:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
  - targeted docs under `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/`

## Next Focus
- Continue macOS behavior parity hardening for effects and WASM observable contracts.
- Keep cleanup incremental: delete only unreferenced historical docs, then run link integrity checks.

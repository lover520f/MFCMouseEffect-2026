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

## Build and Regression Gates
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

## High-Value Manual Entrypoints (macOS)
- One-command launcher:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/mfx`
- Core web settings manual run:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60`
- Effects parity selfcheck:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh --skip-build`
- Automation injection selfcheck:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build`
- WASM runtime selfcheck:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build`

## Key Contracts (Do Not Break)
- `settings schema` vs `runtime diagnostics` capability parity.
- `effects active type` normalization consistency across core/runtime/web state.
- `trail=none` must remain no-render residual.
- Permission revoke/regrant must not require process restart.
- Windows behavior semantics must remain unchanged when touching shared/core paths.

## Docs Governance State
- Top-level docs were compacted to reduce token load:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/README.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/README.zh-CN.md`
- Large non-indexed historical issue bundles were removed.
- Remaining unreferenced issue docs were pruned; `docs/issues` now keeps only actively referenced entries.
- Unreferenced refactoring history docs were also pruned by reachability; `docs/refactoring` keeps the linked active subset plus roadmap snapshot.
- Unreferenced architecture docs (`dawn-native-effects-route`, `mousefx-folder-structure*`, `trail-effects-differentiation*`, `web-settings-ui*`) were pruned; architecture layer now keeps only linked workflow/contracts docs.
- Unreferenced root-level docs (`input_indicator_refactor`, `installer_guide`, `macos-text-click-effect-rendering-fix`, `multi-monitor-positioning`, `singleton_implementation`) were removed as historical records with outdated paths/contracts.
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md` was rewritten into a compact execution snapshot (removed long historical step logs) to keep first-read token cost low.
- Removed unreferenced non-engineering docs in `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/install` and `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/marketing`; retained `trail-profiles-config*` as an active configuration contract and re-linked it in top-level indexes.
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/custom-effects-wasm-route.md` and `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/custom-effects-wasm-route.zh-CN.md` were compacted into architecture-contract summaries; low-value phase narrative was removed in favor of targeted issue docs.
- Current docs focus is now:
  - `P0`: `AGENTS.md`
  - `P1`: this file + roadmap snapshot
  - `P2`: targeted capability docs
  - `P3`: archive

## Where to Read History
- For full chronological details, use:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
  - targeted docs under `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/`

## Next Focus
- Continue macOS behavior parity hardening for effects and WASM observable contracts.
- Keep cleanup incremental: delete only unreferenced historical docs, then run link integrity checks.

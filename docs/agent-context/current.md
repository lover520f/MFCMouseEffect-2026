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
  - Core effects contract now asserts `trail=none` consistency across `POST /api/state` -> `GET /api/state` -> `/api/effects/test-render-profiles` active snapshot, preventing silent fallback to `line`.
  - Core effects contract now asserts legacy alias normalization through runtime state and render-profile snapshot (`textclick/scifi/stardust/scifi3d/suspension + cursor_priority -> text/tubes/twinkle/hologram/tubes + smooth`).
  - Line-trail runtime diagnostics now include `line_trail_line_width_px` so contract gates can catch thin-line regressions.
  - Effects profile probe now exposes metadata-derived `catalog_values`; selfcheck asserts full five-category option coverage.
- Automation:
  - App-scope alias normalization (`process:code` / `code.app` / `code.exe`) is gated.
  - App-scope persistence dedupe is contract-gated: writing equivalent aliases (`process:code.exe/code.app/code`) must collapse to one persisted scope token.
  - Injection selfcheck and app-scope selfcheck are part of POSIX suite phases.
- WASM:
  - macOS runtime path + diagnostics + fallback contracts are active.
  - Capability schema/state parity is gated.
  - Core state gate now asserts wasm runtime backend consistency (`dynamic_bridge|wasm3_static|null|external`) and enforces `runtime_backend=null => runtime_fallback_reason` non-empty.
  - Core state gate now hard-checks schema wasm platform matrix (`macOS: invoke/render=true`, `Linux: invoke/render=false`) before parity checks.
  - Shared config resolvers (text/image/runtime value) are single-source.
- Input indicator / permissions:
  - Permission degrade + hot recovery behavior is implemented and script-gated.
  - VM suppression diagnostics/state exposure is present.
- Shell UX:
  - macOS tray now prefers icon rendering (project logo path fallback + SF Symbol fallback) instead of text-only `MFX`.
  - macOS warning notifications now initialize app icon before delivery to avoid generic default sender icon in unbundled runs.
  - icon resolution supports explicit override via `MFX_MACOS_APP_ICON_PATH` (highest priority), then bundle/dev fallback paths.
  - tray smoke gate now uses explicit CLI args (`--expect-settings-action`, `--settings-url`, `--launch-capture-file`); when sandbox runners do not emit launch-capture files, regression falls back to exit-code gating to avoid false negatives.

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
- Scaffold HTTP regression now logs startup failure with `stage/code`; `bind EPERM/EACCES (stage=2, code=1/13)` is treated as a controlled skip in sandbox-like runners (override with `MFX_HTTP_SKIP_BIND_EACCES=0`).
- Scaffold HTTP startup helper now also treats "early exit without startup log" as constrained-runtime skip, and `http.sh` consumes helper return code `2` across default/custom/missing-webui routes so skip paths no longer break under `set -e`.
- Scaffold/core HTTP gates now support strict non-skip mode (`MFX_HTTP_REQUIRE_EXECUTION=1`, `MFX_CORE_HTTP_REQUIRE_EXECUTION=1`) to force real execution in full-capability environments.
- Core HTTP startup helper now reports bind permission failures as `EPERM/EACCES (stage=2, code=1/13)` and supports the same constrained-runtime skip intent via `MFX_CORE_HTTP_ALLOW_BIND_EACCES_SKIP`.
- Core automation contract regression now defaults `MFX_CORE_HTTP_ALLOW_BIND_EACCES_SKIP=1`, aligning with effects/wasm contracts and avoiding noisy false-negative startup failures under constrained runners.
- Core HTTP startup now short-circuits on the first confirmed bind-permission denial when skip mode is enabled, reducing duplicate retry noise in constrained runners.
- Regression entry helpers (`scaffold/core-http/core-smoke`) now allocate FIFO through private temp directories (`mktemp -d` + `mkfifo`) instead of `mktemp -u`, removing TOCTOU race windows in concurrent runs.
- macOS manual selfcheck host helper now returns controlled skip (`MFX_MANUAL_ALLOW_BIND_EACCES_SKIP=1`) for constrained-runtime startup failures (`websettings_start_failed(stage=2,code=1/13)` and no-probe/no-log early exit), avoiding noisy false-negative `host exited early`; POSIX suite enables this skip policy by default for manual selfcheck phases.
- macOS manual selfchecks now support strict non-skip mode (`MFX_MANUAL_REQUIRE_EXECUTION=1`), failing immediately if constrained-runtime startup would otherwise be treated as skip.
- Startup options now support explicit single-instance key override (`--single-instance-key=...` / `MFX_SINGLE_INSTANCE_KEY`); manual selfchecks can auto-isolate lock key under constrained-runtime skip mode to avoid stale-lock false negatives.

## High-Value Manual Entrypoints (macOS)
- One-command launcher:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/mfx`
  - primary commands: `run`, `run-no-build`, `effects` (legacy aliases `start`, `fast` remain supported).
  - supports `--seconds` for quick auto-stop runs (e.g. `./mfx run-no-build --seconds 30`).
  - regression shortcuts: `verify-effects` (daily gate) and `verify-full` (full suite).
  - strict mode: `effects --strict`, `verify-effects --strict`, `verify-full --strict` force fail on constrained-runtime skip paths.
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
- `capabilities.effects/input/wasm` platform matrix must remain schema-consistent (mac/linux contract checks in core state gate).
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

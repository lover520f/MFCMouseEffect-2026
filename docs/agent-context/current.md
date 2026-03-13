# Agent Current Context (P1, 2026-03-12)

## Purpose
This file is the compact P1 runtime truth for daily execution.  
Deep implementation details are intentionally moved to P2 docs to reduce context waste.

## Scope and Platform Priority
- Primary host: macOS.
- Delivery priority: macOS mainline first, Windows regression-free, Linux compile/contract-level.
- macOS stack rule:
  - New capability modules are Swift-first.
  - Existing `.mm` surface is maintenance/refactor only; avoid expanding `.mm` scope.

## Runtime Lanes
- Stable lane: `scaffold`
- Progressive lane: `core` (`mfx_entry_posix_host`)
- Policy: new cross-platform capability lands in `core` first, while `scaffold` stays stable.

## Active Product Goals
- Goal A: wasm runtime remains bounded-but-expressive (not raw shader ownership), while improving parity and testability.
- Goal B: input indicator and effect plugins coexist safely by lane/surface separation.
- Goal C: automation gesture mapping remains accurate, observable, and low-regression across macOS/Windows.

## Current Capability Snapshot

### Visual Effects / WASM
- `click/trail/scroll/hold/hover` are active in `core`.
- Shared command tail (`blend_mode/sort_key/group_id`) is active.
- Group retained model is active (transform/material/pass remain host-owned bounded surfaces).
- Compatibility boundary remains:
  - wasm can express rich 2D composition.
  - wasm does not own raw GPU pipeline/shader graph.

### Input Indicator
- macOS and Windows indicator labels/streak semantics are aligned (`L xN`, `W+ xN`).
- Indicator wasm dispatch uses dedicated indicator lanes and safer budget floor.
- Indicator plugin routing exposes explicit route snapshot for diagnostics.

### Automation Mapping
- App-scope normalization/parser contracts are stable.
- Gesture mapping supports preset/custom with similarity threshold.
- Custom gesture editor now uses explicit `Draw -> Save` workflow:
  - Drawing is locked until `Draw` is clicked.
  - New strokes persist only on `Save` (saved view is normalized/read-only).
  - Re-edit always requires entering `Draw` mode again.
- Trigger button supports `none` (gesture-only) end-to-end.
- Gesture matching robustness improvements are active (motion-intent arm/high-confidence commit, aspect-ratio normalization, structure-aware scoring).
- Matching candidate generation is now hybrid:
  - time-window sliding for local intent extraction
  - spatial-window sliding + normalized shape scoring for speed-invariant comparison
- Preset/custom runtime matching now shares one window-aware engine:
  - `best_score / runner_up_score / best_window / candidate_count` emitted by matcher
  - preset + custom both use ambiguity rejection (`best-runner_up >= margin`)
  - custom defaults to stroke-count and stroke-order sensitive matching (no new UI toggle in this phase)
- Route-time candidate selection now has a second-stage specificity policy:
  - when scores are close, larger templates with more turns/segments and better window coverage can outrank simpler contained sub-shapes
  - runner-up ambiguity no longer counts same-template internal windows, only competing candidates
- Core automation HTTP contract now contains gesture-similarity sample suite (`windowed hit / ambiguous reject / custom order / min-length gate`), preventing silent matcher regressions.
- Gesture calibration now has explicit test-friendly overrides with production-safe defaults:
  - `MFX_GESTURE_AMBIGUITY_MARGIN`
  - `MFX_GESTURE_CUSTOM_MIN_EFFECTIVE_STROKE_PX`
  - test API window geometry options (`window_coverage_*`, `window_slide_divisor`)
- Gesture structure extraction now keeps strong turning anchors (`DP + protected-turn retention`) to prevent multi-turn presets from being flattened.
- Current calibration baseline is `6/6` sample pass and no longer shows preset `W` as a failing lane.
- Debug observability now has two layers:
  - `/api/state` snapshot keeps `last_*` summary.
  - New `recent_events` ring buffer (`seq/timestamp/stage/reason/...`) keeps latest route trace history, including window/candidate telemetry.
  - gesture route diagnostics now split `recognized` vs `matched` ids end-to-end, so debug UI can show “当前识别到的形状” and “最终命中的模板” separately instead of overloading one field.
- Buttonless gesture route now has a noisy-motion prefilter (`buttonless_noisy_motion_filtered`) so chaotic/high-speed scribble movement does not enter mapping trigger routing.

## Debug and Observability Contract
- Runtime gesture-route diagnostics are gated by debug mode:
  - `./mfx start --debug`
  - or `MFX_RUNTIME_DEBUG=1`
- Default non-debug runs do not emit this lane to avoid overhead.
- WebUI debug rendering is decoupled and mounted in sidebar debug card; runtime state is synced via workspace runtime channel.
- Sidebar debug gesture preview now uses inline SVG stroke/fill attributes (no CSS-only dependency), and the displayed gesture id prefers the latest non-empty `recent_events.gesture_id` to avoid being overwritten by subsequent blank move/skip events.
- Sidebar debug card now prefers explicit fields:
  - `last_recognized_gesture_id` / `recent_events[].recognized_gesture_id`
  - `last_matched_gesture_id` / `recent_events[].matched_gesture_id`
  - `last_matched_gesture_id` is now strict mapping-hit only; no-hit route keeps it empty to avoid stale/false positive "matched gesture" display.
  - old `last_gesture_id` stays as compatibility fallback only.
- Debug-only realtime pull is enabled only when:
  - runtime diagnostics payload exists (debug mode),
  - connection is online,
  - focused section is `automation`.
- Polling cadence is `~200ms`, and only syncs sidebar runtime debug state (no full page rerender).

## Server Structure Note
- `MouseFx/Server` has started physical sub-layer split to match SRP rules:
  - `core/`: web settings server lifecycle + request routing/token monitor entry
  - `routes/automation/`: automation + test-automation route handlers
  - `routes/core/`: core API route + request gateway
  - `routes/testing/`: test-only route handlers
  - `routes/wasm/`: wasm catalog/runtime/import/export route handlers and wasm route utils
  - `diagnostics/`: settings diagnostics mapping builders
  - `settings/`: settings schema/state/wasm capabilities mapping
  - `http/`: embedded http server implementation
  - `webui/`: webui assets + webui path resolver
- Compatibility wrapper headers under `MouseFx/Server` have been removed; includes now point to concrete sub-layer paths directly.
- POSIX shell include boundary tightened: `PosixCoreAppShell.h` now forward-declares `mousefx::AppController` / `mousefx::WebSettingsServer`; concrete headers are included in `.cpp`/action units.
- Scaffold POSIX headers also trimmed:
  - `ScaffoldSettingsApi.h` no longer drags `HttpServer.h` / route config include transitively.
  - `ScaffoldSettingsRequestHandler.h` no longer drags `HttpServer.h` transitively.
  - `ScaffoldSettingsRuntime.Internal.h` now uses forward declarations (`HttpServer`, `SettingsRequestHandler`) instead of full includes.

## Regression Gates (High Frequency)
- Scaffold:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- Core effects:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
- Core automation:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- Core wasm:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-wasm-contract-regression.sh --platform auto`
- Full wasm suite:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-wasm-regression-suite.sh --platform auto`
- macOS ObjC++ surface guard:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-macos-objcxx-surface-regression.sh`

## High-Value Manual Commands
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/mfx run`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/mfx start --debug`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh --skip-build`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-gesture-calibration-sweep.sh --skip-build`

## Contracts That Must Not Drift
- Keep stdin JSON command compatibility.
- Keep wasm ABI backward compatibility inside current major ABI contract unless explicit migration approved.
- Keep host-owned bounded pass/material strategy; do not add raw shader controls without architecture approval.
- Keep docs synchronized in the same change set when behavior/contracts change.

## P2 Detail Routing
Read these only when task keywords match:
- P2 index:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/agent-context/p2-capability-index.md`
- Automation matching and thresholds:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/automation/gesture-matching.md`
- WASM route and ABI:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/custom-effects-wasm-route.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-abi-v3-design.md`
- Workflow contracts:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-regression-suite-workflow.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/posix-core-automation-contract-workflow.md`

## Documentation Governance State
- `current.md` is P1 only (compact execution truth).
- P2 details are indexed and routed by:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/docs/ai-context.sh route --task "<keywords>"`
- Context artifacts:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/.ai/context-index.json`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/.ai/context-map.md`
- Realtime index refresh options:
  - `./tools/docs/ai-context.sh watch`
  - `./tools/docs/install-git-hook.sh`

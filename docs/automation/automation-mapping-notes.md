# Automation Mapping Notes (P2)

## Scope
Detailed behavior notes for automation mapping, gesture recognition, and calibration.
P1 (`docs/agent-context/current.md`) keeps a concise summary only.

## Details
- App-scope normalization/parser contracts are stable.
- Gesture mapping supports preset/custom with similarity threshold.
- Custom gesture editor uses explicit `Draw -> Save` workflow:
  - Drawing is locked until `Draw` is clicked.
  - New strokes persist only on `Save` (saved view is normalized/read-only).
  - Re-edit always requires entering `Draw` mode again.
- Trigger button supports `none` (gesture-only) end-to-end.
- Gesture matching robustness improvements are active (motion-intent arm/high-confidence commit, aspect-ratio normalization, structure-aware scoring).
- Gesture recognizer direction extraction runs `collect -> simplify -> cap` (instead of early cap), and applies diagonal-bridge jitter cleanup. This improves `W`/multi-turn recognition where early noise previously truncated final intent segments.
- Preset routing includes generic sub-shape hijack suppression (complex recognized chain vs low-coverage simple candidate), reducing “复杂手势被简单子形状抢命中” across a class of patterns.
- Matching candidate generation is hybrid:
  - time-window sliding for local intent extraction
  - spatial-window sliding + normalized shape scoring for speed-invariant comparison
- Preset/custom runtime matching shares one window-aware engine:
  - `best_score / runner_up_score / best_window / candidate_count` emitted by matcher
  - preset + custom both use ambiguity rejection (`best-runner_up >= margin`)
  - custom defaults to stroke-count and stroke-order sensitive matching (no new UI toggle in this phase)
- Route-time candidate selection has a second-stage specificity policy:
  - when scores are close, larger templates with more turns/segments and better window coverage can outrank simpler contained sub-shapes
  - runner-up ambiguity no longer counts same-template internal windows, only competing candidates
- Core automation HTTP contract contains gesture-similarity sample suite (`windowed hit / ambiguous reject / custom order / min-length gate`), preventing silent matcher regressions.
- Gesture calibration has explicit test-friendly overrides with production-safe defaults:
  - `MFX_GESTURE_AMBIGUITY_MARGIN`
  - `MFX_GESTURE_CUSTOM_MIN_EFFECTIVE_STROKE_PX`
  - test API window geometry options (`window_coverage_*`, `window_slide_divisor`)
- Gesture structure extraction keeps strong turning anchors (`DP + protected-turn retention`) to prevent multi-turn presets from being flattened.
- Current calibration baseline is `6/6` sample pass and no longer shows preset `W` as a failing lane.
- Debug observability has two layers:
  - `/api/state` snapshot keeps `last_*` summary.
  - `recent_events` ring buffer (`seq/timestamp/stage/reason/...`) keeps latest route trace history, including window/candidate telemetry.
  - gesture route diagnostics split `recognized` vs `matched` ids end-to-end, so debug UI can show “当前识别到的形状” and “最终命中的模板” separately instead of overloading one field.
- Buttonless gesture route has a noisy-motion prefilter (`buttonless_noisy_motion_filtered`) so chaotic/high-speed scribble movement does not enter mapping trigger routing.
- Buttonless route realtime evaluation keeps re-evaluating the same recognized id while stroke grows (no same-id early return), improving late-confidence hit opportunity.
- Pressed-button drag gesture route has noisy-motion suppression (`pressed_noisy_motion_filtered`) with stricter thresholds than buttonless, improving stability on fast chaotic drags while preserving normal `V/W` paths.
- Preset matcher supports dual-direction variants for zig-zag/diagonal families (reverse + mirror + mirror-reverse), reducing direction-habit mismatch in one-way preset shapes.
- Matcher includes time-resample fallback on top of distance-step sampling, improving fast-stroke segment retention without replacing existing spatial sampling contract.
- Runtime matcher window search has bounded candidate budgets to avoid long-path candidate explosion:
  - spatial candidates: `spatialWindowMaxCandidates`
  - time candidates: `timeWindowMaxCandidates`
- Gesture window geometry supports runtime env calibration for both preset/custom dispatch:
  - `MFX_GESTURE_WINDOW_COVERAGE_MIN_PERCENT`
  - `MFX_GESTURE_WINDOW_COVERAGE_MAX_PERCENT`
  - `MFX_GESTURE_WINDOW_COVERAGE_STEP_PERCENT`
  - `MFX_GESTURE_WINDOW_SLIDE_DIVISOR`
  - `MFX_GESTURE_WINDOW_SPATIAL_MAX_CANDIDATES`
  - `MFX_GESTURE_WINDOW_TIME_MIN_MS`
  - `MFX_GESTURE_WINDOW_TIME_MAX_MS`
  - `MFX_GESTURE_WINDOW_TIME_STEP_MS`
  - `MFX_GESTURE_WINDOW_TIME_ANCHOR_STEP_MS`
  - `MFX_GESTURE_WINDOW_TIME_MAX_CANDIDATES`

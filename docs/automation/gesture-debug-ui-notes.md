# Gesture Debug UI Notes (P2)

## Scope
Detailed implementation notes for gesture-route debug UI rendering and preview behavior.
P1 (`docs/agent-context/current.md`) keeps only the summary; use this when working on
gesture diagnostics visualization or debug UI regressions.

## Detailed Notes
- WebUI debug rendering is decoupled and mounted in sidebar debug card; runtime state is synced via workspace runtime channel.
- Sidebar debug gesture preview uses inline SVG stroke/fill attributes (no CSS-only dependency). The displayed gesture id prefers the latest non-empty `recent_events.gesture_id` to avoid being overwritten by subsequent blank move/skip events.
- Sidebar debug card prefers explicit fields:
  - `last_recognized_gesture_id` / `recent_events[].recognized_gesture_id`
  - `last_matched_gesture_id` / `recent_events[].matched_gesture_id`
  - `last_matched_gesture_id` is strict mapping-hit only; no-hit route keeps it empty to avoid stale/false positive matched display.
  - `last_gesture_id` stays as compatibility fallback only.
- Sidebar debug card matched-gesture preview falls back to latest non-empty `recent_events[].matched_gesture_id`, preventing blank matched panel after intermediate non-hit events overwrite `last_*`.
- Gesture-route diagnostics include lightweight sampled preview points (`last_preview_points` + `recent_events[].preview_points`) and a path hash. Sidebar debug drawing is based on runtime sampled trajectory first, with gesture-id template as fallback.
- Debug preview fidelity is raised (default cap `180` points) for better visual restoration; still strictly debug-gated and absent in non-debug runtime.
- Sidebar debug preview rendering uses frontend densify + Chaikin smoothing + larger debug canvas (`108x52`) for less “matchstick” appearance while keeping recognition path unchanged.
- Debug preview fallback path is unified: both id-based and sampled-points rendering now go through resample+dense+smoothing; end-arrow tangent uses a stable tail-distance point to avoid “needle/matchstick” artifacts at stroke tail.
- Sidebar debug card no longer falls back to id-template geometry when sampled points are missing; it caches and renders the latest valid sampled trajectory to reduce “火柴棍” fallback artifacts during intermittent diagnostic frames.
- Gesture recognizer diagnostics carry raw preview trajectory points (`Result.previewPoints`) separate from matching sample points; sidebar debug rendering consumes this raw trajectory to improve stroke fidelity without changing matching behavior.
- Buttonless-gesture realtime loop stops feeding new recognizer points after a successful trigger (until idle re-arm/reset), preventing left debug preview from growing indefinitely during continuous movement.
- Sidebar recognized/matched gesture preview keeps full trajectory semantics and applies even downsampling before rendering (instead of tail-only slicing), so start/end direction remains consistent while controlling render cost.
- Buttonless diagnostics now carry short movement preview points even in `awaiting_motion_arm` and `post_trigger_hold` states; sidebar no longer clears preview on `awaiting_motion_arm`.
- Buttonless diagnostics preview maintains a rolling raw trajectory window (`96` points cap) instead of frame-local snippets, eliminating “moving but tail gradually disappears” behavior.
- Sidebar recognized preview selection prioritizes latest route frame points (`lastPreviewPoints`) over historical recognized-event snapshots, so UI reflects current motion first.
- Custom gesture diagnostics export flattened multi-stroke preview points for the selected best candidate, so debug preview reflects composite shapes (e.g., multi-stroke `D`).
- Sidebar debug recognized panel borrows matched preview as fallback during `custom_trigger/gesture_trigger` when current-frame recognized preview is empty, reducing intermittent blank frames.
- Diagnostics preview selection is generalized for all gesture types: both preset and custom route updates prefer best-window sliced preview points (when available) instead of full raw stroke, reducing partial-shape ambiguity.
- Recognized preview panel fallback is generic (`recognized || matched`) rather than stage-gated, preventing empty recognized panel for valid matched frames.
- Custom gesture matching in buttonless mode is generalized for multi-stroke templates: template strokes are flattened into a single ordered polyline for matching against the live single continuous stroke, instead of rejecting `template_stroke_count > 1`.
- Route-level window-sliced diagnostics preview was reverted for realtime recognized panel stability; diagnostics preview now keeps full selected trajectory to avoid gradual tail-shortening artifacts during continuous drawing.
- Sidebar debug card clears stale recognized-gesture preview when route stage is `buttonless_move_skipped` (awaiting arm) or `buttonless_idle_reset`, avoiding “log更新但图形看起来卡住”的误判。
- Debug-only realtime pull is enabled only when:
  - runtime diagnostics payload exists (debug mode),
  - connection is online,
  - focused section is `automation`.
- Polling cadence is adaptive in debug mode:
  - active gesture stages: `~66ms`
  - idle/non-active stages: `~180ms`
  - only syncs sidebar runtime debug state (no full page rerender).
- WebUI poll bootstrap uses idle interval constant (`GESTURE_DEBUG_POLL_MS_IDLE`) to avoid undefined symbol reload failure.

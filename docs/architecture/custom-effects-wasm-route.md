# Custom Effects (WASM Route)

## Purpose
Define the stable architecture contract for custom effects:
- Plugin logic in WASM.
- Rendering/resource ownership in host C++.
- Behavior parity and runtime safety before visual polish.

This document is intentionally compact. Historical per-phase details are kept in targeted issue docs.

## Scope
- In scope:
  - click/text/image custom logic,
  - host-side budget enforcement,
  - Web settings policy/diagnostics,
  - plugin template and local build workflow.
- Out of scope:
  - direct JS-to-WASM runtime translation,
  - plugin control of host window/swapchain,
  - visual node editor.

## Data Flow
1. Host captures normalized input events.
2. `WasmEffectHost` calls plugin ABI (`on_input`) with event payload.
3. Host drives periodic `on_frame` calls to fetch continuous animation output.
4. Plugin returns command buffer.
5. Host validates budget and command schema.
6. Host renderer executes supported commands and applies fallback on failure.

Core rule: WASM computes; C++ executes.

Surface host routing rule:
- Host maintains two independent wasm host lanes under one shared runtime policy:
  - `effects` lane for click/move/scroll/hold/hover effect plugins.
  - `indicator` lane for `indicator_*` input-indicator plugins.
- `POST /api/wasm/load-manifest` must route by `surface` and validate against the target lane diagnostics.

## Plugin Contract (ABI v2)
Required exports:

```c
uint32_t mfx_plugin_get_api_version(void);
uint32_t mfx_plugin_on_input(
  const uint8_t* input_ptr,
  uint32_t input_len,
  uint8_t* output_ptr,
  uint32_t output_cap);
uint32_t mfx_plugin_on_frame(
  const uint8_t* input_ptr,
  uint32_t input_len,
  uint8_t* output_ptr,
  uint32_t output_cap);
void mfx_plugin_reset(void); // optional but recommended
```

Event kinds follow normalized host semantics (`click/move/scroll/hold*/hover*`) plus indicator lanes
(`indicator_click/indicator_scroll/indicator_key`) for input-indicator styling.

Indicator event notes:
- `indicator_*` events still use `EventInputV2` as the base payload.
- The host appends an optional `IndicatorEventTailV1` after `EventInputV2` to expose
  `size_px` and `duration_ms` for indicator styling (4 bytes total).
- When present, the host also appends `IndicatorEventContextTailV2` after
  `IndicatorEventTailV1` to expose richer style context:
  `primary_code`, `streak`, `modifier_mask`, and `detail_flags`.
  `primary_code` maps to button / wheel-magnitude / `vkCode` depending on event kind.
- When spawning text for indicator labels, use `text_id = kTextIdEventLabel` to receive the
  per-event label (e.g., `L x2`, `W+ x3`, `Cmd+Tab`).

Manifest-level route hints (optional):
- `input_kinds` narrows which normalized input lanes call `on_input`.
- `surfaces` declares which host surface the plugin is intended for (`effects` / `indicator` / `all`).
- `enable_frame_tick` controls whether host drives periodic `on_frame`.

Indicator route observability (test/runtime):
- `POST /api/wasm/test-dispatch-app-indicator-key` dispatches a key event through the real app route
  (`DispatchInputIndicatorKey`) and returns the latest route snapshot.
- Snapshot fields include branch booleans (`route_attempted`, `event_supported`,
  `rendered_by_wasm`, `native_fallback_applied`) and a reason code
  (`wasm_rendered`, `fallback_disabled`, `event_not_supported`, `plugin_unloaded`,
  `anchor_unavailable`, `invoke_failed_no_output`).
- `/api/state` also exposes the same latest snapshot under
  `input_indicator_wasm_route_status` after a wasm-route attempt.

## Command Contract (Current)
Supported render commands (current production path):
- `spawn_text`
- `spawn_image`
- `spawn_image_affine` (keeps `spawn_image` prefix fields and appends affine metadata)
- `spawn_pulse` (generic click-pulse primitive: `ripple` / `star`)
- `spawn_polyline` (generic line/polyline primitive with variable-length point payload)
- `spawn_path_stroke` (generic path-stroke primitive with move/line/quad/cubic/close nodes)
- `spawn_path_fill` (generic path-fill primitive with the same node stream plus `non_zero/even_odd` fill rules)
- `spawn_ribbon_strip` (generic centerline ribbon/trail strip primitive with per-point width)
- `spawn_glow_batch` (generic batched glow-particle primitive for dense particle/spray effects)
- `spawn_sprite_batch` (generic batched image-sprite primitive for dense image/confetti/badge bursts)
- `spawn_quad_batch` (generic batched textured-quad primitive with explicit width/height and optional atlas UV rects)
- `upsert_glow_emitter` (cross-platform retained glow-emitter baseline for updating one long-lived emitter)
- `remove_glow_emitter` (remove a retained glow emitter by id)
- `upsert_sprite_emitter` (cross-platform retained sprite-emitter baseline for updating one long-lived image/fallback emitter)
- `remove_sprite_emitter` (remove a retained sprite emitter by id)
- `upsert_particle_emitter` (cross-platform generic retained particle-emitter baseline for soft-glow/solid-disc particles)
- `remove_particle_emitter` (remove a retained particle emitter by id)
- `upsert_ribbon_trail` (cross-platform retained ribbon/trail baseline for one long-lived centerline strip)
- `remove_ribbon_trail` (remove a retained ribbon trail by id)
- `upsert_quad_field` (cross-platform retained quad-field baseline for one long-lived textured-quad cluster)
- `remove_quad_field` (remove a retained quad field by id)
- `remove_group` (remove all retained instances in the same `group_id` under the active manifest)
- `upsert_group_presentation` (retained-only group presentation update for `alpha_multiplier/visible` across one `group_id` under the active manifest)
- `upsert_group_clip_rect` (retained-only group clip update for one `group_id`; current host meaning is an effective `instance_clip ∩ group_clip` on retained glow/sprite/particle/ribbon/quad, with an optional group-mask tail for `rect|round_rect|ellipse`)
- `upsert_group_layer` (retained-only group layer update for one `group_id`; current host meaning is `blend override + sort bias` on retained glow/sprite/particle/ribbon/quad)
- `upsert_group_transform` (retained-only group transform update for one `group_id`; base header keeps translation `offset_x_px/offset_y_px`, an optional first tail can add `rotation_rad + uniform_scale`, an optional second tail can add `pivot_x_px/pivot_y_px`, and an optional third tail can add `scale_x/scale_y`; current host meaning remains staged, with translation on retained glow/sprite/particle/ribbon/quad and geometry recomposition from the transform tails now applying to retained glow/sprite/particle/ribbon/quad lanes that also opt into `...FLAG_USE_GROUP_LOCAL_ORIGIN`)
- `upsert_group_local_origin` (retained-only group local-origin update for one `group_id`; current host meaning is one screen-space local origin consumed only by retained commands that opt into `...FLAG_USE_GROUP_LOCAL_ORIGIN`)
- `upsert_group_material` (retained-only group material update for one `group_id`; current host meaning is host-owned `tint override + intensity multiplier`, plus optional host-owned style/response/feedback tails for `soft_bloom_like|afterimage_like`, `diffusion_amount|persistence_amount`, and `echo_amount|echo_drift_px`, plus optional host-owned feedback-mode/feedback-stack tails for `directional|tangential|swirl + phase_rad` and `echo_layers + echo_falloff`, all reapplied to retained glow/sprite/particle/ribbon/quad only)
- `upsert_group_pass` (retained-only group controlled-pass update for one `group_id`; current host meaning is intentionally narrow and host-owned: `soft_bloom_like|afterimage_like|echo_like + pass_amount + response_amount`, plus an optional mode tail for `directional|tangential|swirl + phase_rad`, plus an optional stack tail for `echo_layers + echo_falloff`, plus an optional pipeline tail for one secondary host-owned pass stage `secondary_pass_kind + secondary_pass_amount + secondary_response_amount`, plus an optional blend tail for `secondary_blend_mode(multiply|lerp) + secondary_blend_weight`, plus an optional routing tail for `secondary_route_mask(glow|sprite|particle|ribbon|quad)`, plus an optional lane-response tail for `secondary_glow|sprite|particle|ribbon|quad_response`, plus an optional temporal tail for `phase_rate_rad_per_sec + secondary_decay_per_sec + secondary_decay_floor`, plus an optional temporal-mode tail for `exponential|linear|pulse + temporal_strength`, plus an optional tertiary-stage tail for `tertiary_pass_kind + tertiary_pass_amount + tertiary_response_amount + tertiary_blend_mode + tertiary_blend_weight`, plus an optional tertiary-routing tail for `tertiary_route_mask(glow|sprite|particle|ribbon|quad)`, plus an optional tertiary lane-response tail for `tertiary_glow|sprite|particle|ribbon|quad_response`, plus an optional tertiary temporal tail for `tertiary_phase_rate_rad_per_sec + tertiary_decay_per_sec + tertiary_decay_floor`, plus an optional tertiary temporal-mode tail for `tertiary_temporal_mode + tertiary_temporal_strength`, plus an optional tertiary stack tail for `tertiary_echo_layers + tertiary_echo_falloff`, reapplied to retained glow/sprite/particle/ribbon/quad only without exposing raw shader/post-process control)

Common fields:
- transform: `x, y, scale, rotation`
- motion: `vx, vy, ax, ay`
- style: `alpha, color`
- lifecycle: `delay_ms, life_ms`
- resource selector: `text_id` or `image_id`

`spawn_pulse` fields:
- geometry: `x, y, start_radius_px, end_radius_px, stroke_width_px`
- style: `alpha, fill_argb, stroke_argb, glow_argb`
- lifecycle: `delay_ms, life_ms`
- renderer selector: `pulse_kind = ripple | star`

`spawn_polyline` fields:
- geometry: `point_count`, `points[]`, `line_width_px`
- style: `alpha, stroke_argb, glow_argb`
- lifecycle: `delay_ms, life_ms`
- flags: `closed`

`spawn_glow_batch` fields:
- batch header: `item_count, flags, delay_ms, life_ms`
- item geometry: `x, y, radius_px`
- item style: `alpha, color_argb`
- item motion: `vx, vy, ax, ay`
- legacy flags: `screen_blend`
- optional render tail (appended after item payload): `blend_mode, sort_key, group_id`
- optional clip tail (currently sprite/quad lanes on macOS, appended after the render tail): `left_px, top_px, width_px, height_px`

`spawn_path_stroke` fields:
- path header: `node_count, line_width_px, alpha, delay_ms, life_ms`
- style: `stroke_argb, glow_argb`
- stroke style: `line_join, line_cap`
- node stream: `move_to | line_to | quad_to | cubic_to | close`
- optional render tail (appended after node payload): `blend_mode, sort_key, group_id`
- optional clip tail (currently path lane only, appended after the render tail): `left_px, top_px, width_px, height_px`

`spawn_path_fill` fields:
- path header: `node_count, alpha, glow_width_px, delay_ms, life_ms`
- style: `fill_argb, glow_argb`
- fill style: `fill_rule`
- node stream: `move_to | line_to | quad_to | cubic_to | close`
- optional render tail (appended after node payload): `blend_mode, sort_key, group_id`
- optional clip tail (currently path lane only, appended after the render tail): `left_px, top_px, width_px, height_px`

`spawn_ribbon_strip` fields:
- strip header: `point_count, alpha, glow_width_px, delay_ms, life_ms`
- style: `fill_argb, glow_argb`
- point stream: `x, y, width_px`
- flags: `closed`
- optional render tail (appended after point payload): `blend_mode, sort_key, group_id`
- optional clip tail (currently path lane only, appended after the render tail): `left_px, top_px, width_px, height_px`

`spawn_sprite_batch` fields:
- batch header: `item_count, flags, delay_ms, life_ms`
- item geometry: `x, y, scale`
- item style: `alpha, rotation, tint_argb`
- item resource: `image_id`
- item motion: `vx, vy, ax, ay`
- legacy flags: `screen_blend`
- optional render tail (appended after item payload): `blend_mode, sort_key, group_id`

`spawn_quad_batch` fields:
- batch header: `item_count, flags, delay_ms, life_ms`
- item geometry: `x, y, width_px, height_px`
- item style: `alpha, rotation, tint_argb`
- item resource: `image_id`
- item atlas rect: `src_u0, src_v0, src_u1, src_v1`
- item motion: `vx, vy, ax, ay`
- legacy flags: `screen_blend`
- optional render tail (appended after item payload): `blend_mode, sort_key, group_id`
- optional clip tail (currently sprite/quad lanes on macOS, appended after the render tail): `left_px, top_px, width_px, height_px`

`upsert_glow_emitter` fields:
- emitter identity/anchor: `emitter_id, x, y`
- emission: `emission_rate_per_sec, direction_rad, spread_rad`
- particle speed/size: `speed_min, speed_max, radius_min_px, radius_max_px`
- particle style: `alpha_min, alpha_max, color_argb`
- lifecycle/budget: `acceleration_x, acceleration_y, emitter_ttl_ms, particle_life_ms, max_particles`
- legacy flags: `screen_blend`
- optional render tail (appended after fixed payload): `blend_mode, sort_key, group_id`
- optional clip tail (currently retained emitter lanes on macOS, appended after the render tail): `left_px, top_px, width_px, height_px`

`remove_glow_emitter` fields:
- `emitter_id`

`upsert_sprite_emitter` fields:
- emitter identity/anchor: `emitter_id, image_id, x, y`
- emission: `emission_rate_per_sec, direction_rad, spread_rad`
- particle speed/size: `speed_min, speed_max, size_min_px, size_max_px`
- particle style: `alpha_min, alpha_max, tint_argb, rotation_min_rad, rotation_max_rad`
- lifecycle/budget: `acceleration_x, acceleration_y, emitter_ttl_ms, particle_life_ms, max_particles`
- legacy flags: `screen_blend`
- optional render tail (appended after fixed payload): `blend_mode, sort_key, group_id`
- optional clip tail (currently retained emitter lanes on macOS, appended after the render tail): `left_px, top_px, width_px, height_px`

`remove_sprite_emitter` fields:
- `emitter_id`

`upsert_particle_emitter` fields:
- emitter identity/anchor: `emitter_id, x, y`
- emission: `emission_rate_per_sec, direction_rad, spread_rad`
- particle speed/size: `speed_min, speed_max, radius_min_px, radius_max_px`
- particle style: `alpha_min, alpha_max, color_argb, particle_style = soft_glow | solid_disc`
- lifecycle/budget: `acceleration_x, acceleration_y, emitter_ttl_ms, particle_life_ms, max_particles`
- optional curve tail (appended after fixed payload, before the optional render tail): `size_start_scale, size_end_scale, alpha_start_scale, alpha_end_scale, color_start_argb, color_end_argb`
- legacy flags: `screen_blend`
- optional render tail (appended after fixed payload): `blend_mode, sort_key, group_id`
- optional clip tail (currently retained emitter lanes on macOS, appended after the render tail): `left_px, top_px, width_px, height_px`

`remove_particle_emitter` fields:
- `emitter_id`

`upsert_ribbon_trail` fields:
- trail identity: `trail_id`
- strip geometry/lifetime: `point_count, alpha, glow_width_px, life_ms`
- style: `fill_argb, glow_argb`
- point stream: `x, y, width_px`
- flags: `closed`
- optional render tail (appended after point payload): `blend_mode, sort_key, group_id`

`remove_ribbon_trail` fields:
- `trail_id`

`upsert_quad_field` fields:
- field identity/lifetime: `field_id, item_count, ttl_ms`
- item geometry: `x, y, width_px, height_px`
- item style: `alpha, rotation, tint_argb`
- item resource: `image_id`
- item atlas rect: `src_u0, src_v0, src_u1, src_v1`
- item motion: `vx, vy, ax, ay`
- optional render tail (appended after item payload): `blend_mode, sort_key, group_id`
- optional clip tail (currently macOS retained quad lane only, appended after the render tail): `left_px, top_px, width_px, height_px`
- item motion: `vx, vy, ax, ay`
- legacy flags: `screen_blend`
- optional render tail (appended after item payload): `blend_mode, sort_key, group_id`

`remove_quad_field` fields:
- `field_id`

Notes:
- ABI is v2, while command struct names remain `*CommandV1` for binary-layout continuity.
- `spawn_image_affine` currently resolves to image translation/scale/rotation semantics in host render path.
- `spawn_pulse` intentionally reuses host click-pulse renderers instead of adding a new effect-specific engine lane; WASM can layer delayed pulses/stars without modifying native effect routing again.
- `spawn_polyline` is the first variable-length WASM draw command in the host. It is intended as the baseline primitive for bolts, trajectories, outlines, and future trail-like custom effects without adding another built-in effect lane.
- `spawn_path_stroke` extends the same command-buffer model to path/vector stroke composition. It is intended as the baseline primitive for ribbons, curved bolts, sigils, and closed stroke paths without forcing each shape into one-off host commands.
- `spawn_path_fill` extends that vector surface to filled shapes and cutouts. It is intended as the baseline primitive for badges, seals, blobs, and `even_odd` hollow motifs without forcing a host-side shape catalog.
- `spawn_ribbon_strip` extends that same surface into centerline-driven strips. It is intended as the first reusable baseline for ribbons, swing traces, light trails, and energy streaks without jumping directly to retained mesh/material contracts.
- `spawn_glow_batch` is the first batched particle primitive in the host. It is intended as the baseline building block for bursts, sprays, embers, glows, and other high-density effects without forcing plugins back onto image-specific batching.
- `spawn_sprite_batch` extends the same batched-command pattern to image-backed particles. It is intended as the baseline primitive for confetti, stickers, badges, emoji/image sprays, and similar high-density sprite effects without spawning one overlay per image command.
- `spawn_quad_batch` extends that transient image lane from scale-only sprites to explicit quads. It is intended as the first reusable baseline for atlas-backed UI shards, cropped badges/cards, stretched decals, and later `quad/atlas/ribbon` work without introducing a raw mesh/shader contract.
- `upsert_glow_emitter/remove_glow_emitter` are the first cross-platform retained lifecycle commands in the host. They let WASM keep a glow emitter alive, update it in place, and remove it explicitly instead of re-emitting only instant burst commands.
- `upsert_sprite_emitter/remove_sprite_emitter` extend that retained lifecycle surface to image-backed sprites while keeping host-owned overlay/resource control. When an image asset is missing, the host falls back to a tinted orb-style sprite instead of dropping the command.
- `upsert_particle_emitter/remove_particle_emitter` are the first generic retained particle-emitter baseline on top of that lifecycle model. v1 intentionally keeps the host style surface small (`soft_glow` / `solid_disc`) while reusing the same host-owned overlay lifecycle, counters, and render-semantics tail.
- `upsert_ribbon_trail/remove_ribbon_trail` extend that retained lifecycle surface to non-particle geometry. v1 intentionally keeps the contract small: a host-owned centerline strip with per-point width, fade-over-ttl behavior, and the same optional render-semantics tail.
- `upsert_quad_field/remove_quad_field` extend that retained lifecycle surface to non-particle textured geometry. v1 intentionally keeps the contract small: a host-owned quad cluster with per-item atlas rects, simple velocity/acceleration motion over a shared ttl, and the same optional render-semantics tail.
- particle emitters now also accept a small optional spawn tail for `cone|radial` emission plus `point|disc|ring` spawn shapes, an optional dynamics tail for `drag/turbulence`, and the optional curve tail for `size/alpha/color over life`. That keeps the fixed ABI stable while giving WASM a first retained particle spawn+dynamics+curve surface without introducing a full material/shader layer.
- `spawn_path_stroke`, `spawn_path_fill`, `spawn_ribbon_strip`, `spawn_glow_batch`, `spawn_sprite_batch`, `spawn_quad_batch`, `upsert_glow_emitter`, `upsert_sprite_emitter`, `upsert_particle_emitter`, `upsert_ribbon_trail`, and `upsert_quad_field` now also accept an optional shared render-semantics tail in v2 for `blend_mode/sort_key/group_id` without breaking the base layouts.
- current host meaning is intentionally small: `blend_mode=screen|add` both resolve to screen-like composition, `sort_key` orders Windows ripple overlays and macOS overlay window levels, and `group_id` is now the retained group key consumed by `remove_group`, `upsert_group_presentation`, `upsert_group_clip_rect`, `upsert_group_layer`, `upsert_group_transform`, and `upsert_group_material`.
- `spawn_path_stroke`, `spawn_path_fill`, and `spawn_ribbon_strip` now also accept a second optional `clip_rect` tail after the shared render-semantics tail; phase1 host support is intentionally narrow and currently clips only the macOS path lane with a screen-space rectangle.
- `spawn_sprite_batch`, `spawn_quad_batch`, and `upsert_quad_field` now also accept the same optional `clip_rect` tail after the shared render-semantics tail; phase2 host support remains intentionally narrow and currently clips only the macOS sprite/quad transient lanes plus retained quad-field lane with a screen-space rectangle.
- `upsert_glow_emitter`, `upsert_sprite_emitter`, and `upsert_particle_emitter` now also accept the same optional `clip_rect` tail after the shared render-semantics tail; phase3 host support remains intentionally narrow and currently clips only the macOS retained emitter lanes with a screen-space rectangle.
- `remove_group` is retained-only in v2. It clears retained glow/sprite/particle/ribbon/quad instances that share the same `group_id` under the current manifest, but it does not affect transient `spawn_*` overlays.
- `upsert_group_presentation` is also retained-only in v2. Current host meaning stays intentionally narrow: it stores per-group `alpha_multiplier/visible` state, applies that state to current and future retained glow/sprite/particle/ribbon/quad instances in the same manifest group, and still does not affect transient `spawn_*` overlays.
- `upsert_group_clip_rect` now also accepts an optional group-mask tail. Current host meaning stays intentionally narrow: the retained lane still resolves a final effective clip bounds rectangle first, then optionally renders that retained group through a `rect|round_rect|ellipse` mask that fits those bounds. Windows keeps compile/runtime parity without yet surfacing a visible group-mask effect.
- `upsert_group_layer` is also retained-only in v2. Current host meaning stays intentionally narrow: it stores per-group `blend override/sort bias` state, applies that state to current and future retained glow/sprite/particle/ribbon/quad instances in the same manifest group, and still does not affect transient `spawn_*` overlays.
- `upsert_group_transform` is retained-only too. The 16-byte base header remains translation-only, and optional transform tails can append `rotation_rad + uniform_scale`, `pivot_x_px/pivot_y_px`, and then `scale_x/scale_y` without changing the command kind. Current host interpretation stays intentionally narrow: translation applies to current and future retained glow/sprite/particle/ribbon/quad instances, while geometry recomposition from those optional tails now applies to retained glow/sprite/particle/ribbon/quad lanes that also use `group_local_origin`; transient `spawn_*` overlays remain unaffected.
- `upsert_group_local_origin` is retained-only too, and the host meaning also stays intentionally narrow: it stores one screen-space local origin per manifest/group pair, but only retained commands that explicitly opt into `...FLAG_USE_GROUP_LOCAL_ORIGIN` consume that origin. That keeps existing grouped retained samples on absolute coordinates while enabling a first group-local coordinate frame for new retained glow/sprite/particle/ribbon/quad instances.
- `upsert_group_material` is retained-only too, and the host meaning also stays intentionally narrow: it stores one host-owned material state per manifest/group pair. The base command keeps `tint override + intensity multiplier`; an optional v2 tail can append `soft_bloom_like|afterimage_like + style_amount`; an optional v3 response tail can append `diffusion_amount|persistence_amount`; an optional v4 feedback tail can append `echo_amount|echo_drift_px`; an optional v5 feedback-mode tail can append `directional|tangential|swirl + phase_rad`; and an optional v6 feedback-stack tail can append `echo_layers + echo_falloff`. The host then reapplies that state only to retained glow/sprite/particle/ribbon/quad instances by re-resolving their existing retained configs into color/size/glow/life/ttl and controlled echo-drift changes. That keeps the ABI closer to a controlled effect-language surface instead of exposing raw shader/material freedom to wasm.
- grouped retained samples such as `click-retained-group-clear` need `runtime_max_commands >= 3`; `click-retained-group-alpha`, `click-retained-group-clip`, `click-retained-group-layer`, `click-retained-group-mask`, and `click-retained-group-transform` need `runtime_max_commands >= 4`; `click-retained-group-local-origin` now needs `runtime_max_commands >= 7` because it emits grouped `glow + sprite + particle + ribbon + quad` upserts on left click. The core HTTP regression raises the runtime policy before dispatching grouped samples so multi-command bundles are not budget-truncated.
- The current retained public surface now includes glow-emitter, sprite-emitter, generic particle-emitter, ribbon-trail, and quad-field baselines. Material/post-process retained contracts remain follow-up v3 work.

## Runtime Budgets and Fallback
Default budgets:
- `max_execution_ms <= 1.0`
- `max_commands <= 256`
- `output_buffer_bytes` policy-bound

Fallback policy:
- timeout: drop current event output
- overflow: truncate command list
- repeated failure: disable plugin route and fallback to built-in effect

Observable runtime state:
- `wasm.runtime_backend`
- `wasm.runtime_fallback_reason`

## Current Delivery State
- Runtime route, diagnostics, and fallback are active in settings/state.
- Policy controls (`enabled`, `manifest_path`, `fallback_to_builtin_click`, budget fields) are persisted.
- Template ecosystem (presets/assets/build scripts) is published under:
  - `examples/wasm-plugin-template`

## Source of Truth Docs
- Template authoring guide:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/examples/wasm-plugin-template/README.md`
- Compatibility:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-compatibility.md`
- Troubleshooting entry:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/examples/wasm-plugin-template/README.md`
- Historical phase-by-phase notes were intentionally removed from active docs to reduce token load.
- For deep implementation chronology, use git history on this file and related WASM commits.

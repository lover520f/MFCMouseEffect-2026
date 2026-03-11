# WASM Plugin Template Quick Start

Template root:
- `examples/wasm-plugin-template`

## 1. Build

```bash
cd examples/wasm-plugin-template
npm install
npm run build
```

Or use pnpm:

```bash
pnpm install
pnpm run build
```

Output:
- `dist/effect.wasm`
- `dist/plugin.json`

Sample presets:

```bash
npm run build:sample -- --sample text-burst
npm run build:samples
npm run sync:runtime-samples
```

Cleanup behavior:
- `build` replaces root-level generated files under `dist/` and keeps `dist/samples/`.
- `build:sample` replaces the selected `dist/samples/<sample_key>/` bundle.
- `build:samples` clears `dist/samples/` first, then rebuilds the full preset matrix.
- `sync:runtime-samples` rebuilds official bundles, removes managed legacy/runtime sample dirs from the runtime scan root, and republishes the current official bundles.

Preset output:
- `dist/samples/<sample_key>/effect.wasm`
- `dist/samples/<sample_key>/plugin.json`
- `dist/samples/<sample_key>/assets/*` (if `image_assets` is declared)

Runtime sync options:
- `npm run sync:runtime-samples -- --skip-build true`
- `npm run sync:runtime-samples -- --runtime-root /custom/plugin/root`

Managed cleanup scope:
- current official sample/template ids (`demo.*.v2` from template presets)
- derived legacy official ids (`demo.*.v1`)
- non-official plugin directories are preserved

## 2. Optional `spawn_image` Assets

`plugin.json` can include `image_assets`.
- paths are relative to `plugin.json`
- supported: `.png/.jpg/.jpeg/.bmp/.gif/.tif/.tiff`
- `imageId` maps by index (modulo)
- invalid/missing assets fallback to built-in image renderer

`plugin.json` can also include route/cadence hints:
- `input_kinds`: optional string array to limit host input lanes
  (`click/move/scroll/hold_start/hold_update/hold_end/hover_start/hover_end/indicator_click/indicator_scroll/indicator_key/all`)
- `enable_frame_tick`: optional boolean to control host-driven `mfx_plugin_on_frame` calls

## 3. Place Plugin

Create folder by `plugin.json.id`:
- debug default: `<exe_dir>/plugins/wasm/<plugin_id>/`
- release default: `%AppData%\\MFCMouseEffect\\plugins\\wasm\\<plugin_id>/`
- extra scan root: `<exe_dir>/plugins/wasm` (portable fallback)
- debug convenience: host also scans `examples/wasm-plugin-template/dist`
- settings page can append an extra catalog root (`WASM Plugin -> Catalog root path`)

Copy at least `effect.wasm` + `plugin.json`; copy `assets/` when `image_assets` is used.

Notes:
- template default manifest id is `demo.template.default.v2`
- sample manifests use `demo.*.<sample>.v2` ids under `dist/samples/*`
- duplicate ids are deduped by scan-root precedence; configured `catalog_root_path` overrides default roots
- unsupported manifest `api_version` is ignored by catalog discovery
- nested `plugin.json` inside another plugin package is ignored by catalog discovery

## 4. Load + Enable

```bash
POST /api/wasm/load-manifest
{
  "manifest_path": "C:\\path\\to\\plugins\\wasm\\demo.template.default.v2\\plugin.json"
}
```

```bash
POST /api/wasm/enable
```

## 5. ABI Contract

Required exports:
- `mfx_plugin_on_input(input_ptr, input_len, output_ptr, output_cap)`
- `mfx_plugin_on_frame(input_ptr, input_len, output_ptr, output_cap)`

Recommended exports:
- `mfx_plugin_get_api_version() -> 2`
- `mfx_plugin_reset()`

ABI definition:
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginAbi.h`

Template helper note:
- `assembly/common/abi.ts` exposes `writeSpawnImageAffine(...)` for command kind `spawn_image_affine`.
- `assembly/common/abi.ts` also exposes `writeSpawnPulse(...)` for command kind `spawn_pulse` (`ripple/star` host primitive).
- `assembly/common/abi.ts` also exposes `writeSpawnPolylineHeader(...)` + `writeSpawnPolylinePoint(...)` for command kind `spawn_polyline` (variable-length polyline primitive).
- `assembly/common/abi.ts` also exposes `writeSpawnPathStrokeHeader(...)`, `writePathStrokeNodeMoveTo(...)`, `writePathStrokeNodeLineTo(...)`, `writePathStrokeNodeQuadTo(...)`, `writePathStrokeNodeCubicTo(...)`, and `writePathStrokeNodeClose(...)` for command kind `spawn_path_stroke` (variable-length path-stroke primitive).
- `assembly/common/abi.ts` also exposes `writeSpawnPathFillHeader(...)` and `writeSpawnPathFillHeaderWithSemantics(...)` for command kind `spawn_path_fill` (variable-length path-fill primitive sharing the same node writers).
- `assembly/common/abi.ts` also exposes `writeSpawnRibbonStripHeader(...)`, `writeSpawnRibbonStripHeaderWithSemantics(...)`, and `writeSpawnRibbonStripPoint(...)` for command kind `spawn_ribbon_strip` (variable-length centerline ribbon/trail strip primitive with per-point width).
- `assembly/common/abi.ts` also exposes `writeSpawnGlowBatchHeader(...)` + `writeSpawnGlowBatchItem(...)` for command kind `spawn_glow_batch` (batched glow-particle primitive for dense bursts/sprays).
- `assembly/common/abi.ts` also exposes `writeSpawnSpriteBatchHeader(...)` + `writeSpawnSpriteBatchItem(...)` for command kind `spawn_sprite_batch` (batched image-sprite primitive for dense confetti/sticker/image bursts).
- `assembly/common/abi.ts` also exposes `writeSpawnQuadBatchHeader(...)` + `writeSpawnQuadBatchItem(...)` for command kind `spawn_quad_batch` (batched textured-quad primitive for explicit width/height image shards and atlas crops).
- `assembly/common/abi.ts` also exposes `writeUpsertGlowEmitter(...)` + `writeRemoveGlowEmitter(...)` for command kinds `upsert_glow_emitter` / `remove_glow_emitter` (cross-platform retained glow-emitter baseline).
- `assembly/common/abi.ts` also exposes `writeUpsertSpriteEmitter(...)` + `writeRemoveSpriteEmitter(...)` for command kinds `upsert_sprite_emitter` / `remove_sprite_emitter` (cross-platform retained sprite-emitter baseline with image/fallback particles).
- `assembly/common/abi.ts` also exposes `writeUpsertParticleEmitter(...)` + `writeRemoveParticleEmitter(...)` for command kinds `upsert_particle_emitter` / `remove_particle_emitter` (cross-platform retained particle-emitter baseline with `soft_glow` / `solid_disc` styles).
- `assembly/common/abi.ts` also exposes `writeUpsertRibbonTrailHeader(...)`, `writeUpsertRibbonTrailHeaderWithSemantics(...)`, `writeUpsertRibbonTrailPoint(...)`, and `writeRemoveRibbonTrail(...)` for command kinds `upsert_ribbon_trail` / `remove_ribbon_trail` (cross-platform retained ribbon/trail baseline for host-owned centerline strips).
- `assembly/common/abi.ts` also exposes `writeUpsertQuadFieldHeader(...)`, `writeUpsertQuadFieldHeaderWithSemantics(...)`, `writeUpsertQuadFieldItem(...)`, and `writeRemoveQuadField(...)` for command kinds `upsert_quad_field` / `remove_quad_field` (cross-platform retained quad-field baseline for host-owned textured-quad clusters).
- `assembly/common/abi.ts` also exposes `writeRemoveGroup(...)` for command kind `remove_group` (retained-only group lifecycle primitive that clears all retained instances sharing one `group_id` under the active manifest).
- `assembly/common/abi.ts` also exposes `writeUpsertGroupPresentation(...)` for command kind `upsert_group_presentation` (retained-only group presentation primitive that updates `alpha/visible` for all retained instances sharing one `group_id` under the active manifest).
- `assembly/common/abi.ts` also exposes `writeUpsertGroupClipRect(...)` for command kind `upsert_group_clip_rect` (retained-only group clip primitive that updates the effective group clip for all retained instances sharing one `group_id` under the active manifest).
- `assembly/common/abi.ts` also exposes `writeUpsertGroupClipRectWithMaskTail(...)` when `upsert_group_clip_rect` also needs an optional group-mask tail (`rect|round_rect|ellipse` over the final effective retained clip bounds).
- `assembly/common/abi.ts` also exposes `writeUpsertGroupLayer(...)` for command kind `upsert_group_layer` (retained-only group layer primitive that updates `blend override/sort bias` for all retained instances sharing one `group_id` under the active manifest).
- `assembly/common/abi.ts` also exposes `writeUpsertGroupTransform(...)`, `writeUpsertGroupTransformWithTail(...)`, `writeUpsertGroupTransformWithTailAndPivot(...)`, and `writeUpsertGroupTransformWithTailPivotAndScale2D(...)` for command kind `upsert_group_transform`. The base form keeps translation-only `offset_x_px/offset_y_px`; the optional transform tails add `rotation_rad + uniform_scale`, then `pivot_x_px/pivot_y_px`, and then `scale_x/scale_y`. Current host meaning stays staged: translation affects all retained instances sharing one `group_id`, while geometry recomposition from the transform tails now applies to retained glow/sprite/particle/ribbon/quad lanes that also opt into `group_local_origin`.
- `assembly/common/abi.ts` also exposes `writeUpsertGroupLocalOrigin(...)` for command kind `upsert_group_local_origin` (retained-only group local-origin primitive that stores one screen-space origin per `group_id`; retained commands opt into that local coordinate frame via `...FLAG_USE_GROUP_LOCAL_ORIGIN`).
- `assembly/common/abi.ts` also exposes `writeUpsertGroupMaterial(...)`, `writeUpsertGroupMaterialWithStyle(...)`, `writeUpsertGroupMaterialWithStyleAndResponse(...)`, `writeUpsertGroupMaterialWithStyleResponseAndFeedback(...)`, `writeUpsertGroupMaterialWithAllTails(...)`, and `writeUpsertGroupMaterialWithFullTails(...)` for command kind `upsert_group_material` (retained-only group material primitive that updates host-owned `tint override + intensity multiplier`, plus optional host-owned style tail `soft_bloom_like|afterimage_like + style_amount`, plus optional host-owned response tail `diffusion_amount|persistence_amount`, plus optional host-owned feedback tail `echo_amount|echo_drift_px`, plus optional host-owned feedback-mode tail `directional|tangential|swirl + phase_rad`, plus optional host-owned feedback-stack tail `echo_layers + echo_falloff`, for all retained instances sharing one `group_id` under the active manifest).
- `assembly/common/abi.ts` also exposes `writeUpsertGroupPass(...)`, `writeUpsertGroupPassWithMode(...)`, `writeUpsertGroupPassWithModeAndStack(...)`, `writeUpsertGroupPassWithAllTails(...)`, `writeUpsertGroupPassWithFullTails(...)`, `writeUpsertGroupPassWithRoutingTails(...)`, `writeUpsertGroupPassWithLaneResponseTails(...)`, `writeUpsertGroupPassWithTemporalTails(...)`, `writeUpsertGroupPassWithTemporalModeTails(...)`, `writeUpsertGroupPassWithTertiaryTails(...)`, `writeUpsertGroupPassWithTertiaryRoutingTails(...)`, `writeUpsertGroupPassWithTertiaryLaneResponseTails(...)`, `writeUpsertGroupPassWithTertiaryTemporalTails(...)`, `writeUpsertGroupPassWithTertiaryTemporalModeTails(...)`, and `writeUpsertGroupPassWithTertiaryStackTails(...)` for command kind `upsert_group_pass` (retained-only host-owned controlled-pass primitive that updates `soft_bloom_like|afterimage_like|echo_like + pass_amount + response_amount`, plus optional `directional|tangential|swirl + phase_rad` mode, `echo_layers + echo_falloff` stack, `secondary_pass_kind + secondary_pass_amount + secondary_response_amount` pipeline, `secondary_blend_mode(multiply|lerp) + secondary_blend_weight` blend, `secondary_route_mask(glow|sprite|particle|ribbon|quad)` routing, `secondary_glow|sprite|particle|ribbon|quad_response` lane-response, `phase_rate_rad_per_sec + secondary_decay_per_sec + secondary_decay_floor` temporal, `exponential|linear|pulse + temporal_strength` temporal-mode, `tertiary_pass_kind + tertiary_pass_amount + tertiary_response_amount + tertiary_blend_mode + tertiary_blend_weight` tertiary-stage, `tertiary_route_mask(glow|sprite|particle|ribbon|quad)` tertiary-routing, `tertiary_glow|sprite|particle|ribbon|quad_response` tertiary lane-response, `tertiary_phase_rate_rad_per_sec + tertiary_decay_per_sec + tertiary_decay_floor` tertiary temporal, `tertiary_temporal_mode + tertiary_temporal_strength` tertiary temporal-mode, and `tertiary_echo_layers + tertiary_echo_falloff` tertiary stack tails, for all retained instances sharing one `group_id` under the active manifest).
- `assembly/common/abi.ts` also exposes `writeUpsertParticleEmitterWithSpawnTail(...)`, `writeUpsertParticleEmitterWithDynamicsTail(...)`, `writeUpsertParticleEmitterWithSpawnAndDynamicsTailsAndSemantics(...)`, and `writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemantics(...)` when the retained particle emitter needs explicit `cone|radial` emission, `point|disc|ring` spawn shapes, `drag/turbulence`, and `size/alpha/color over life`.
- `assembly/common/abi.ts` also exposes `writeSpawnPathStrokeHeaderWithSemantics(...)`, `writeSpawnPathFillHeaderWithSemantics(...)`, `writeSpawnGlowBatchHeaderWithSemantics(...)`, `writeSpawnSpriteBatchHeaderWithSemantics(...)`, `writeUpsertGlowEmitterWithSemantics(...)`, `writeUpsertSpriteEmitterWithSemantics(...)`, `writeUpsertParticleEmitterWithSemantics(...)`, `writeUpsertRibbonTrailHeaderWithSemantics(...)`, and `writeUpsertQuadFieldHeaderWithSemantics(...)` when you need the optional shared render-semantics tail (`blend_mode/sort_key/group_id`).
- `assembly/common/abi.ts` also exposes `writeSpawnPathStrokeHeaderWithSemanticsAndClip(...)`, `writeSpawnPathFillHeaderWithSemanticsAndClip(...)`, and `writeSpawnRibbonStripHeaderWithSemanticsAndClip(...)` when the path lane also needs the optional `clip_rect` tail after render semantics.
  - preset coverage: `click-path-fill-clip-window` covers `spawn_path_fill + clip_rect`, and `click-path-clip-lanes` covers the remaining `spawn_path_stroke + clip_rect` and `spawn_ribbon_strip + clip_rect` helpers.
- `assembly/common/abi.ts` also exposes `writeSpawnSpriteBatchHeaderWithSemanticsAndClip(...)`, `writeSpawnQuadBatchHeaderWithSemanticsAndClip(...)`, and `writeUpsertQuadFieldHeaderWithSemanticsAndClip(...)` when sprite/quad geometry lanes need the same `clip_rect` tail.
- `assembly/common/abi.ts` also exposes `writeUpsertGlowEmitterWithSemanticsAndClip(...)`, `writeUpsertSpriteEmitterWithSemanticsAndClip(...)`, and `writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemanticsAndClip(...)` when retained emitter lanes need that same `clip_rect` tail.
- `assembly/common/abi.ts` also exposes `writeUpsertRibbonTrailHeaderWithSemanticsAndClip(...)` when retained ribbon trails need that same `clip_rect` tail.

## 6. Built-in Presets

- `text-rise`
- `text-burst`
- `text-spiral`
- `text-wave-chain`
- `image-pulse`
- `image-burst`
- `image-lift`
- `mixed-text-image`
- `mixed-emoji-celebrate`
- `button-adaptive`
- `click-pulse-dual`
- `click-polyline-zigzag`
- `click-path-stroke-ribbon`
- `click-path-fill-badge`
- `click-path-fill-clip-window`
- `click-path-clip-lanes`
- `click-ribbon-trace`
- `click-glow-burst`
- `click-sprite-burst`
- `click-quad-atlas-burst`
- `click-retained-glow-field`
- `click-retained-sprite-fountain`
- `click-retained-particle-field`
- `click-retained-ribbon-trail`
- `click-retained-quad-field`
- `click-retained-group-clear`
- `click-retained-group-alpha`
- `click-retained-group-clip`
- `click-retained-group-layer`
- `click-retained-group-mask`
- `click-retained-group-transform`
- `click-retained-group-local-origin`
- `click-retained-group-material`
- `click-retained-group-pass`
- `move-stream-sparks`
- `scroll-particle-burst`
- `scroll-neon-burst`
- `hold-orbit-pulse`
- `hover-spark-ring`

Template details:
- `examples/wasm-plugin-template/README.md`
- `examples/wasm-plugin-template/README.zh-CN.md`

## 7. Troubleshooting

- `load-manifest` fails: check `entry` and file existence.
- No runtime output: verify `mfx_wasm_runtime.dll` build output.
- Runtime bridge missing: host falls back to Null runtime.
- Output dropped: inspect `/api/state` -> `wasm` diagnostics/budget flags.
- Grouped retained samples such as `click-retained-group-clear` emit multiple commands per input; they need `runtime_max_commands >= 3`.
- `click-retained-group-alpha` emits four commands on left click (`upsert_group_presentation + glow + ribbon + quad`), so it needs `runtime_max_commands >= 4`.
- `click-retained-group-clip`, `click-retained-group-layer`, and `click-retained-group-transform` also emit four commands on left click, so they need `runtime_max_commands >= 4`.
- `click-retained-group-mask` also emits four commands on left click (`upsert_group_clip_rect + mask tail + glow + ribbon + quad`), so it needs `runtime_max_commands >= 4`.
- `click-retained-group-local-origin` emits seven commands on left click (`upsert_group_local_origin + upsert_group_transform + glow + sprite + particle + ribbon + quad`), so it needs `runtime_max_commands >= 7`.
- `click-retained-group-material` emits five commands on left click (`glow + sprite + particle + ribbon + quad`), so it needs `runtime_max_commands >= 5`; the middle click update path adds one `upsert_group_material` that now uses the optional style tail, response tail, feedback tail, feedback-mode tail, and feedback-stack tail together.
- `click-retained-group-pass` emits five commands on left click (`glow + sprite + particle + ribbon + quad`), so it needs `runtime_max_commands >= 5`; the middle click update path adds one `upsert_group_pass`.

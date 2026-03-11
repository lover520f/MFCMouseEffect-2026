# MFCMouseEffect WASM Plugin Template

Official AssemblyScript template for `MFCMouseEffect` WASM v2 plugins.

Language: [English](README.md) | [中文](README.zh-CN.md)

## What This Template Contains

- Stable ABI helpers for click/event input and command-buffer output.
- Reusable random/color helpers.
- A full sample matrix covering text, image, mixed, pulse, and normalized event-lane effects.
- Build scripts for default build, single sample build, all-samples build, and runtime sample sync.

## Directory Structure

```text
examples/wasm-plugin-template/
  assembly/
    common/
      abi.ts                  # ABI constants + read/write helpers
      random.ts               # deterministic pseudo-random helpers
    samples/
      effects/
        text-rise.ts
        text-burst.ts
        text-spiral.ts
        text-wave-chain.ts
        image-pulse.ts
        image-burst.ts
        image-lift.ts
        mixed-text-image.ts
        mixed-emoji-celebrate.ts
        button-adaptive.ts
        click-pulse-dual.ts
        click-polyline-zigzag.ts
        click-path-fill-clip-window.ts
        click-path-clip-lanes.ts
        click-ribbon-trace.ts
        click-glow-burst.ts
        click-sprite-burst.ts
        click-quad-atlas-burst.ts
        click-retained-quad-field.ts
        click-retained-ribbon-trail.ts
        click-retained-group-pass.ts
        move-stream-sparks.ts
        scroll-particle-burst.ts
        scroll-neon-burst.ts
        hold-orbit-pulse.ts
        hover-spark-ring.ts
      indicator/
        input-indicator-basic.ts
        input-indicator-keyviz.ts
    index.ts                  # default entry (currently exports text-rise)
  scripts/
    build-lib.mjs             # shared build helper
    sample-presets.mjs        # sample matrix metadata
    build.mjs                 # build default entry
    build-sample.mjs          # build one sample by key
    build-all-samples.mjs     # build all sample bundles
    sync-runtime-samples.mjs  # sync official bundles into runtime plugin root
    clean.mjs                 # clean dist
  plugin.json                 # default manifest template
  asconfig.json
  package.json
  README.md
  README.zh-CN.md
```

## Install and Build

```bash
pnpm install
pnpm run build
```

Or with npm:

```bash
npm install
npm run build
```

Default output:
- `dist/effect.wasm`
- `dist/effect.wat`
- `dist/plugin.json`

## Build Sample Presets

Build one sample:

```bash
pnpm run build:sample -- --sample text-burst
```

Build all samples:

```bash
pnpm run build:samples
pnpm run sync:runtime-samples
```

Sample output layout:
- `dist/samples/<sample_key>/effect.wasm`
- `dist/samples/<sample_key>/effect.wat`
- `dist/samples/<sample_key>/plugin.json`
- `dist/samples/<sample_key>/assets/*` (auto-copied image assets referenced by `image_assets`)

Cleanup semantics:
- `pnpm run build` replaces root-level generated files under `dist/` and preserves `dist/samples/`.
- `pnpm run build:sample -- --sample <key>` replaces only `dist/samples/<key>/`.
- `pnpm run build:samples` clears `dist/samples/` first, then rebuilds the full preset matrix.
- `pnpm run sync:runtime-samples` rebuilds official bundles, removes managed legacy/runtime sample dirs, and republishes current official bundles into the runtime scan root.
- `pnpm run clean` removes the entire `dist/` tree.

Runtime sync options:
- `pnpm run sync:runtime-samples -- --skip-build true`
- `pnpm run sync:runtime-samples -- --runtime-root /custom/plugin/root`

Managed cleanup scope:
- official template/sample ids (`demo.*.v2` from this template)
- derived legacy official ids (`demo.*.v1`)
- custom plugin directories outside that id set are preserved

## Full Sample Matrix

| key | category | behavior summary | image_assets |
| --- | --- | --- | --- |
| `text-rise` | text | single upward text with button-sensitive drift | no |
| `text-burst` | text | dual-side text burst | no |
| `text-spiral` | text | tri-text spiral-like spread | no |
| `text-wave-chain` | text | 4-step waved text chain | no |
| `image-pulse` | image | single pulse image chosen by button | yes |
| `image-burst` | image | 3-image radial burst | yes |
| `image-lift` | image | 2-image lift sequence | yes |
| `mixed-text-image` | mixed | one text + one image | yes |
| `mixed-emoji-celebrate` | mixed | 2 texts + 2 images celebration | yes |
| `button-adaptive` | mixed | text/image ids adapt by mouse button | yes |
| `indicator-basic` | indicator | input-indicator style sample: keyboard uses a dedicated key-cap panel, pointer lanes use a dedicated pointer shell + button zone + wheel slot helper, and all lanes consume `indicator_*` tails for streak/modifier-aware styling (no extra pulse/ripple command; scroll lane keeps click-zone inactive) | no |
| `indicator-keyviz-style` | indicator | keyviz-inspired indicator style: keyboard lane uses layered keycap cards and pointer/scroll lanes use dark pointer cards with tinted accent zones, while preserving shared `indicator_*` tail semantics (no extra pulse/ripple command) | no |
| `click-pulse-dual` | pulse | layered ripple + star pulse stack using `spawn_pulse` only | no |
| `click-polyline-zigzag` | polyline | 3 delayed zigzag bolt strokes using `spawn_polyline` only | no |
| `click-path-stroke-ribbon` | path-stroke | 2 delayed curved ribbon strokes using `spawn_path_stroke` plus the optional shared render-semantics tail | no |
| `click-path-fill-badge` | path-fill | 2 delayed filled badge/spark motifs using `spawn_path_fill` with an `even_odd` cutout plus the optional shared render-semantics tail | no |
| `click-path-fill-clip-window` | path-fill | 1 clipped filled badge motif using `spawn_path_fill` with the shared render-semantics tail followed by a `clip_rect` tail | no |
| `click-path-clip-lanes` | path-clip | 1 clipped `spawn_path_stroke` command plus 1 clipped `spawn_ribbon_strip` command so the remaining path-lane `clip_rect` helpers are covered by one dedicated preset | no |
| `click-ribbon-trace` | ribbon-strip | 2 delayed centerline ribbon traces using `spawn_ribbon_strip` with per-point width plus the optional shared render-semantics tail | no |
| `click-glow-burst` | glow-batch | 2 batched glow-particle bursts using `spawn_glow_batch` plus the optional shared render-semantics tail | no |
| `click-sprite-burst` | sprite-batch | 2 batched image-sprite bursts using `spawn_sprite_batch` with the shared render-semantics tail plus a `clip_rect` tail | yes |
| `click-quad-atlas-burst` | quad-batch | 2 batched textured-quad bursts using `spawn_quad_batch` with explicit width/height, per-item atlas UV crops, and a `clip_rect` tail after shared render semantics | yes |
| `click-retained-glow-field` | retained-emitter | left/middle click upserts a retained glow emitter with shared render semantics plus a clipped window, right click removes the same emitter id | no |
| `click-retained-sprite-fountain` | retained-emitter | left click upserts a retained sprite emitter with shared render semantics plus a clipped window, right click removes the same emitter id | yes |
| `click-retained-particle-field` | retained-emitter | left click upserts an expanding soft-glow particle field, middle click switches to shrinking solid-disc particles, and both retained variants now use a clipped window before right-click remove | no |
| `click-retained-quad-field` | retained-quad | left/middle click upserts a host-owned retained quad field with atlas rects, velocity/acceleration drift, shared render semantics, and a clipped window, right click removes the same field id | yes |
| `click-retained-ribbon-trail` | retained-ribbon | left click upserts a host-owned retained ribbon trail from a centerline point stream plus a clipped window, right click removes the same trail id | no |
| `click-retained-group-clear` | retained-group | left/middle click upserts glow+ribbon+quad retained instances into one shared `group_id`, right click issues `remove_group` to clear the whole retained group | yes |
| `click-retained-group-alpha` | retained-group | left click upserts glow+ribbon+quad plus `upsert_group_presentation(alpha=1, visible=true)`, middle click dims the same retained group, and right click issues `remove_group` | yes |
| `click-retained-group-clip` | retained-group | left click upserts glow+ribbon+quad plus `upsert_group_clip_rect`, middle click shifts the group clip window, and right click issues `remove_group` | yes |
| `click-retained-group-layer` | retained-group | left click upserts glow+ribbon+quad plus `upsert_group_layer(blend_override=off, sort_bias=0)`, middle click promotes the same retained group with a group layer override, and right click issues `remove_group` | yes |
| `click-retained-group-mask` | retained-group | left click upserts glow+ribbon+quad plus `upsert_group_clip_rect + mask tail(round-rect)`, middle click switches the same retained group to an ellipse mask, and right click issues `remove_group` | yes |
| `click-retained-group-transform` | retained-group | left click upserts glow+ribbon+quad plus `upsert_group_transform(offset=0,0)`, middle click translates the same retained group, and right click issues `remove_group` | yes |
| `click-retained-group-local-origin` | retained-group | left click sets `upsert_group_local_origin(x,y)` and spawns glow+sprite+particle+ribbon+quad in group-local coordinates plus `upsert_group_transform(..., rotation, scale, pivot, scale2d)` tails, middle click moves the same retained group's local origin, and right click issues `remove_group` | yes |
| `click-retained-group-material` | retained-group | left click spawns glow+sprite+particle+ribbon+quad under one retained `group_id`, middle click applies `upsert_group_material(tint,intensity,style_tail,response_tail,feedback_tail,feedback_mode_tail,feedback_stack_tail)` to the same group, and right click issues `remove_group` | yes |
| `click-retained-group-pass` | retained-group | left click spawns glow+sprite+particle+ribbon+quad under one retained `group_id`, middle click applies `upsert_group_pass(pass_kind,pass_amount,response_amount,mode_tail,stack_tail,pipeline_tail,blend_tail,routing_tail,lane_response_tail,temporal_tail,temporal_mode_tail,tertiary_tail,tertiary_routing_tail,tertiary_lane_response_tail,tertiary_temporal_tail,tertiary_temporal_mode_tail,tertiary_stack_tail)` to the same group so the existing grouped pass surface can express a fixed third host-owned stage with its own bounded routing/time/stack shaping on top of the routed/timed secondary stage, and right click issues `remove_group` | yes |
| `move-stream-sparks` | event(move) | cursor-move triggered text sparks stream | no |
| `scroll-particle-burst` | event(scroll) | wheel-driven colorful particle burst, now frame-driven (`on_input` state + `on_frame` continuous emit) | yes |
| `scroll-neon-burst` | event(scroll) | html-style neon wheel particle spray, frame-driven and using `spawn_image_affine` for richer sprite transforms | yes |
| `hold-orbit-pulse` | event(hold) | hold-driven orbit pulse with frame-updated ring sparks | no |
| `hover-spark-ring` | event(hover) | hover start/end with frame-driven spark ring twinkles | no |

`sample-presets.mjs` is the source of truth for:
- sample key
- source entry file
- plugin id/name/version
- manifest route hints (`input_kinds`/`enable_frame_tick`)
- optional `image_assets`

## Built-in Asset Pack (all supported formats)

Template `assets/` now includes and uses all host-supported image formats:
- `.png`: `smile.png`, `confetti.png`, `crown.png`, `emoji-2.png`, `mix-a.png`, `btn-left.png`, `particle-glow.png`
- `.jpg`: `coin.jpg`, `mix-b.jpg`
- `.jpeg`: `emoji-1.jpeg`
- `.bmp`: `star.bmp`
- `.gif`: `cat.gif`, `emoji-3.gif`, `party.gif`, `btn-right.gif`
- `.tif`: `lift-a.tif`
- `.tiff`: `lift-b.tiff`, `btn-middle.tiff`

These files are downloaded/generated into `examples/wasm-plugin-template/assets`
and copied into sample build outputs automatically.

## Manifest (`plugin.json`)

Minimal fields:

```json
{
  "id": "demo.template.default.v2",
  "name": "Demo Template Default",
  "version": "0.1.0",
  "api_version": 2,
  "entry": "effect.wasm",
  "surfaces": ["effects"],
  "input_kinds": ["click"],
  "enable_frame_tick": false
}
```

Optional image file mapping:

```json
{
  "image_assets": [
    "assets/smile.png",
    "assets/cat.gif"
  ]
}
```

Rules:
- paths are relative to `plugin.json`
- supported formats: `.png/.jpg/.jpeg/.bmp/.gif/.tif/.tiff`
- `imageId` maps to array index (modulo)
- invalid/missing assets fallback to built-in host renderers
- optional `input_kinds` limits which input lanes invoke the plugin
- optional `surfaces` declares which host surface the plugin targets
- optional `enable_frame_tick` controls whether host drives `mfx_plugin_on_frame`

`surfaces` values:
- `effects`
- `indicator`
- `all` (legacy manifests default to this when omitted, but new manifests should declare it explicitly)

`input_kinds` values:
- `click`
- `move`
- `scroll`
- `hold_start`
- `hold_update`
- `hold_end`
- `hover_start`
- `hover_end`
- `indicator_click`
- `indicator_scroll`
- `indicator_key`
- `all` (default when field is omitted)

Indicator payload notes:
- `indicator_*` still starts with `EventInputV2`
- optional `IndicatorEventTailV1` adds `size_px` + `duration_ms`
- optional `IndicatorEventContextTailV2` adds `primary_code` + `streak` + `modifier_mask` + `detail_flags`
- `assembly/common/abi.ts` exposes helpers such as `hasIndicatorContextTail(...)` and `readIndicatorStreak(...)`
- template motif helpers under `assembly/common/` can be reused when that improves the result, but indicator styling now also has its own dedicated helper at `assembly/common/input-indicator-style.ts`

## Runtime Placement

Copy `effect.wasm` + `plugin.json` to one plugin folder:
- Debug: `<exe_dir>/plugins/wasm/<plugin_id>/`
- Release: `%AppData%\\MFCMouseEffect\\plugins\\wasm\\<plugin_id>/`

`<plugin_id>` must equal `plugin.json.id`.

For sample bundles, copy the whole sample folder (`effect.wasm` + `plugin.json` + `assets/`).

Avoid scanning both:
- template root manifest (`dist/plugin.json`)
- and sample manifests under `dist/samples/*`
into one runtime catalog root when ids are duplicated by custom edits.

## ABI Reminder

Current ABI:
- `api_version = 2`

Required exports:
- `mfx_plugin_get_api_version`
- `mfx_plugin_on_input`
- `mfx_plugin_on_frame`

Recommended export:
- `mfx_plugin_reset`

Host compatibility rule:
- plugin must export `mfx_plugin_on_input` and `mfx_plugin_on_frame`.

Binary layout source of truth:
- `MFCMouseEffect/MouseFx/Core/Wasm/WasmPluginAbi.h`

Template ABI helper also exposes:
- `writeSpawnImageAffine(...)` (`COMMAND_KIND_SPAWN_IMAGE_AFFINE`, 88-byte command)
- `writeSpawnPulse(...)` (`COMMAND_KIND_SPAWN_PULSE`, 52-byte command)
- `writeSpawnPolylineHeader(...)` + `writeSpawnPolylinePoint(...)` (`COMMAND_KIND_SPAWN_POLYLINE`, variable-length command)
- `writeSpawnPathStrokeHeader(...)` + `writePathStrokeNode* (...)` (`COMMAND_KIND_SPAWN_PATH_STROKE`, variable-length path-stroke command)
- `writeSpawnPathFillHeader(...)` (`COMMAND_KIND_SPAWN_PATH_FILL`, variable-length path-fill command reusing the same path node writers)
- `writeSpawnRibbonStripHeader(...)` + `writeSpawnRibbonStripPoint(...)` (`COMMAND_KIND_SPAWN_RIBBON_STRIP`, variable-length centerline ribbon/trail strip command with per-point width)
- `writeSpawnGlowBatchHeader(...)` + `writeSpawnGlowBatchItem(...)` (`COMMAND_KIND_SPAWN_GLOW_BATCH`, variable-length batched particle command)
- `writeSpawnSpriteBatchHeader(...)` + `writeSpawnSpriteBatchItem(...)` (`COMMAND_KIND_SPAWN_SPRITE_BATCH`, variable-length batched sprite command)
- `writeSpawnQuadBatchHeader(...)` + `writeSpawnQuadBatchItem(...)` (`COMMAND_KIND_SPAWN_QUAD_BATCH`, variable-length batched textured-quad command with explicit width/height and atlas UVs)
- `writeUpsertGlowEmitter(...)` + `writeRemoveGlowEmitter(...)` (`COMMAND_KIND_UPSERT_GLOW_EMITTER` / `COMMAND_KIND_REMOVE_GLOW_EMITTER`, cross-platform retained glow-emitter baseline commands)
- `writeUpsertSpriteEmitter(...)` + `writeRemoveSpriteEmitter(...)` (`COMMAND_KIND_UPSERT_SPRITE_EMITTER` / `COMMAND_KIND_REMOVE_SPRITE_EMITTER`, cross-platform retained sprite-emitter baseline commands)
- `writeUpsertParticleEmitter(...)` + `writeRemoveParticleEmitter(...)` (`COMMAND_KIND_UPSERT_PARTICLE_EMITTER` / `COMMAND_KIND_REMOVE_PARTICLE_EMITTER`, cross-platform retained particle-emitter baseline commands)
- `writeUpsertRibbonTrailHeader(...)` + `writeUpsertRibbonTrailPoint(...)` + `writeRemoveRibbonTrail(...)` (`COMMAND_KIND_UPSERT_RIBBON_TRAIL` / `COMMAND_KIND_REMOVE_RIBBON_TRAIL`, cross-platform retained ribbon-trail baseline commands)
- `writeUpsertQuadFieldHeader(...)` + `writeUpsertQuadFieldItem(...)` + `writeRemoveQuadField(...)` (`COMMAND_KIND_UPSERT_QUAD_FIELD` / `COMMAND_KIND_REMOVE_QUAD_FIELD`, cross-platform retained quad-field baseline commands)
- `writeRemoveGroup(...)` (`COMMAND_KIND_REMOVE_GROUP`, retained-only group lifecycle command)
- `writeUpsertParticleEmitterWithLifeTail(...)`, `writeUpsertParticleEmitterWithSpawnTail(...)`, `writeUpsertParticleEmitterWithDynamicsTail(...)`, `writeUpsertParticleEmitterWithSpawnAndDynamicsTailsAndSemantics(...)`, and `writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemantics(...)` (retained particle-emitter helpers for explicit spawn-shape/emission tails, `drag/turbulence`, and `size/alpha/color over life`)
- `writeSpawnPathStrokeHeaderWithSemantics(...)`, `writeSpawnPathFillHeaderWithSemantics(...)`, `writeSpawnRibbonStripHeaderWithSemantics(...)`, `writeSpawnGlowBatchHeaderWithSemantics(...)`, `writeSpawnSpriteBatchHeaderWithSemantics(...)`, `writeSpawnQuadBatchHeaderWithSemantics(...)`, `writeUpsertGlowEmitterWithSemantics(...)`, `writeUpsertSpriteEmitterWithSemantics(...)`, `writeUpsertParticleEmitterWithSemantics(...)`, `writeUpsertRibbonTrailHeaderWithSemantics(...)`, and `writeUpsertQuadFieldHeaderWithSemantics(...)` for the optional shared render-semantics tail (`blend_mode/sort_key/group_id`)
- `writeSpawnPathStrokeHeaderWithSemanticsAndClip(...)`, `writeSpawnPathFillHeaderWithSemanticsAndClip(...)`, and `writeSpawnRibbonStripHeaderWithSemanticsAndClip(...)` when the path lane also needs the optional `clip_rect` tail (`left_px/top_px/width_px/height_px`) after the shared render-semantics tail
  - preset coverage: `click-path-fill-clip-window` covers `spawn_path_fill + clip_rect`, and `click-path-clip-lanes` covers the remaining `spawn_path_stroke + clip_rect` and `spawn_ribbon_strip + clip_rect` helpers
- `writeSpawnSpriteBatchHeaderWithSemanticsAndClip(...)`, `writeSpawnQuadBatchHeaderWithSemanticsAndClip(...)`, and `writeUpsertQuadFieldHeaderWithSemanticsAndClip(...)` when sprite/quad geometry lanes also need that same optional `clip_rect` tail
- `writeUpsertGlowEmitterWithSemanticsAndClip(...)`, `writeUpsertSpriteEmitterWithSemanticsAndClip(...)`, and `writeUpsertParticleEmitterWithSpawnDynamicsAndLifeTailsAndSemanticsAndClip(...)` when retained emitter lanes also need that same optional `clip_rect` tail
- `writeUpsertRibbonTrailHeaderWithSemanticsAndClip(...)` when retained ribbon trails also need that same optional `clip_rect` tail
- `writeUpsertGroupPresentation(...)` (`COMMAND_KIND_UPSERT_GROUP_PRESENTATION`, retained-only group presentation command)
- `writeUpsertGroupClipRect(...)` (`COMMAND_KIND_UPSERT_GROUP_CLIP_RECT`, retained-only group clip command)
- `writeUpsertGroupClipRectWithMaskTail(...)` (`COMMAND_KIND_UPSERT_GROUP_CLIP_RECT` + optional mask tail for `rect|round_rect|ellipse`)
- `writeUpsertGroupLayer(...)` (`COMMAND_KIND_UPSERT_GROUP_LAYER`, retained-only group layer command)
- `writeUpsertGroupTransform(...)` / `writeUpsertGroupTransformWithTail(...)` / `writeUpsertGroupTransformWithTailAndPivot(...)` / `writeUpsertGroupTransformWithTailPivotAndScale2D(...)` (`COMMAND_KIND_UPSERT_GROUP_TRANSFORM`; base form is translation-only, optional transform tails add `rotation_rad + uniform_scale`, then `pivot_x_px/pivot_y_px`, and then `scale_x/scale_y`; current geometry recomposition applies to retained glow/sprite/particle/ribbon/quad lanes that also opt into `group_local_origin`)
- `writeUpsertGroupLocalOrigin(...)` (`COMMAND_KIND_UPSERT_GROUP_LOCAL_ORIGIN`, retained-only group local-origin command)
- `writeUpsertGroupMaterial(...)` / `writeUpsertGroupMaterialWithStyle(...)` / `writeUpsertGroupMaterialWithStyleAndResponse(...)` / `writeUpsertGroupMaterialWithStyleResponseAndFeedback(...)` / `writeUpsertGroupMaterialWithAllTails(...)` / `writeUpsertGroupMaterialWithFullTails(...)` (`COMMAND_KIND_UPSERT_GROUP_MATERIAL`, retained-only group material command for host-owned `tint override + intensity multiplier`, plus optional host-owned style tail `soft_bloom_like|afterimage_like + style_amount`, plus optional host-owned response tail `diffusion_amount|persistence_amount`, plus optional host-owned feedback tail `echo_amount|echo_drift_px`, plus optional host-owned feedback-mode tail `directional|tangential|swirl + phase_rad`, plus optional host-owned feedback-stack tail `echo_layers + echo_falloff`)
- `writeUpsertGroupPass(...)` / `writeUpsertGroupPassWithMode(...)` / `writeUpsertGroupPassWithModeAndStack(...)` / `writeUpsertGroupPassWithAllTails(...)` / `writeUpsertGroupPassWithFullTails(...)` / `writeUpsertGroupPassWithRoutingTails(...)` / `writeUpsertGroupPassWithLaneResponseTails(...)` / `writeUpsertGroupPassWithTemporalTails(...)` / `writeUpsertGroupPassWithTemporalModeTails(...)` / `writeUpsertGroupPassWithTertiaryTails(...)` / `writeUpsertGroupPassWithTertiaryRoutingTails(...)` / `writeUpsertGroupPassWithTertiaryLaneResponseTails(...)` / `writeUpsertGroupPassWithTertiaryTemporalTails(...)` / `writeUpsertGroupPassWithTertiaryTemporalModeTails(...)` (`COMMAND_KIND_UPSERT_GROUP_PASS`, retained-only host-owned controlled-pass command for `soft_bloom_like|afterimage_like|echo_like + pass_amount + response_amount`, plus optional `directional|tangential|swirl + phase_rad` mode, `echo_layers + echo_falloff` stack, `secondary_pass_kind + secondary_pass_amount + secondary_response_amount` pipeline, `secondary_blend_mode(multiply|lerp) + secondary_blend_weight` blend, `secondary_route_mask(glow|sprite|particle|ribbon|quad)` routing, `secondary_glow|sprite|particle|ribbon|quad_response` lane-response, `phase_rate_rad_per_sec + secondary_decay_per_sec + secondary_decay_floor` temporal, `exponential|linear|pulse + temporal_strength` temporal-mode, `tertiary_pass_kind + tertiary_pass_amount + tertiary_response_amount + tertiary_blend_mode + tertiary_blend_weight` tertiary-stage, `tertiary_route_mask(glow|sprite|particle|ribbon|quad)` tertiary-routing, `tertiary_glow|sprite|particle|ribbon|quad_response` tertiary lane-response, `tertiary_phase_rate_rad_per_sec + tertiary_decay_per_sec + tertiary_decay_floor` tertiary temporal, and `tertiary_temporal_mode + tertiary_temporal_strength` tertiary temporal-mode tails)
- retained commands also expose `...FLAG_USE_GROUP_LOCAL_ORIGIN` when one retained instance should treat its own coordinates as local to `upsert_group_local_origin(...)`
- grouped retained samples such as `click-retained-group-clear` need `runtime_max_commands >= 3`; `click-retained-group-alpha`, `click-retained-group-clip`, `click-retained-group-layer`, `click-retained-group-mask`, and `click-retained-group-transform` need `runtime_max_commands >= 4`
  `click-retained-group-material` and `click-retained-group-pass` need `runtime_max_commands >= 5`, and `click-retained-group-local-origin` needs `runtime_max_commands >= 7`

## Related Docs

- `docs/architecture/wasm-plugin-template-quickstart.md`
- `docs/architecture/wasm-plugin-template-quickstart.zh-CN.md`
- `docs/architecture/wasm-plugin-compatibility.md`
- `docs/architecture/wasm-plugin-troubleshooting.md`

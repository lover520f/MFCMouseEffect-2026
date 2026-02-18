# WASM Phase2 Runtime Render Landing Validation

## Goal

Validate that WASM click commands now drive visible rendering (not log-only):
- `spawn_text`: visible floating text
- `spawn_image`: visible icon/image-command rendering
- on render failure, fallback to built-in click effect without breaking main flow

## Scope

Covers the 5 commit landing batch:
1. `spawn_text` command executor wired into click path
2. `spawn_image` command executor wired into click path
3. deterministic `text_id/image_id` resource resolver with fallback
4. render diagnostics coupled with budget/error fallback
5. this validation closure document

## Preconditions

1. Main app builds successfully (Debug x64).
2. Template plugin artifacts are generated:
   - `examples/wasm-plugin-template/dist/effect.wasm`
   - `examples/wasm-plugin-template/dist/plugin.json`
3. Artifacts are copied into plugin directory under `plugin.json.id`.

## Minimal validation flow

1. `POST /api/wasm/load-manifest`
2. `POST /api/wasm/enable`
3. Click anywhere on screen
4. `GET /api/state` and inspect `wasm` diagnostics

## Expected checks

1. Plugin/runtime status
- `wasm.enabled = true`
- `wasm.plugin_loaded = true`
- `wasm.runtime_backend` is present (`dynamic_bridge` or `null`)

2. Command execution status
- `wasm.last_command_count > 0`
- `wasm.last_rendered_by_wasm = true`
- at least one of:
  - `wasm.last_executed_text_commands > 0`
  - `wasm.last_executed_image_commands > 0`

3. Fallback robustness
- if `wasm.last_render_error` is non-empty, click feedback still appears via built-in fallback
- no crash/hang in main loop

## Known limitations (current stage)

1. `spawn_image` currently maps to built-in icon renderer pool (`star/ripple`), not external image assets yet.
2. `delay_ms` is currently treated as immediate render.
3. Motion parameters (`vx/vy/ax/ay`) are approximate mappings in current renderers.

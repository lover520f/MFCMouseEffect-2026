# Agent Current Context (2026-03-09)

## Scope and Priority
- Primary host: macOS.
- Primary objective: macOS mainline reaches stable Windows-equivalent behavior first.
- Hard constraints:
  - No Windows regressions.
  - Linux follows compile + contract coverage.
  - New macOS features stay Swift-first; do not expand `.mm` surface area.

## Runtime Lanes
- Stable lane: `scaffold`
- Progressive lane: `core` (`mfx_entry_posix_host`)
- Policy: land new cross-platform capability in `core` while keeping `scaffold` stable.

## Current Snapshot
- Current mainline is no longer “wasm demo” level; it is a cross-platform controlled-effects runtime with retained/group semantics.
- Primary active capability track is `wasm v3 phase1+`: expand expressive power without exposing raw GPU/shader control.
- Current stabilization theme is `group_pass` convergence: keep external ABI stable while reducing parser/runtime/style duplication before further widening.
- macOS hold-behavior baseline was recently corrected so `blend` no longer stalls stationary hold animation waiting for a pointer move; periodic hold updates now stay enabled for both `hold_only` and `blend`, with `move_only` remaining the only mode that disables the dedicated hold lane.
- macOS hold charge/neon overlays also now disable implicit Core Animation actions during timer-driven updates, so the orbit-head marker stays pinned to the ring instead of drifting through the circle interior between frames.
- macOS hold style parity was tightened again so only `charge` and `neon` keep the explicit outer progress arc/head marker; `lightning/hex/techRing/hologram/quantumHalo/fluxField` no longer inherit the shared circle that their Windows contracts do not render.
- macOS input-indicator placement is now aligned to the same top-left screen-space contract used by Windows and the shared WASM/docs layer; both `relative` offsets and `absolute` coordinates are interpreted with `+Y` pointing down, and the final panel origin is translated into Cocoa window coordinates only at the last presentation step.
- macOS input-indicator visuals no longer use the earlier text-only panel. The Swift bridge now draws a Windows-aligned mouse/keyboard indicator surface directly (mouse body, button highlights, wheel arrow, key panel), and scroll labels were normalized to `W+ / W-` to match the Windows indicator contract.
- macOS and Windows input-indicator keyboard labels now reuse the same shared formatter/streak semantics instead of drifting apart. macOS no longer falls back to opaque labels like `K88`; it now resolves readable key tokens (`Cmd+Tab`, `Key`, `X x2`) using the same combo/streak rules as Windows, while keeping `Cmd` as the platform-facing meta label.
- macOS input-indicator mouse clicks/scrolls now reuse the same streak + label logic as Windows (`L2/L3`, `W+ 2`) and the Swift view is now time-animated instead of a single static frame, so mouse click/scroll visuals track the Windows indicator cadence more closely.

## WASM Capability Summary

### Base Runtime
- Host accepts `api_version=2` only.
- Required exports: `mfx_plugin_on_input`, `mfx_plugin_on_frame`; `mfx_plugin_reset` remains optional.
- Runtime drives frame ticks independently of raw input frequency.
- Official template/sample matrix now covers `click/move/scroll/hold/hover`.
- Template manual-validation samples were recently hardened so multi-image/mixed/sprite/pulse presets read clearly in the macOS host: `image-burst`, `mixed-text-image`, `click-sprite-burst`, and `click-pulse-dual` now avoid overlapping anchors or misleading off-center pulse placement.
- macOS wasm transient lanes were further hardened after manual validation exposed real host bugs: `spawn_pulse` now converts screen coordinates to overlay coordinates before rendering, and shared `clip_rect` tails now resolve from screen space into overlay space before transient/retained clip application.

### Transient Primitives
- `spawn_text`
- `spawn_image`
- `spawn_image_affine`
- `spawn_pulse`
- `spawn_polyline`
- `spawn_path_stroke`
- `spawn_path_fill`
- `spawn_glow_batch`
- `spawn_sprite_batch`
- `spawn_quad_batch`
- `spawn_ribbon_strip`

### Retained Primitives
- `upsert_glow_emitter/remove_glow_emitter`
- `upsert_sprite_emitter/remove_sprite_emitter`
- `upsert_particle_emitter/remove_particle_emitter`
- `upsert_ribbon_trail/remove_ribbon_trail`
- `upsert_quad_field/remove_quad_field`

### Shared Semantics Already Live
- Shared render tail:
  - `blend_mode`
  - `sort_key`
  - `group_id`
- Shared optional clip tail:
  - path lane
  - sprite/quad lane
  - retained emitters
  - retained ribbon/quad field

### Retained Group Semantics
- `remove_group`
- `upsert_group_presentation`
- `upsert_group_clip_rect`
- `upsert_group_layer`
- `upsert_group_transform`
- `upsert_group_local_origin`
- `upsert_group_material`
- `upsert_group_pass`

### Group Clip / Mask
- Group clip is retained-only and resolves as `instance_clip ∩ group_clip`.
- Optional group mask tail supports:
  - `rect`
  - `round_rect`
  - `ellipse`
- Current visible interpretation is macOS-first; Windows keeps ABI/runtime parity first.

### Group Transform
- Base transform supports group translation.
- Optional tails now support:
  - `rotation_rad`
  - `uniform_scale`
  - `pivot_x_px/pivot_y_px`
  - `scale_x/scale_y`
- Geometry recomposition is active on retained `glow/sprite/particle/ribbon/quad` when the lane opts into `...FLAG_USE_GROUP_LOCAL_ORIGIN`.

### Group Material
- Group material stays host-owned and bounded.
- Current grouped material surface includes:
  - `tint override`
  - `intensity multiplier`
  - style presets
  - response shaping
  - feedback / feedback mode
  - feedback stack

### Group Pass
- `upsert_group_pass` is the dedicated retained-only controlled-pass primitive.
- Current grouped pass surface is at `v15`.
- Current pass surface includes:
  - primary pass: `pass_kind + pass_amount + response_amount`
  - mode tail
  - stack tail
  - secondary stage:
    - pipeline
    - blend
    - routing
    - lane response
    - temporal
    - temporal mode
  - tertiary stage:
    - stage tail
    - routing
    - lane response
    - temporal
    - temporal mode
    - stack
- Important boundary:
  - this is still a host-owned bounded pass surface
  - not a raw pass graph
  - not raw shader/material control

### Recent Internal Convergence
- `group_pass` secondary/tertiary runtime state is now unified under shared `GroupPassStageState`.
- `TryResolveUpsertGroupPassCommand(...)` now uses one shared optional-tail reader plus one advancing offset cursor.
- These changes are internal only; external ABI/sample behavior remains unchanged at `v15`.

### Known Boundaries
- Wasm can now express many complex 2D effects and grouped retained compositions.
- It still cannot directly own raw GPU/shader/pipeline control like native C++ rendering code.
- Windows runtime still needs more visible-behavior validation; macOS remains the most complete live lane.

## Current Capability Status
- Effects:
  - click/trail/scroll/hold/hover all supported in `core`
  - shared compute-command model is active
  - trail continuity fixes and hold-move arbitration hardening are in place
  - `overlay_target_fps` is wired on both macOS and Windows execution lanes
- Automation:
  - app-scope normalization/persistence/matcher contracts are stable
  - parser helpers are shared across regression/manual paths
- Theme catalog:
  - runtime catalog is registry-driven
  - external catalog root loading and folder-path settings are already live

## Regression Gates
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
- macOS ObjC++ surface gate:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-macos-objcxx-surface-regression.sh`

## High-Value Manual Entrypoints
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/mfx`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh --skip-build`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build`

## Key Contracts
- Keep `stdin` JSON command compatibility.
- Wasm ABI changes must remain backward compatible within the current v2 surface unless an explicit breaking migration is approved.
- Group/material/pass semantics stay host-owned and bounded; do not open raw shader/post-process control without an explicit architecture review.
- New macOS capability work stays Swift-first.

## Docs Governance State
- `current.md` is intentionally a compact first-read summary, not a change log.
- Detailed capability evolution belongs in:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/custom-effects-wasm-route.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/custom-effects-wasm-route.zh-CN.md`

## Where to Read History
- Phase and acceptance history:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/refactoring/phase-roadmap-macos-m1-status.md`
- Wasm architecture contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/custom-effects-wasm-route.md`
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/custom-effects-wasm-route.zh-CN.md`
- WASM v3 direction:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-abi-v3-design.zh-CN.md`

## Next Focus
- Keep `group_pass` converged before widening it again.
- Shift near-term effort from “add one more tail” toward:
  - Windows visible-runtime validation
  - sample/regression matrix hardening
  - doc/index noise control

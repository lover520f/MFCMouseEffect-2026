# Agent Current Context (2026-03-10)

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
- macOS input-indicator mouse clicks/scrolls now reuse the same streak + label logic as Windows (`L x2/L x3`, `W+ x2`) and the Swift view is now time-animated instead of a single static frame, so mouse click/scroll visuals track the Windows indicator cadence more closely.
- Input-indicator styling can now be routed through the WASM host via new `indicator_*` input kinds; indicator events append `size_px/duration_ms` plus an optional context tail (`primary_code/streak/modifier_mask/detail_flags`), and `kTextIdEventLabel` resolves per-event labels for key/click/scroll.
- The input-indicator settings UI now exposes a dedicated second-level plugin sub-tab: enabling the indicator plugin path flips `render_mode=wasm`, surfaces runtime/plugin readiness inline, and loads only indicator-compatible manifests (`indicator_*` or explicit `surfaces:["indicator"]`) into an indicator-dedicated WASM host lane.
- The base input-indicator toggles (`enabled` / `keyboard_enabled`) were compacted into a single horizontal toggle row under the section title instead of two separate form rows, reducing vertical whitespace while keeping the title/description block visually separate.
- The indicator plugin tab status area now uses a compact status-panel layout instead of equal-width metric cards: primary runtime/plugin facts stay readable in two columns, route diagnostics wrap independently, and the row density was tightened further (smaller paddings/min-heights/value text) to reduce vertical whitespace.
- WASM runtime host management is now split by surface (`effects` vs `indicator`) while keeping a shared enable/budget policy, so effect plugins and indicator plugins can coexist without last-load-wins override; startup/bootstrap and `/api/wasm/load-manifest` now route manifests to the correct host by surface.
- Effects plugin selection in the WASM panel now supports per-category routing (`click/trail/scroll/hold/hover`) with immediate policy persistence (`manifest_path_*`) plus per-category load, and the WebUI now materializes a single `effects` catalog into five pre-grouped channel lists by `input_kinds` so each dropdown stays channel-scoped without re-running fragile per-render filtering.
- WebUI information architecture now places effect-plugin management under `Visual Effects` as a second-level tab (`Effect Plugins`) instead of a top-level section; legacy `#wasm` hash routes are compatibility-mapped to `#active` + plugin sub-tab to avoid broken bookmarks.
- WebUI workspace information architecture now keeps the left sidebar navigation-only (hint + section tabs), and renders `Current Section` summary in the main content header area instead of as a sidebar pseudo-tab, reducing tab affordance confusion.
- WASM plugin manifests now support an explicit optional `surfaces` declaration (`effects` / `indicator` / `all`); catalog/UI now apply surface-aware filtering (including `/api/wasm/catalog` surface query) so indicator-vs-effect lists do not cross-contaminate, while legacy manifests still fall back to inferred `input_kinds`.
- Template/sample layering now explicitly treats indicator styling as the same WASM effect surface rather than a forked one, while keeping visual freedom: indicator lanes share the same ABI/command surface as regular effects, but the template now also ships a dedicated `input-indicator-style` helper for keyboard/mouse HUD composition instead of forcing reuse of click motifs when that hurts quality.
- POSIX core wasm regression gate is green after isolating dynamic event-label scope into its own runtime translation unit, and runtime sample sync now includes both `demo.indicator.basic.v2` and `demo.indicator.keyviz.v2` for direct validation of `indicator_click/indicator_scroll/indicator_key`; the core wasm HTTP contract lane now also exposes `test-dispatch-indicator-key` for scripted indicator-route checks when websettings bind is available.
- Input-indicator wasm dispatch now writes a reasoned route snapshot (`wasm_rendered` / `fallback_disabled` / `event_not_supported` / `plugin_unloaded` / `anchor_unavailable` / `invoke_failed_no_output`) and test route `test-dispatch-app-indicator-key` is wired into core wasm regression to assert both non-fallback wasm routing and native-fallback routing branches.
- `/api/state` now exports the latest indicator wasm route snapshot as `input_indicator_wasm_route_status` (when at least one wasm-route attempt occurred), so WebUI can display concrete fallback/non-render reasons without re-dispatching test events.
- Input-indicator "Plugin Override" menu now consumes `input_indicator_wasm_route_status` directly and shows route reason/event/fallback-applied state inline, avoiding opaque "plugin loaded but no effect" debugging on manual tests.
- The WebUI global stylesheet now mirrors the indicator plugin route diagnostics classes (`indicator-plugin-menu__route*`, `is-warn/is-ok/is-idle`) so the inline route-status block keeps card/pill layout instead of falling back to unstyled stacked text.
- When `input_indicator.render_mode=wasm`, runtime now enforces a safer wasm execution-budget floor for indicator plugins (`output_buffer_bytes>=4096`, `max_commands>=32`) so low-budget stress settings (for example `1024/1`) do not silently force indicator plugins into native fallback with zero wasm output.
- macOS indicator wasm text resolution now honors `kTextIdEventLabel` the same way as the core/windows route, so `indicator_*` samples no longer leak into generic `text_click` content (for example showing unrelated words like `美丽`) when plugins emit dynamic per-event labels.
- Indicator wasm labels now run through an indicator-event static text motion path (no random drift/sway and no pop-scale), while regular click text effects keep their floating animation semantics; static-label alpha now starts visible immediately (fade-out only near tail) to avoid high-frequency scroll updates making `W± xN` appear missing.
- Indicator wasm route keeps click/scroll streak in both context tails and dynamic text labels (`L x2/L x3`, `W+ x2`) with scroll streak cap aligned to `99` (not `9`), and `demo.indicator.basic.v2` now anchors mouse click + scroll labels to the same shell lower-half zone (scroll keeps streak-length adaptive scale) so wheel glyph + `W± xN` stay visible without boundary overflow.
- macOS wasm text dispatch now uses explicit dual channels (`effects` vs `indicator_label`) with independent admission policy (`min_text_interval_ms` vs `min_indicator_text_interval_ms`) and window caps (`text_max_windows` vs `indicator_text_max_windows`), removing the previous backend hardcoded `1/8` toggle and stabilizing `W`/`L xN` indicator label continuity under burst input.

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
  - gesture mappings now support shared modifier conditions (`any` / `none` / exact `primary|shift|alt`) on both macOS and Windows input lanes
  - WebUI gesture mapping now exposes preset-vs-custom gesture pattern editing, stores custom stroke templates plus per-binding similarity threshold (default `75%`), and keeps app targeting aligned with mouse mappings (`all apps` or selected app list)
  - gesture drag button is now owned per mapping (default `left`) instead of only a single recognizer-level top setting; legacy `gesture.trigger_button` is still accepted and used as a migration fallback when old configs do not yet store per-binding `trigger_button`
  - WebUI gesture-trigger button dropdown order is now pinned to `left -> middle -> right` (default-first), independent of backend option source ordering
  - shortcut recording now accepts modifier-only chords on both the WebUI keydown path and native shortcut-capture session instead of silently dropping them; displayed modifier labels are platform-aware (`Cmd`/`Option` on macOS, `Win`/`Alt` on Windows) and no longer duplicate a lone modifier press
  - native shortcut capture now applies a short grace window for modifier-only keydown: modifier+main chords (for example `Cmd+A`) win when typed continuously, while lone modifier presses (for example `Cmd`) still resolve as single-key shortcuts after the grace delay
  - enabled mouse/gesture automation mappings may now persist with empty shortcut text as an intentional "trigger scaffold" state; WebUI no longer blocks save for blank `keys`, while runtime dispatch still skips empty chords so placeholder mappings do not inject anything
  - gesture mapping rows now expose two independent shortcut lanes: `trigger modifiers` (match-only, recorded from `Cmd/Ctrl/Shift/Alt`) and `output shortcut` (injected keys); this restores `Cmd + gesture -> shortcut` workflows without introducing command-execution support
  - gesture row header now surfaces modifier summary as a dedicated pill alongside app-scope and output-shortcut pills
  - WebUI automation now wraps `mouse mappings` and `gesture mappings` in dedicated section shells to match the plugin-style grouped visual hierarchy and reduce visual clutter in mixed mapping screens
- automation settings layout is now task-flow based: top-level enable card, full-width `mouse mappings`, then full-width `gesture mappings`; `gesture recognizer` parameters are merged into the `gesture mappings` card to keep ownership and horizontal space consistent
- automation shortcut capture helper copy is no longer always visible; WebUI now shows capture guidance only while recording is active, reducing persistent instructional noise in the mapping panel
- gesture mapping trigger-modifier editor now uses placeholder semantics for `any/none` states (instead of persisting those labels as editable field values), and macOS-facing shortcut placeholders/labels are normalized to `Cmd` wording
- gesture trigger-modifier input now treats empty as `no modifier` (not `any`), and empty-state placeholder explicitly states this (`optional / empty means no modifier`)
- shortcut recording fallback now includes a short local grace for modifier-first chords, so `Cmd/Ctrl + key` combos are no longer prematurely committed as a lone modifier when native remote capture is delayed/unavailable
- native shortcut capture formatting now treats macOS `meta` and `win` flags as the same primary modifier source, fixing cases where `Cmd+<key>` could be degraded to a plain key token in capture results
- while any shortcut recording session is active, WebUI now installs capture-phase `keydown/keyup/keypress` hard suppression (`preventDefault + stopPropagation + stopImmediatePropagation`) to block page/app shortcut side effects (copy/paste/navigation) across both trigger-modifier and output-shortcut recorders
- local keydown capture now remains active even when remote shortcut-capture polling session exists, so fast modifier+main chords (for example `Cmd+C`) are committed by the focused editor path instead of being dropped behind remote-session timing
- automation quick-template catalog is now platform-aware on the frontend; templates tagged with `platforms` (for example `window_snap` as Windows-only) are filtered out on non-target hosts, avoiding macOS showing Win-specific presets
- gesture mapping preset selection now uses visual preset cards (mini path preview + direction hint arrows), with centered uniform-fit paths, start marker, and larger hollow tangent-aligned end arrowhead (no duplicate end dot); bottom direction-sequence text has been removed to cut visual noise, while direction semantics remain in card tooltip/aria-label; preset/custom mode rendering is state-bound so selecting `custom` shows only the drawing pad, while selecting `preset` shows only preset cards
- gesture preset localization now normalizes the Chinese `backslash` display label to a single `\` (instead of duplicated `\\`) to keep card titles semantically clean.
- gesture preset Chinese naming now removes raw slash glyphs for 4 direction presets (`line_right/line_left/slash/backslash`), using semantic labels (`横线向右/横线向左/斜线右上/斜线右下`) for cleaner card text.
- custom gesture drawing now supports multi-stroke authoring (up to `4` strokes) with per-stroke order badges and direction chips (`#1/#2/...` + arrow summary), plus a limit-tip hint in the editor header
- custom draw pad stroke markers are now aligned with preset semantics: stroke index badge is pinned to each stroke start point, and stroke end arrow is tangent-aligned to the final segment direction
- custom draw pad arrowhead rendering is now open/hollow with larger geometry and tangent lookback stabilization, so end arrows stay visible after commit and keep correct final-direction readability even when tail samples are dense
- gesture custom editor meta values (`stroke count`, `point count`, `similarity threshold`) are now local-draft reactive rather than payload-lag dependent, and threshold summary accepts both camel/snake keys (`matchThresholdPercent` / `match_threshold_percent`)
- gesture custom editor threshold control no longer uses range slider; WebUI now uses numeric input (`50..95`) with mouse-wheel hover increment/decrement to avoid slider track/thumb drift regressions on WebKit
- gesture custom editor metrics layout is now single-row by default (`stroke count | threshold input | point count`) to reduce duplicated labels and vertical noise in the drawing panel
- draw-pad storage/rendering now uses canvas-relative coordinates (`0..100` in panel space) instead of bbox auto-fit, so replay keeps the same paper-like spatial layout rather than stretching to fill the panel
  - gesture editor writes `mode=custom` together with `customStrokes` (UI) and flattened `customPoints` (compat path), and pad/editor both keep local commit state while parent row sync is pending to prevent pointer-up rollback/flicker
  - runtime gesture matching now natively consumes `gesture_pattern.custom_strokes` on both apply-path and config-codec paths; custom gesture bindings are matched by per-stroke similarity (threshold from `match_threshold_percent`) with strict stroke-count/order and trigger-button scope, while `custom_points` remains backward-compatible fallback
  - automation editor hydration is now content-signature based (not prop-reference based), so repeated host renders with semantically identical payload no longer wipe in-progress gesture custom-draw edits
  - automation editor now persists local draft state to browser localStorage and restores it when the incoming payload signature matches the previous baseline, so reopening WebSettings returns to the pre-close editing state without backend changes
  - WebSettings navigation state now persists locally in frontend storage (active section + effects sub-tab + indicator sub-tab) using dual cache (`localStorage` + cookie fallback) to survive restart/origin-port drift; reopen restores last-closed view without depending on URL token/query, and explicit hash-change interaction still takes effect after page load
  - POSIX scaffold websettings now resolves WebUI base dirs with a source-tree-first fallback (unless `MFX_SCAFFOLD_WEBUI_DIR` override is set), reducing dev-time regressions where stale cwd WebUI assets masked freshly built `WebUIWorkspace` output
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

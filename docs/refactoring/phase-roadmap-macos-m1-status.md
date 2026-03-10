# macOS Mainline Roadmap Snapshot (2026-03-02)

## Purpose
Single high-value status page for AI-IDE and reviewers.
This file intentionally excludes low-value historical step logs.

## Scope Baseline
- Primary host and priority: macOS.
- Hard constraints:
  - Windows behavior must not regress.
  - Linux follows compile + contract checks.
  - macOS new work is Swift-first; no Objective-C++ surface expansion.

## Runtime Lanes
- `scaffold` lane: default stable path.
- `core` lane (`mfx_entry_posix_host`): progressive parity path.
- Policy: ship new cross-platform capability through core lane while preserving scaffold stability.

## Phase Status (Plan vs Actual)
- Phase 50: done.
  - Dual-lane guardrails in place.
- Phase 51: done.
  - Core decoupled from Win32-only assumptions for POSIX compile/start baseline.
- Phase 52: done.
  - macOS input capture + permission degrade/recover + indicator baseline complete.
- Phase 53: done (M1 automation scope).
  - App-scope normalization, mapping/injection path, and test-route contracts complete.
- Phase 54: done (Linux follow scope).
  - Compile-level + contract-level gates complete.
- Phase 55: done (WASM runtime baseline in current scope).
  - macOS wasm runtime path, diagnostics, and selfcheck gates complete.
- Phase 56+: ongoing.
  - Behavior parity hardening and token-efficient docs governance.
  - macOS hold-effect conflict handling was corrected so `blend` now keeps the periodic hold-update timer active; stationary long-press effects no longer freeze at the initial 6 o'clock marker while waiting for a move event. `move_only` remains the only policy that suppresses the dedicated hold lane.
  - macOS hold charge/neon overlays now wrap timer-driven layer mutations in a disabled-actions transaction; the orbit-head marker no longer visually cuts through the circle interior due to implicit Core Animation interpolation.
  - macOS hold style builders no longer attach the shared progress-circle lane to `lightning/hex/techRing/hologram/quantumHalo/fluxField`; only `charge` and `neon` retain the explicit outer progress arc/head marker, matching the Windows visual contracts again.
  - macOS input-indicator placement now follows the same top-left screen-space contract as Windows: `relative` offsets and `absolute` coordinates are interpreted with `+Y` pointing down, then translated into Cocoa panel origins only at the presentation boundary. This fixes the previous left-lower placement drift caused by feeding top-left config values directly into bottom-left AppKit window coordinates.
  - macOS input-indicator style parity also moved off the old text-only panel: the Swift bridge now renders a Windows-aligned mouse/keyboard indicator surface directly (mouse shell, left/right/middle highlight states, wheel arrow, key panel), and scroll labels were normalized to `W+ / W-` so style/content both match the Windows indicator contract more closely.
  - macOS input-indicator keyboard labels now share one formatter/streak helper with Windows instead of using the old mac-only fallback path. The visible result is that readable combo tokens and repeat suffixes (`Cmd+Tab`, `Key`, `X x2`) replace opaque `K88`-style fallback labels, while the mac surface still keeps `Cmd` as the user-facing meta-key token.
  - macOS input-indicator mouse clicks/scrolls now share the Windows streak/label rules (`L2/L3`, `W+ 2`), and the Swift indicator view now animates over the same duration window instead of showing only a static frame, reducing click/scroll appearance drift between platforms.
  - WASM general-primitives follow-up now includes `remove_group` as the first retained group lifecycle command, and core HTTP regression raises `max_commands` before the grouped retained sample so the multi-command bundle is not budget-truncated.
  - macOS wasm transient manual-validation hardening exposed and fixed two real host-side issues: `spawn_pulse` had been feeding screen coordinates directly into the overlay pulse renderer, and shared `clip_rect` tails had been interpreted in screen space even after transient/retained lanes had moved into overlay space. The fix now resolves pulse anchors and clip tails through the overlay coordinate service before rendering.
  - retained group semantics now also include `upsert_group_local_origin` plus explicit `...FLAG_USE_GROUP_LOCAL_ORIGIN` opt-in on retained glow/sprite/particle/ribbon/quad lanes, so new grouped retained samples can use one group-local coordinate frame without reinterpreting existing absolute-coordinate samples.
  - retained group semantics now also include `upsert_group_presentation` as the first retained-only group presentation primitive (`alpha_multiplier/visible`), with explicit core HTTP regression coverage via `click-retained-group-alpha`.
  - retained group semantics now also include `upsert_group_clip_rect` as the first retained-only group clip primitive (`instance_clip ∩ group_clip` on retained glow/sprite/particle/ribbon/quad), with explicit core HTTP regression coverage via `click-retained-group-clip`.
  - retained group clip semantics now also accept an optional group-mask tail (`rect|round_rect|ellipse`) on top of that same `upsert_group_clip_rect` surface; macOS retained glow/sprite/particle/ribbon/quad lanes now apply the final effective clip through that shape mask, and core HTTP regression covers it via `click-retained-group-mask`.
  - retained group semantics now also include `upsert_group_layer` as the first retained-only group layer primitive (`blend override/sort bias` on retained glow/sprite/particle/ribbon/quad), with explicit core HTTP regression coverage via `click-retained-group-layer`.
  - retained group semantics now also include `upsert_group_transform` as the first retained-only group transform primitive. v1 keeps host meaning deliberately narrow: translation only (`offset_x_px/offset_y_px`) on retained glow/sprite/particle/ribbon/quad. Template helper `writeUpsertGroupTransform(...)`, sample `click-retained-group-transform`, and the shared POSIX/core wasm contract regression now cover this path.
  - retained group semantics now also include `upsert_group_local_origin` plus explicit `...FLAG_USE_GROUP_LOCAL_ORIGIN` opt-in on retained glow/sprite/particle/ribbon/quad lanes; the first grouped local-origin contract run exposed a parser whitelist regression rather than a runtime design issue, because `WasmCommandBufferParser::IsSupportedKind(...)` was still missing both `upsert_group_transform` and `upsert_group_local_origin`. That parser gate is now aligned with the ABI so `click-retained-group-local-origin` can execute through the normal host dispatch path.
  - retained group semantics now also include `upsert_group_material` with an optional v6 feedback-stack tail on top of the existing style/response/feedback/feedback-mode tails. Host meaning still stays deliberately narrow and retained-only: base v1 keeps `tint override + intensity multiplier`, v2 adds host-owned style presets (`soft_bloom_like` / `afterimage_like` + `style_amount`), v3 adds host-owned `diffusion_amount + persistence_amount`, v4 adds host-owned `echo_amount + echo_drift_px`, v5 adds host-owned `directional|tangential|swirl + phase_rad`, and the new feedback-stack tail only adds host-owned `echo_layers + echo_falloff` that are reapplied to current and future retained glow/sprite/particle/ribbon/quad instances through the existing retained upsert path rather than a new raw rendering API. Template helpers `writeUpsertGroupMaterial(...)` + `writeUpsertGroupMaterialWithStyle(...)` + `writeUpsertGroupMaterialWithStyleAndResponse(...)` + `writeUpsertGroupMaterialWithStyleResponseAndFeedback(...)` + `writeUpsertGroupMaterialWithAllTails(...)` + `writeUpsertGroupMaterialWithFullTails(...)`, sample `click-retained-group-material`, and the shared POSIX/core wasm contract regression now cover this path.
  - retained grouped post-process semantics now also include `upsert_group_pass` as the first dedicated host-owned controlled-pass primitive. This deliberately separates grouped pass updates from `upsert_group_material`: `group_material` stays material-oriented, while `group_pass` stores one retained-only `soft_bloom_like|afterimage_like|echo_like + pass_amount + response_amount` profile per `group_id` and can now append an optional `directional|tangential|swirl + phase_rad` mode tail, an optional `echo_layers + echo_falloff` stack tail, an optional secondary pass pipeline tail (`secondary_pass_kind + secondary_pass_amount + secondary_response_amount`), an optional blend tail (`secondary_blend_mode(multiply|lerp) + secondary_blend_weight`), a routing tail (`secondary_route_mask(glow|sprite|particle|ribbon|quad)`), a per-lane response tail (`secondary_glow|sprite|particle|ribbon|quad_response`), a temporal tail (`phase_rate_rad_per_sec + secondary_decay_per_sec + secondary_decay_floor`), a temporal-mode tail (`exponential|linear|pulse + temporal_strength`), a tertiary-stage tail (`tertiary_pass_kind + tertiary_pass_amount + tertiary_response_amount + tertiary_blend_mode + tertiary_blend_weight`), a tertiary-routing tail (`tertiary_route_mask(glow|sprite|particle|ribbon|quad)`), a tertiary lane-response tail (`tertiary_glow|sprite|particle|ribbon|quad_response`), a tertiary temporal tail (`tertiary_phase_rate_rad_per_sec + tertiary_decay_per_sec + tertiary_decay_floor`), a tertiary temporal-mode tail (`tertiary_temporal_mode + tertiary_temporal_strength`), and now a tertiary stack tail (`tertiary_echo_layers + tertiary_echo_falloff`) before reapplying that state to current/future retained glow/sprite/particle/ribbon/quad instances through the existing retained upsert path. Template helpers now extend through `writeUpsertGroupPassWithTertiaryStackTails(...)`, sample `click-retained-group-pass`, and the shared POSIX/core wasm contract regression continue to cover `create -> pass update -> remove_group`.
  - internal convergence follow-up: the grouped-pass parser/runtime/style path now uses one shared `GroupPassStageState` for secondary and tertiary stages. This keeps the external ABI/sample surface unchanged at v15, but reduces parser/runtime/style drift before the next grouped-pass capability slice.
  - internal convergence follow-up: the grouped-pass parser also now advances through optional tails with one shared reader + one offset cursor instead of maintaining duplicated secondary/tertiary offset ladders. This keeps the external ABI/sample surface unchanged at v15, but lowers the next-slice parser regression risk.
  - retained group transform semantics now also accept a second optional pivot tail (`pivot_x_px/pivot_y_px`) on top of the existing `rotation_rad + uniform_scale` tail without changing the command kind. Current host interpretation stays intentionally staged: translation still applies to all retained glow/sprite/particle/ribbon/quad lanes, while geometry recomposition from the transform tails now applies to retained glow/sprite/particle emitters plus retained ribbon/quad when that lane also opts into `...FLAG_USE_GROUP_LOCAL_ORIGIN`. The shared POSIX/core wasm contract regression now exercises that widened path via `click-retained-group-local-origin`.
  - retained group transform semantics now also accept a third optional non-uniform-scale tail (`scale_x/scale_y`) after the existing `rotation + pivot` tails. Current host interpretation still stays intentionally staged, but grouped retained glow/sprite/particle/ribbon/quad lanes can now recompose around one local pivot with anisotropic scaling while the command kind and base layout stay ABI-compatible.
  - Headless/core HTTP wasm policy control is now hardened: `AppController::HandleCommand(...)` no longer drops JSON commands when no dispatch window exists, so `/api/wasm/policy` mutates the live controller/host budget in posix-host runs; the grouped retained wasm contract also refreshes `/api/state` after `max_commands` restore so later samples do not read a stale pre-restore budget snapshot.

## Current Capability State (macOS)
- Effects:
  - click/trail/scroll/hold/hover paths available in core lane.
  - Type normalization and command-based profile flow active.
  - `trail=none` hard-disable and anti-origin-connector guards active.
  - Effects contract now hard-checks trail mode runtime semantics on macOS overlay probe:
    - `trail=line` must produce observable trail overlay window increase.
    - `trail=none` must not produce trail overlay window increase.
  - Effects contract now hard-checks trail mode command semantics in render profile probe:
    - `trail=line` => command `normalized_type=line` and `emit=true`.
    - `trail=none` => command `normalized_type=none` and `emit=false`.
  - Effects contract now hard-checks command geometry diagnostics:
    - click text sample geometry (`font_size/float_distance`) must be positive.
    - for normalized `line` trail, command line-width must match profile line-width.
  - Effects contract now hard-checks command parity for scroll/hover/hold canonical modes:
    - `scroll=helix` => command emits with helix mode on.
    - `hover=tubes` => command resolves tubes mode on.
    - `hold=hologram` => hold start/update command remains active with normalized hologram semantics.
  - Effects contract now hard-checks alias and fallback command semantics:
    - legacy aliases (`textclick/scifi/stardust/scifi3d/suspension`) map to expected command normalized types and mode flags.
    - `none` fallbacks for click/scroll/hover map to (`ripple/arrow/glow`) with conflicting mode flags disabled.
  - Effects contract now hard-checks hold follow-mode update semantics:
    - smooth mode sample must output deterministic smoothed overlay coordinates on second update.
    - efficient mode sample must satisfy emit throttling cadence (first emit, short-interval suppress, long-interval resume).
  - Effects contract assertion script has been deduplicated to shared helper-based checks (active+type, mode flags, nested command fields) with no behavior contract change.
- Automation:
  - `process:code` / `code.app` / `code.exe` scope semantics normalized.
  - Non-Windows scope persistence now canonicalizes suffix variants to `process:<base>` and enforces legacy `app_scope == app_scopes[0]` parity in automation contract checks.
  - Binding-priority contract now verifies platform-aware normalized selected scope and alias-tie deterministic winner behavior.
  - macOS manual app-scope selfcheck now includes alias-dedupe persistence assertions.
  - App-scope JSON parse helpers are shared across regression/manual scripts for single-source contract parsing behavior.
  - Nested contract scalar parsing moved from regex extraction to JSON path parsing with compatible fallback, reducing false positives under expanding payloads.
  - Injection and matcher contracts script-gated.
- WASM:
  - load/invoke/render/fallback path active.
  - default runtime backend selection is now policy-unified:
    - try `wasm3_static` first, fallback to `dynamic_bridge`, then `null`.
  - Windows build wiring now includes `Wasm3Runtime*` and wasm3 C runtime sources in `MFCMouseEffect.vcxproj`, so unified backend policy can execute in both macOS and Windows lanes.
  - schema vs state capability parity is contract-gated.
  - wasm platform checks now also lock runtime backend expectation (`wasm3_static`) and schema/state invoke-render capability parity in wasm-only contract scope.
  - command surface now includes `spawn_path_stroke`, `spawn_path_fill`, retained `sprite_emitter`, plus the shared render-semantics tail on path-stroke/path-fill/glow-batch/sprite-batch/retained glow-emitter/sprite-emitter commands.
  - macOS wasm contract regression now explicitly covers the `path-stroke -> retained-glow upsert/remove` sequence; the retained glow window ownership path was hardened so right-click remove no longer crashes after a prior path-stroke dispatch.
  - retained sprite emitter v1 is now available on macOS/Windows host paths: `upsert_sprite_emitter/remove_sprite_emitter`, image-asset aware with tinted fallback when the image asset is missing.
  - core HTTP wasm contract/sample regression now explicitly covers `click-retained-sprite-fountain`, including retained sprite-emitter upsert/remove counters plus active-count reset after right-click remove.
  - fixed a macOS dispatch regression where retained sprite-emitter commands parsed successfully but were dropped in the platform render switch; `click-retained-sprite-fountain` now renders and reports `rendered_any=true` in core HTTP wasm contracts.
  - glow/sprite retained-emitter runtimes now share one C++ helper for store/key/prune/reset/counter plumbing; emitter-specific render/update code stays separate, but lifecycle scaffolding is no longer duplicated.
  - retained particle emitter v1 is now available on macOS/Windows host paths: `upsert_particle_emitter/remove_particle_emitter`, with `soft_glow` / `solid_disc` styles sharing the same retained lifecycle and render-semantics tail.
  - retained particle emitter now also accepts an optional dynamics tail for `drag/turbulence` in addition to the spawn tail (`cone|radial`, `point|disc|ring`) and curve tail for `size/alpha/color over life`; the sample/template route uses it to demonstrate ring-like radial glows that keep drifting instead of reading as fixed-rate bursts, again without changing the fixed command layout.
  - `spawn_quad_batch` is now available on macOS/Windows host paths as the first `quad + atlas` baseline: explicit per-item `width/height`, optional normalized `src_u/v` rects, and shared render-semantics tail support, while reusing the existing sprite-batch diagnostics lane and transient overlay path.
  - `spawn_ribbon_strip` is now available on macOS/Windows host paths as the first transient ribbon/trail baseline: a centerline point stream with per-point width, resolved by the host into a filled strip and rendered through the existing path-fill lane plus shared render-semantics tail support.
  - the same macOS path lane now also accepts an optional second `clip_rect` tail after the shared render-semantics tail for `spawn_path_stroke`, `spawn_path_fill`, and `spawn_ribbon_strip`; phase1 support is intentionally scoped to screen-space rectangular clipping on macOS path overlays, with template samples `click-path-fill-clip-window` plus `click-path-clip-lanes` now covering all three path-lane clip helpers.
  - `clip_rect` phase2 extends the same optional tail to `spawn_sprite_batch`, `spawn_quad_batch`, and `upsert_quad_field`; macOS transient sprite/quad overlays plus retained quad-field now mask to a screen-space rectangle, and the existing sample/regression set (`click-sprite-burst`, `click-quad-atlas-burst`, `click-retained-quad-field`) now exercises those clipped lanes without introducing new sample ids.
  - `clip_rect` phase3 extends the same optional tail to retained emitter lanes (`upsert_glow_emitter`, `upsert_sprite_emitter`, `upsert_particle_emitter`); macOS retained glow/sprite/particle overlays now mask to a screen-space rectangle, and the existing sample/regression set (`click-retained-glow-field`, `click-retained-sprite-fountain`, `click-retained-particle-field`) now exercises those clipped lanes without introducing new sample ids.
  - retained ribbon trail v1 is now available on macOS/Windows host paths: `upsert_ribbon_trail/remove_ribbon_trail`, using a centerline+width point stream that the host keeps as a retained strip; template sample and core HTTP wasm contract regression both cover upsert/remove and active-count reset.
  - retained quad field v1 is now available on macOS/Windows host paths: `upsert_quad_field/remove_quad_field`, using a retained `quad_batch`-derived cluster with per-item atlas rects plus shared ttl motion; template sample and core HTTP wasm contract regression both cover upsert/remove and active-count reset.
  - macOS retained-window ownership was hardened after the new retained quad-field lane exposed a delayed ribbon-trail `NSWindow` over-release; the actual root fix was to keep retained ribbon-trail windows non-self-releasing (`isReleasedWhenClosed = false`), and the full wasm contract regression is now green again after retained quad-field cleanup.
- Permissions:
  - runtime revoke -> degrade + notify.
  - runtime regrant -> hot recovery without restart.

## Regression Gates (Canonical)
- Scaffold regression:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-scaffold-regression.sh --platform auto`
- Effects contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-effects-contract-regression.sh --platform auto`
- Automation contract:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-core-automation-contract-regression.sh --platform auto`
- WASM suite:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-posix-wasm-regression-suite.sh --platform auto`
- macOS ObjC++ surface gate:
  - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/regression/run-macos-objcxx-surface-regression.sh`

## Manual Selfcheck Entrypoints (macOS)
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/mfx`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-core-websettings-manual.sh --auto-stop-seconds 60`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-effects-type-parity-selfcheck.sh --skip-build`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-automation-injection-selfcheck.sh --skip-build`
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-macos-wasm-runtime-selfcheck.sh --skip-build`
- Manual core-host startup now validates `/api/state` readiness and fails fast when a scaffold-lane binary is accidentally reused via `--skip-build`.

## Active Supporting Docs (Small Set)
- `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/wasm-plugin-abi-v3-design.zh-CN.md`

## Completion Criteria for Current Milestone
- macOS behavior parity acceptable for main user-facing paths (effects/automation/wasm/permissions).
- Windows no-regression verified by existing gates.
- Linux compile + contract follow remains green.
- No uncontrolled growth in P0/P1 docs.

## Notes
- Historical granular phase logs were intentionally removed from this file to keep first-read token usage low.
- Phase52-54 closure details are retained in git history and regression scripts; separate phase documents were removed to reduce stale-doc maintenance.
- Old one-off phase and bugfix notes with no active contract ownership were deleted from `/docs` during doc hygiene cleanup.
- If deep history is required, use git history on this file and prior commits.

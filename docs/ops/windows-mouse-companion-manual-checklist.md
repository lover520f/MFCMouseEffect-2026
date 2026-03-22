# Windows Mouse Companion Manual Checklist

## Purpose
- Provide a compact manual regression checklist for the current Windows `Mouse Companion` Phase1.5 path.
- Keep verification focused on the real visible contract:
  - host lifecycle
  - placement/clamp
  - action semantics
  - asset status plumbing

## Scope
- Target platform: Windows
- Build target: `Release|x64`
- Current renderer scope:
  - transparent layered host window
  - placeholder presenter/renderer path
  - non-GPU action/pose/appearance consumption

## Build Gate
1. Build [MFCMouseEffect.vcxproj](/f:/language/cpp/code/MFCMouseEffect/MFCMouseEffect/MFCMouseEffect.vcxproj) with `Release|x64`.
2. Confirm output exists at [MFCMouseEffect.exe](/f:/language/cpp/code/MFCMouseEffect/x64/Release/MFCMouseEffect.exe).

## Smoke Path
1. Launch the app.
2. Enable `Mouse Companion` from Web settings.
3. Confirm a visible pet window appears.
4. Disable `Mouse Companion`.
5. Confirm the pet window disappears immediately.

## Placement
1. Set `position_mode = relative`.
2. Before moving the mouse, enable the pet and confirm first-show placement is near the target monitor center, not `(0,0)`.
3. Move the mouse and confirm the pet follows relative placement offsets.
4. Set `position_mode = absolute` and change `absolute_x / absolute_y`.
5. Confirm the pet moves to the expected absolute screen position.
6. Set `position_mode = fixed_bottom_left`.
7. Confirm the pet anchors to the lower-left area of the target monitor.

## Clamp
1. Set `edge_clamp_mode = strict` and push the pet near screen edges.
2. Confirm the full pet stays inside the monitor bounds.
3. Set `edge_clamp_mode = soft`.
4. Confirm partial off-screen placement is allowed but the pet remains visibly present.
5. Set `edge_clamp_mode = free`.
6. Confirm clamping is no longer enforced.

## Monitor Routing
1. On a multi-monitor setup, switch `target_monitor`.
2. Confirm `absolute` and fallback `relative` placement both resolve against the selected monitor.

## Action Semantics
1. Idle without moving the mouse.
2. Confirm the pet stays in the idle silhouette.
3. Move the mouse continuously.
4. Confirm `follow` becomes visibly distinct from idle.
5. While moving left/right, confirm the pet shows a stable left/right facing tendency instead of staying rigidly front-facing or jitter-flipping on tiny pointer shakes.
6. Confirm the near-side ear/limb reads slightly more forward than the far side, so follow/drag does not look completely flat.
7. During idle/follow, confirm the pet has light procedural life: subtle bob, ear sway, paw cadence, and shadow/tail response instead of remaining completely rigid.
8. Click repeatedly.
9. Confirm `click_react` is visible and stronger than idle/follow.
10. Press and drag.
11. Confirm `drag` silhouette differs from `click_react`.
12. Hold the pointer steady with button pressed.
13. Confirm `hold_react` appears.
14. Scroll in both directions.
15. Confirm `scroll_react` appears and body lean changes direction.
16. Click repeatedly and confirm there is a short impact-and-rebound feel, not only a static squash pose.
17. Hold steadily and confirm the body compression settles into a gentle press pulse instead of flickering.
18. Scroll in short bursts and confirm the directional kick decays after the burst instead of staying latched.
19. During click/hold/scroll, confirm face/chest details respond too: whiskers spread slightly, cheeks lift, mouth opens a bit more, and chest motion remains subtle instead of noisy.
20. During stronger reactions, confirm pupils/brows also change slightly so the face reads more focused rather than remaining a fixed sticker.
21. During follow/drag/scroll, confirm low-cost motion accents remain tasteful: collar/charm stay attached cleanly and dust cues are subtle, not noisy or always-on.
22. During follow/drag, confirm head/body/limbs read as one connected creature: neck bridge, limb bridge lines, and back/hip accents should improve structure without making the placeholder look busy.
23. If appearance accessories are enabled, confirm the head accessory resolves consistently and does not jump to a random place while moving.
24. Confirm material/detail accents stay coherent across variants: tail tip, paw pads, body/head fills, and accessory colors should change together instead of looking like unrelated hardcoded colors.
25. During follow/drag, confirm ear, tail, and front/rear limbs feel rhythmically related rather than moving as unrelated parts; the timing should read as one creature, not several independent stickers.
26. Confirm silhouette anchor nodes improve readability without noise: ear roots, tail root, shoulder/hip patches, and front/rear depth patches should make the pet easier to read, not busier.
27. Compare `idle / follow / drag / hold` directly and confirm the base stance changes too: body center, head anchor, ear spread, and fore/rear stance width should differ even before micro-motion and reaction pulses are noticed.
28. Compare `click / hold / scroll / drag / follow` and confirm the silhouette profile changes too: chest lift/width, tail-root attitude, shoulder-vs-hip emphasis, and front/rear depth patches should not all look like the same creature with only timing differences.

## Pose / Action Library
1. Keep default `pet-actions.json` available.
2. Confirm runtime status reports action-library loaded.
3. Trigger click/scroll/follow again.
4. Confirm placeholder motion still reacts even without a real 3D model.
5. If pose samples are flowing, confirm ear/hand/leg motion differs from pure action-label fallback.

## Appearance
1. Keep default `pet-appearance.json` available.
2. Confirm runtime status reports appearance-profile loaded.
3. Change appearance profile content if needed and reload settings.
4. Confirm base color / accessory accent changes are reflected by the placeholder.

## Runtime Diagnostics
Check `/api/state.mouse_companion_runtime` or equivalent diagnostics snapshot and confirm:
- `visual_host_active`
- `model_loaded`
- `action_library_loaded`
- `appearance_profile_loaded`
- `pose_frame_available`
- `preferred_renderer_backend_source`
- `preferred_renderer_backend`
- `selected_renderer_backend`
- `renderer_backend_selection_reason`
- `renderer_backend_failure_reason`
- `available_renderer_backends`
- `unavailable_renderer_backends`
- `renderer_backend_catalog`
- `real_renderer_unmet_requirements`
- `real_renderer_preview`
- `renderer_runtime_backend`
- `renderer_runtime_ready`
- `renderer_runtime_frame_rendered`
- `renderer_runtime_frame_count`
- `renderer_runtime_last_render_tick_ms`
- `renderer_runtime_action_name`
- `renderer_runtime_reactive_action_name`
- `renderer_runtime_model_ready`
- `renderer_runtime_action_library_ready`
- `renderer_runtime_appearance_profile_ready`
- `renderer_runtime_pose_frame_available`
- `renderer_runtime_pose_binding_configured`
- `renderer_runtime_model_source_format`
- `renderer_runtime_surface_width`
- `renderer_runtime_surface_height`
- `renderer_runtime_before`
- `renderer_runtime_after`
- `renderer_runtime_delta`
- `renderer_runtime_wait_for_frame_ms`
- `renderer_runtime_expect_frame_advance`
- `renderer_runtime_expectation_met`
- `renderer_runtime_expectation_status`
- `configured_renderer_backend_preference_effective`
- `configured_renderer_backend_preference_status`
- `loaded_model_path`
- `loaded_action_library_path`
- `loaded_appearance_profile_path`
- `model_load_error`
- `action_library_load_error`
- `appearance_profile_load_error`

Compact proof route:
- `POST /api/mouse-companion/test-render-proof`
- returns:
  - `renderer_runtime_before`
  - `renderer_runtime_after`
  - `renderer_runtime_delta`
  - `renderer_runtime_expectation_met`
  - `renderer_runtime_expectation_status`
  - `selected_renderer_backend`
  - `real_renderer_preview`
- intended use:
  - frame-advance verification when you do not need the full `runtime` and `action_coverage` payload from `/api/mouse-companion/test-dispatch`

## Test-Friendly Backend Preference
Current Windows pet backend preference is intentionally test-friendly and does not require a schema change yet.
The current config-backed preference seam is internal-only for now; manual validation still uses env override as the public test path.
Hidden `config.json` fields now persist backend preference source/name, but they are not yet part of the supported settings UI contract.
Those hidden fields now also round-trip through settings-state output and apply-settings payloads, even though the current UI does not render them.
If those hidden fields change while the host is already active, the Windows visual host now attempts an in-place backend reselection instead of waiting for a full restart.

1. Leave `MFX_WIN32_MOUSE_COMPANION_RENDERER_BACKEND` unset.
2. Launch the app and confirm runtime diagnostics report:
   - `preferred_renderer_backend_source = default`
   - `preferred_renderer_backend = auto`
   - `configured_renderer_backend_preference_status = not_configured`
3. Set:
   - `MFX_WIN32_MOUSE_COMPANION_RENDERER_BACKEND=placeholder`
4. Relaunch the app and confirm runtime diagnostics now report:
   - `preferred_renderer_backend_source = env:MFX_WIN32_MOUSE_COMPANION_RENDERER_BACKEND`
   - `preferred_renderer_backend = placeholder`
   - if hidden config preference fields are also populated, `configured_renderer_backend_preference_status = overridden_by_env`
5. Set:
   - `MFX_WIN32_MOUSE_COMPANION_RENDERER_BACKEND=default`
6. Relaunch the app and confirm runtime diagnostics now report:
   - `preferred_renderer_backend_source = env:MFX_WIN32_MOUSE_COMPANION_RENDERER_BACKEND`
   - `preferred_renderer_backend = auto`
   - if hidden config preference fields are also populated, `configured_renderer_backend_preference_status = overridden_by_env`
7. If a future experimental backend name is forced but not registered, confirm:
   - `renderer_backend_selection_reason` explains the fallback
   - `renderer_backend_failure_reason` is non-empty
   - `selected_renderer_backend` still resolves to the effective fallback backend
8. If a future backend is registered but currently unavailable on the machine/runtime, confirm:
   - `unavailable_renderer_backends` contains an entry in the form `backend_name:reason`
   - `available_renderer_backends` still lists only selectable backends
   - `renderer_backend_selection_reason` reports either direct fallback or unavailable-preferred fallback semantics
   - `renderer_backend_catalog` includes both available and unavailable entries with explicit `priority` and `unavailable_reason`
9. Current baseline expectation after this refactor:
   - by default `unavailable_renderer_backends` should include `real:rollout_disabled`
   - `available_renderer_backends` should still include `placeholder`
   - `renderer_backend_catalog` should contain at least `real` and `placeholder` entries in priority order
   - the `real` catalog entry should now carry an empty unmet-requirements list
   - top-level `real_renderer_unmet_requirements` should also be empty
   - this list should no longer include `scene_runtime_adapter`; if it reappears, treat it as a regression in the real-renderer readiness seam
   - this list should no longer include `asset_resource_adapter`; if it reappears, treat it as a regression in the real-renderer readiness seam
   - this list should no longer include `renderer_draw_execution`; if it reappears, treat it as a regression in the real-renderer draw seam
   - default selection should continue to resolve to the placeholder path until a real backend becomes available
10. Hidden rollout-gate validation:
   - set `MFX_WIN32_MOUSE_COMPANION_REAL_RENDERER_ENABLE=1`
   - optionally set `MFX_WIN32_MOUSE_COMPANION_RENDERER_BACKEND=real`
   - for test dispatch, you can also send:
     - `wait_for_frame_ms`
       - default: `0`
       - recommended test value: `120`
     - `expect_frame_advance`
       - default: `false`
       - recommended test value: `true`
   - expected diagnostics:
     - `preferred_renderer_backend=real` when explicit backend env is set
     - `selected_renderer_backend=real` only when rollout gate is enabled
     - `renderer_backend_catalog` entry for `real` should flip to `available=true`
     - `real_renderer_preview.rollout_enabled=true`
     - `real_renderer_preview.preview_selected=true`
     - `real_renderer_preview.preview_active=true` after the visual host is active
     - `real_renderer_preview.action_name` should track the latest runtime action
     - `real_renderer_preview.reactive_action_name` should reflect the current reactive lane
     - `real_renderer_preview.pose_binding_configured` and `real_renderer_preview.pose_frame_available` should reflect whether the preview path is really receiving pose data
     - `real_renderer_preview.model_ready / action_library_ready / appearance_profile_ready` should expose the three asset lanes directly
     - `renderer_runtime_backend=real`
     - `renderer_runtime_frame_rendered=true` after at least one frame
     - repeated dispatch events should increase `renderer_runtime_frame_count`
     - repeated dispatch events should advance `renderer_runtime_last_render_tick_ms`
     - `renderer_runtime_action_name` and `renderer_runtime_reactive_action_name` should follow the last renderer-fed runtime state
     - `renderer_runtime_surface_width` / `renderer_runtime_surface_height` should stay non-zero while the preview window is active
     - `renderer_runtime_before / after / delta` should show the transition in a single response, and `renderer_runtime_delta.frame_count_delta` should be positive once a new render is produced
     - when `expect_frame_advance=true`, the route should return:
       - `renderer_runtime_expectation_met=true`
       - `renderer_runtime_expectation_status=frame_advanced`
   - for compact proof-only checks, call `/api/mouse-companion/test-render-proof` with the same `wait_for_frame_ms` and `expect_frame_advance` parameters
     - expected compact result:
       - `selected_renderer_backend=real`
       - `real_renderer_preview.preview_active=true`
       - `renderer_runtime_delta.frame_count_delta > 0` once a new frame is observed
       - when `expected_backend` is provided, `backend_expectation_met=true`
       - when `expect_preview_active=true`, `preview_expectation_met=true`
       - `all_expectations_met=true` once frame/backend/preview checks all pass
   - for compact multi-step bring-up checks, call `/api/mouse-companion/test-render-proof-sweep`
     - expected compact result:
       - response `event=render_proof_sweep`
       - response `summary.all_expectations_met=true`
       - when `expected_backend` is provided, `summary.all_backend_expectations_met=true`
       - when `expect_preview_active=true`, `summary.all_preview_expectations_met=true`
       - response `results` includes at least:
         - `status`
         - `click`
         - `hold_start`
         - `scroll`
         - `move`
         - `hold_end`
       - each result keeps the same `renderer_runtime_before / after / delta` shape as the single proof route
       - the action results that request frame advance should report:
         - `renderer_runtime_expectation_met=true`
         - `renderer_runtime_expectation_status=frame_advanced`
   - for a reusable Git Bash entry on Windows, run:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-windows-mouse-companion-render-proof.sh --base-url <url> --token <token> --route sweep`
     - single-event proof is also supported:
       - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-windows-mouse-companion-render-proof.sh --base-url <url> --token <token> --route proof --event click`
     - when validating the gated real preview path, prefer:
       - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-windows-mouse-companion-render-proof.sh --base-url <url> --token <token> --route sweep --expected-backend real --expect-preview-active true`
       - or use the shortest preset form:
         - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-windows-mouse-companion-render-proof.sh --base-url <url> --token <token> --preset real-preview-smoke`
     - the script now exits non-zero if proof expectations are missed, so it can be used as a simple bring-up gate instead of only a logging helper
   - expected visual boundary:
     - current `real` backend still looks like a preview renderer, not macOS SceneKit parity
     - but it should now render a stylized pet silhouette with visible ears/limbs/face plus asset-lane badges, rather than only an abstract diagnostics card
     - action changes should now be visually distinguishable even before true-model rendering lands:
       - `click` should show a ring overlay near the head/body center
       - `hold` should show a grip-band style overlay between the hands
       - `scroll` should show an orbit arc around the body
       - `follow` should show a short trailing-motion overlay
       - `drag` should show a directional motion slash
       - action changes should also carry a mild theme tint:
         - `click` should read warmer/pinker
         - `hold` should read warmer/golden
         - `scroll` should read cooler/cyan
         - `drag` should read more violet
         - `follow` should read fresher/green-cyan
       - the face should now react too:
         - `click` should read as the happiest/openest expression
         - `hold` should look more concentrated/tense
         - `scroll` should show an offset mouth/brow expression
       - the eyes should no longer read as static stickers:
         - pupils should bias slightly toward facing/action direction
         - eye highlights should feel brighter in `click/follow` and more restrained in `hold`
         - `follow` should look more alert
       - whole-body posture should now react too:
         - `click` should feel slightly lifted/lightened
         - `hold` should feel lower and more planted
         - `follow` should feel forward/alert with more kinetic cadence
         - `scroll` should show a tilted, slightly orbiting stance
         - `drag` should lean in the drag direction more obviously
       - `idle` should no longer look frozen:
         - slight breathing should be visible in body/shadow rhythm
         - ears should keep a small cadence
         - tail should sway subtly

## Current Expected Boundary
- `model_loaded` may still be `false` on Windows because the current Windows path does not render the real 3D model yet.
- This is not a regression by itself as long as:
  - visible placeholder host works
  - action/pose/appearance lanes still feed the placeholder path

## Regression Rule
- Any future Windows pet change must re-run at least:
  - `Build Gate`
  - `Smoke Path`
  - `Placement`
  - `Action Semantics`
  - `Runtime Diagnostics`

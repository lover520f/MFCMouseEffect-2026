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
- `loaded_model_path`
- `loaded_action_library_path`
- `loaded_appearance_profile_path`
- `model_load_error`
- `action_library_load_error`
- `appearance_profile_load_error`

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

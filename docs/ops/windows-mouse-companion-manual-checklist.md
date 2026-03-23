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
6. If `scene_runtime_adapter_mode=pose_bound`, confirm ear/hand/leg pose offsets read stronger than `pose_unbound`; `pose_unbound` should still move, but more softly.
7. If `scene_runtime_adapter_mode=pose_bound`, confirm head/body expression also shifts a bit more than `pose_unbound`: slight forward attitude, head nod, and eye/brow focus should feel more pose-led instead of purely action-led.
8. With `pose_bound`, confirm face placement details also move a little more with the pose lane: eye/nose/mouth anchor, blush height, and whisker spread/tilt should read slightly more pose-driven than `pose_unbound`, but still remain stable and not noisy.
9. With `pose_bound`, confirm accessory and overlay staging also feel more tied to the pose lane: pose badge should stay visible on adapter-aware frames, accessory anchor should drift slightly with the same head/hand bias, and click/hold/scroll/drag/follow overlays should inherit a small pose-led center offset instead of remaining perfectly action-only.
10. With `pose_bound`, confirm grounding also changes a little: shadow and pedestal should shift/scale subtly with the bound leg lane, while `pose_unbound` remains softer and `runtime_only` stays closest to the old pure-action baseline.
11. With `pose_bound`, confirm painter readability also tightens slightly: pose badge should feel a bit more confident, accessory stroke/fill should read a touch stronger, and shadow/pedestal should look a little more grounded than `pose_unbound`, without turning into a new flashy effect layer.
12. In runtime/proof output, confirm `scene_runtime_pose_adapter_brief` and the numeric `scene_runtime_pose_adapter_influence / scene_runtime_pose_readability_bias` agree with the visible three-tier read: `runtime_only` lowest, `pose_unbound` softer middle, `pose_bound` strongest.

## Appearance
1. Keep default `pet-appearance.json` available.
2. Confirm runtime status reports appearance-profile loaded.
3. Change appearance profile content if needed and reload settings.
4. Confirm base color / accessory accent changes are reflected by the placeholder.

## Appearance Combo Persona
1. Keep default `pet-appearance.json` available and confirm Windows real preview is active.
2. Validate these three high-value combinations first:
   - `activePreset = "cream-moon"`
   - `activePreset = "night-leaf"`
   - `activePreset = "strawberry-ribbon-bow"`
3. Static-frame expectation:
   - `cream + moon` should read lighter/softer/dreamier than the other two.
   - `night + leaf` should read tighter/more directional/more agile than the other two.
   - `strawberry + ribbon-bow` should read sweeter/brighter/more charming than the other two.
4. Dynamic expectation:
   - `cream + moon` should feel lighter during `follow / scroll`; glow and lift should read slightly more airy than baseline.
   - `night + leaf` should feel sharper during `drag / scroll`; ear, tail, whisker, and grounding direction should read more decisive than baseline.
   - `strawberry + ribbon-bow` should feel more buoyant during `click / hold`; blush/accent/overlay emphasis should read sweeter and springier than baseline.
   - latest Windows real-preview tuning also expects:
     - `cream + moon`: `follow` should sit a bit higher with softer shadow/pedestal weight and gentler tail swing
     - `night + leaf`: `drag` should show a stronger forward/reach posture and tighter eye/brow focus
     - `strawberry + ribbon-bow`: `hold/click` should show slightly fuller blush/mouth response and a rounder upbeat bounce
5. Combination pass criteria:
   - differences must not reduce to accessory shape only
   - differences must not reduce to palette only
   - at least two of these layers should differ together:
     - face mood
     - body/appendage silhouette
     - action rhythm
     - glow/shadow/pedestal mood
6. If the combination is only weakly readable in a static frame but clearly readable during action changes, mark it as:
   - `pass (dynamic-biased)`
7. If the combination still feels indistinguishable after `follow / hold / click / drag / scroll`, mark it as:
   - `fail (persona readability too weak)`
8. Fastest Windows entry:
   - `D:\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-combo-persona-acceptance.cmd`
   - this first runs the real-preview smoke gate, then switches runtime `appearance_profile_path` across the three synced combo-only JSON files and validates each case
9. Runtime handoff discovery note:
   - in `Release`, the app normally writes `websettings_runtime_auto.json` under `%APPDATA%\MFCMouseEffect`
   - in `Debug`, config-path fallback may place the same file near the built exe instead (for example `F:\language\cpp\code\MFCMouseEffect\x64\Debug\websettings_runtime_auto.json`)
   - the Windows-native proof/acceptance scripts now check both locations automatically
10. Machine-check note:
   - after switching `activePreset`, verify `/api/state` or `/api/mouse-companion/test-render-proof` also reflects:
     - `appearance_requested_preset_id`
     - `appearance_resolved_preset_id`
     - `appearance_skin_variant_id`
     - `appearance_accessory_family`
     - `appearance_combo_preset`
     - `appearance_plugin_id`
     - `appearance_plugin_kind`
     - `appearance_plugin_source`
     - `appearance_plugin_selection_reason`
     - `appearance_plugin_failure_reason`
     - `appearance_plugin_manifest_path`
     - `appearance_plugin_runtime_backend`
     - `appearance_plugin_metadata_path`
     - `appearance_plugin_metadata_schema_version`
   - if `requested` and `resolved` differ, treat it as fallback behavior first, not immediate renderer drift
   - if `appearance_plugin_kind=builtin` while the env explicitly requested `wasm`, treat `appearance_plugin_selection_reason` + `appearance_plugin_failure_reason` as the authoritative fallback explanation before judging visual drift
   - if `appearance_plugin_metadata_path` is empty, the host is still running on manifest-only compatibility mode rather than the renderer sidecar contract
   - if sidecar exists and is valid, `appearance_plugin_appearance_semantics_mode` should currently report either:
     - `builtin_passthrough`
     - `wasm_v1`
   - runtime diagnostics now also expose default-lane decision state:
     - `default_lane_candidate`
     - `default_lane_source`
     - `default_lane_rollout_status`
     - `default_lane_style_intent`
     - `default_lane_candidate_tier`
   - current interpretation:
     - `default_lane_rollout_status = stay_on_builtin` means runtime still recommends keeping builtin as shipped default
     - `default_lane_rollout_status = candidate_pending_manual_confirmation` means runtime sees a non-builtin candidate, but the lane is still not approved for default rollout until observation says so
   - common `default_lane_source` readings:
     - `runtime_builtin_default`: no stronger lane is currently nominated
     - `env_builtin_forced`: runtime is intentionally pinned to builtin
     - `env_wasm_candidate`: wasm lane is healthy enough to be nominated as a candidate
     - `env_wasm_fallback_builtin`: wasm was requested, but runtime stayed on builtin after fallback
     - `runtime_plugin_candidate`: a non-builtin candidate exists through runtime plugin selection even without an explicit env-wasm request path
   - common `default_lane_style_intent` readings:
     - `style_candidate:none`
     - `style_candidate:builtin_passthrough_baseline`
     - `style_candidate:balanced_default_candidate`
     - `style_candidate:agile_follow_drag`
     - `style_candidate:dreamy_follow_scroll`
     - `style_candidate:charming_click_hold`
   - common `default_lane_candidate_tier` readings:
     - `builtin_shipped_default`: runtime still treats builtin as shipped default
     - `baseline_reference_candidate`: non-builtin lane is only baseline/reference quality
     - `ship_default_candidate`: current candidate matches the balanced ship-default tier
     - `experimental_style_candidate`: current candidate is intentionally experimental
   - checked-in sidecars now declare `style_intent` explicitly; when a non-builtin lane is active, runtime should prefer that declared intent instead of only deriving from combo preset
   - checked-in sidecars now also declare `sample_tier`:
     - `ship_default_candidate`: current balanced default sample
     - `experimental_style_candidate`: agile / dreamy / charming
   - runtime now surfaces the same value as `appearance_plugin_sample_tier`, so Win-side validation can tell “ship default candidate” from “experimental style candidate” without reopening the sidecar file
   - runtime now also surfaces `appearance_plugin_contract_brief = semantics_mode/style_intent/sample_tier`, so the current sidecar contract can be skimmed from one short field
   - runtime now also surfaces scene-runtime adapter progress:
     - `scene_runtime_adapter_mode = runtime_only|pose_unbound|pose_bound`
     - `scene_runtime_pose_sample_count`
     - `scene_runtime_bound_pose_sample_count`
   - practical reading:
     - `runtime_only`: renderer runtime is still only consuming action/runtime lanes
     - `pose_unbound`: pose samples are present, but binding is not configured yet
     - `pose_bound`: the current frame is already on the bound-pose lane
  - runtime / proof now also surface `scene_runtime_model_scene_adapter_state`, `scene_runtime_model_scene_seam_readiness`, and `scene_runtime_model_scene_adapter_brief`; current expected progression is:
    - `preview_only/unknown/runtime_only` or `preview_only/phase1_placeholder/runtime_only`
    - `asset_stub_ready/<format>/runtime_only`
    - `pose_stub_ready/<format>/pose_unbound`
    - `pose_bound_preview_ready/<format>/pose_bound`
  - runtime / proof / lane matrix now also surface `scene_runtime_model_node_adapter_influence` and `scene_runtime_model_node_adapter_brief`; expected reading is:
    - `preview_only/0.00` before builders consume the shared node seam
    - `asset_stub_ready/<small>` while asset seam is present but pose influence is still reduced
    - `pose_stub_ready/<mid>` once pose samples are feeding the shared node seam
    - `pose_bound_preview_ready/<largest>` once frame/face/adornment/overlay/grounding are all reading the same bound node offsets
  - `scene_runtime_model_node_channel_brief` should now also move from all-zero toward non-zero channel weights; expected relative emphasis is usually:
    - `body` and `face` first
    - then `appendage` / `overlay`
    - `grounding` slightly softer than fully bound body/face
  - `scene_runtime_model_node_binding_brief` should now also move through:
    - `preview_only/0/0`
    - `binding_scaffold/5/<low>`
    - `binding_stub_ready/5/<mid>`
    - `binding_ready/5/<high>`
  - `scene_runtime_model_node_binding_weight_brief` should stay near all-zero in `runtime_only`, rise into sub-1.0 weights for `pose_unbound`, and approach fuller `body/head/appendage/overlay/grounding` weights once `pose_bound` is active
  - `scene_runtime_model_node_slot_brief` should now also move through:
    - `preview_only/0/0`
    - `slot_scaffold/5/<low>`
    - `slot_stub_ready/5/<mid>`
    - `slot_binding_ready/5/<high>`
  - `scene_runtime_model_node_slot_name_brief` should stay stable across lanes; it is the future asset-node vocabulary seam, so `body_root / head_anchor / appendage_anchor / overlay_anchor / grounding_anchor` should not drift between runtime, proof, and WebUI
  - `scene_runtime_model_node_registry_brief` should now also move through:
    - `preview_only/0/0`
    - `registry_scaffold/5/<low>`
    - `registry_stub_ready/5/<mid>`
    - `registry_binding_ready/5/<high>`
  - `scene_runtime_model_node_registry_asset_node_brief` should stay stable across lanes; it is the next seam after slots, so `asset.body.root / asset.head.anchor / asset.appendage.anchor / asset.overlay.anchor / asset.grounding.anchor` should not drift between runtime, proof, lane matrix, and WebUI
  - `scene_runtime_model_node_registry_weight_brief` should remain near all-zero in `runtime_only`, then rise along with bound slots; frame grounding, accessory readability, and overlay emphasis should now subtly strengthen with that registry-weight seam instead of only raw binding weight
  - `scene_runtime_asset_node_binding_brief` should now also move through:
    - `preview_only/0/0`
    - `asset_binding_scaffold/5/<low>`
    - `asset_binding_stub_ready/5/<mid>`
    - `asset_binding_ready/5/<high>`
  - `scene_runtime_asset_node_binding_path_brief` should stay stable across lanes; it is the future model-path seam, so `/pet/body/root / /pet/body/head / /pet/body/appendage / /pet/fx/overlay / /pet/fx/grounding` should not drift between runtime, proof, lane matrix, and WebUI
  - `scene_runtime_asset_node_binding_weight_brief` should stay near zero in `runtime_only`, rise in `pose_unbound`, and become stronger in `pose_bound`; frame grounding, head/body anchor bias, accessory readability, and overlay readability should all now reflect this extra binding-table seam
   - `render-proof` can now pin pose-adapter state directly with `-ExpectedSceneRuntimeAdapterMode`, `-ExpectedSceneRuntimePoseAdapterBrief`, `-ExpectedSceneRuntimePoseAdapterInfluenceMin`, and `-ExpectedSceneRuntimePoseReadabilityBiasMin`
   - `render-proof` and lane matrix now also echo `default_lane_candidate_tier`, so `ship_default_candidate` vs `experimental_style_candidate` no longer needs to be inferred manually from raw sample metadata
   - smoke expectation now also pins that value:
     - `renderer-sidecar-smoke` -> `baseline_reference`
     - `renderer-sidecar-wasm-v1-smoke(default)` -> `ship_default_candidate`
     - `renderer-sidecar-wasm-v1-smoke(agile|dreamy|charming)` -> `experimental_style_candidate`
   - native shortcut:
     - `D:\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-render-proof.cmd -Route proof -Event status -ExpectAppearanceProfileMatch $true`
   - `run-windows-mouse-companion-combo-persona-acceptance.cmd` now enables the same appearance-profile machine check automatically and uses dedicated combo-only appearance JSON files instead of asking Win-side validation to edit the synced main profile
11. Optional wasm skeleton smoke:
   - set `MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN=wasm`
   - set `MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN_WASM_MANIFEST=<manifest path>`
   - ensure that manifest at least targets the `effects` surface and has `frame_tick=true`
   - optional sidecar path: replace the manifest extension with `.mouse_companion_renderer.json`
   - sample sidecar template:
     - `F:\language\cpp\code\MFCMouseEffect\tools\platform\manual\lib\windows-mouse-companion-renderer-sidecar.sample.json`
     - `F:\language\cpp\code\MFCMouseEffect\tools\platform\manual\lib\windows-mouse-companion-renderer-sidecar.wasm-v1.sample.json`
   - fastest dedicated Windows-native smoke:
     - `F:\language\cpp\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-renderer-sidecar-smoke.cmd`
     - or `F:\language\cpp\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-render-proof.cmd -Preset renderer-sidecar-smoke`
   - fastest dedicated Windows-native `wasm_v1` smoke:
     - `F:\language\cpp\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-renderer-sidecar-wasm-v1-smoke.cmd`
     - or `F:\language\cpp\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-render-proof.cmd -Preset renderer-sidecar-wasm-v1-smoke`
   - fastest dedicated Windows-native lane comparison:
     - `F:\language\cpp\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-renderer-lane-matrix.cmd`
     - optional style selection: `-WasmV1Style default|agile|dreamy|charming`
     - optional full style matrix: `-AllWasmV1Styles`
     - this runs `builtin -> builtin_passthrough -> wasm_v1` in sequence, swaps the checked-in sidecar samples automatically, and restores the original sidecar/env afterward
     - it also writes per-lane proof json plus `summary.json` and `summary.md`; if `-JsonOutput` is omitted, the script auto-picks a temp prefix and prints those output paths
   - checked-in `wasm_v1` sample set now has three recommended styles:
     - `windows-mouse-companion-renderer-sidecar.wasm-v1.sample.json`: balanced default-candidate baseline
     - `windows-mouse-companion-renderer-sidecar.wasm-v1.agile.sample.json`: agile-leaning follow/drag profile
     - `windows-mouse-companion-renderer-sidecar.wasm-v1.dreamy.sample.json`: dreamy / floaty follow-first profile
     - `windows-mouse-companion-renderer-sidecar.wasm-v1.charming.sample.json`: charming / click-hold-first profile
   - if sidecar exists, ensure it includes:
     - `schema_version`
     - `renderer_lane = mouse_companion_renderer`
     - `supports_appearance_semantics = true`
     - `appearance_semantics_mode = builtin_passthrough|wasm_v1`
     - optional `combo_preset_override = none|dreamy|agile|charming`
     - when mode=`builtin_passthrough`, optional controlled tuning floats in `0.5 ~ 1.5`:
       - `follow_lift_scale`
       - `click_squash_scale`
       - `drag_lean_scale`
       - `highlight_alpha_scale`
       - `follow_tail_swing_scale`
       - `hold_head_nod_scale`
       - `scroll_tail_lift_scale`
       - `follow_head_nod_scale`
     - when mode=`wasm_v1`, require `appearance_semantics` with bounded patch fields such as:
       - `theme.glow_color / body_stroke / head_fill / accent_fill / accessory_fill / pedestal_fill`
       - `frame.body_width_scale / body_height_scale / head_width_scale / head_height_scale`
       - `face.blush_width_scale / muzzle_width_scale / forehead_width_scale / pupil_focus_scale / highlight_alpha_scale / whisker_spread_scale`
       - `appendage.ear_scale / tail_width_scale / tail_height_scale / follow_tail_width_scale / follow_ear_spread_scale / click_ear_lift_scale`
       - `motion.follow_state_lift_scale / click_squash_scale / drag_lean_scale / hold_head_nod_scale / scroll_tail_lift_scale / follow_head_nod_scale`
       - `mood.glow_tint_mix_scale / accent_tint_mix_scale / shadow_tint_mix_scale / shadow_alpha_bias / pedestal_alpha_bias / hold_band_alpha_scale / scroll_arc_alpha_scale / drag_line_alpha_scale / follow_trail_alpha_scale`
   - then call the same proof/status route and confirm the runtime reports either:
     - `appearance_plugin_kind=wasm`, or
     - builtin fallback plus a non-empty `appearance_plugin_failure_reason`
   - the dedicated `renderer-sidecar-smoke` preset now also fails fast when:
     - `appearance_plugin_kind` is not `wasm`
     - `appearance_plugin_metadata_path` is empty
     - `appearance_plugin_appearance_semantics_mode` is not `builtin_passthrough`
     - `default_lane_candidate` is not `builtin_passthrough`
     - `default_lane_source` is not `env_wasm_candidate`
     - `default_lane_rollout_status` is not `candidate_pending_manual_confirmation`
     - `default_lane_style_intent` is not `style_candidate:builtin_passthrough_baseline`
   - if you swap to the checked-in `wasm_v1` sample, keep the same wasm env, but use:
     - `F:\language\cpp\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-renderer-sidecar-wasm-v1-smoke.cmd`
     - or `F:\language\cpp\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-render-proof.cmd -Route proof -Event status -ExpectedAppearancePluginKind wasm -ExpectAppearancePluginMetadataPresent $true -ExpectedAppearanceSemanticsMode wasm_v1`
   - that `wasm_v1` smoke now also expects:
     - `default_lane_candidate = wasm_v1`
     - `default_lane_source = env_wasm_candidate`
     - `default_lane_rollout_status = candidate_pending_manual_confirmation`
     - `default_lane_style_intent = style_candidate:balanced_default_candidate`
   - optional single-style assertion:
     - `-WasmV1Style default|agile|dreamy|charming`
     - when used, the same smoke route will assert the matching `default_lane_style_intent`
   - if you use the sample sidecar unchanged, expect the effective combo persona to move toward `dreamy` and the dynamic motion to feel more lifted/elastic during `follow / click / drag / hold / scroll`
   - if you use the checked-in default `wasm_v1` sample unchanged, expect a slightly cooler, cleaner, more balanced full-pet read than builtin, with moderate lift/squash/drag deltas and no single action family dominating the overall style
   - if you use the checked-in `wasm_v1` agile sample unchanged, expect a narrower/agiler body read with a slightly taller torso, slightly broader muzzle, slightly tighter forehead silhouette, cooler glow/accent mood, stronger follow lift/drag lean, slightly tighter eye focus, slightly wider whisker spread, a fuller follow-tail read with slightly taller tail volume, a slightly tighter head silhouette, a broader `follow` ear spread, a more obvious `click` ear lift, and a more obvious `scroll` tail lift / `follow` head nod than the baseline builtin lane; the checked-in sample now also pushes clearer `hold` band / `drag` line / `follow` trail / `scroll` arc alpha plus a slightly cooler shadow tint and cooler body-stroke + head-fill read than builtin
   - if you swap to the checked-in `dreamy` sample unchanged, expect a softer head read, brighter highlights, lighter shadow/pedestal grounding, stronger `follow` trail, gentler `drag`, and a more floaty `follow / scroll` feel than builtin
   - if you swap to the checked-in `charming` sample unchanged, expect a rounder head/body read, warmer accent/accessory mood, stronger `click` ear lift and `click` squash, and a more obvious `hold` band than builtin
   - if you use the dedicated lane matrix unchanged, expect:
     - `builtin`: baseline real-preview lane with no renderer sidecar contract attached
     - `builtin_passthrough`: same wasm provider attach, but still closest to builtin semantics with extra dreamy lift/elasticity from the sample tuning
     - `wasm_v1`: the strongest lane delta, with a cooler/agiler mood and more obvious structure/motion patching than baseline builtin
   - if you use `-AllWasmV1Styles`, expect the matrix to expand into:
     - `wasm_v1_default`
     - `wasm_v1_agile`
     - `wasm_v1_dreamy`
     - `wasm_v1_charming`
     - and the generated observation template to list all four separately under each action group
   - the generated `summary.json` / `summary.md` now also contain an automatic `vs builtin` compare section, which is the fastest way to confirm lane drift before doing the manual motion read:
     - `plugin_kind`
     - `appearance_semantics_mode`
     - `default_lane_candidate`
     - `default_lane_source`
     - `default_lane_rollout_status`
     - `default_lane_style_intent`
     - `appearance_plugin_sample_tier`
     - `appearance_combo_preset`
     - `selection_reason`
     - `failure_reason`
     - whether a renderer sidecar metadata path was present
   - each lane row now also reports `default_lane_brief = candidate/source/rollout/style_intent`, so the runtime default-lane call can be skimmed without opening the raw per-lane json
   - each lane row now also reports `configured_style` and `configured_sample_path`, so the checked-in sample contract used for that lane is visible in the summary itself
   - each lane row now also reports `style_focus_profile`, so the intended motion emphasis is visible without reopening the sidecar
   - each lane row now also reports `configured_sample_tier`, so ship-default candidates and experimental styles are not mixed together
   - each lane row now also reports `runtime_sample_tier` and `runtime_contract_brief`, so summary can confirm whether runtime diagnostics still match the checked-in sample contract
   - each lane row now also reports a short `style` field, so `wasm_v1_agile / dreamy / charming` can be skimmed directly from the summary without re-parsing the lane label
   - the same summary now also carries a conservative machine recommendation for `recommended_default_lane`; treat it as a triage hint first, not an automatic ship decision
   - when the matrix recommends one of the expanded `wasm_v1_*` lanes, it now also records `recommendation_style_intent`, so you can see whether the machine is currently leaning toward `agile_follow_drag`, `dreamy_follow_scroll`, or `charming_click_hold`
   - the same recommendation now also records `recommended_sample_path`, so the next checked-in sidecar candidate can be picked up directly
  - machine recommendation priority now prefers runtime `default_lane_candidate_tier` first, then sample tier, then runtime `default_lane_style_intent`, rather than depending only on a fixed lane-name order
   - `render-proof` console output now also prints `default_lane_summary = candidate/source/rollout/style_intent` and `appearance_plugin_contract_brief = semantics_mode/style_intent/sample_tier`, so single-lane smoke and sweep logs use the same vocabulary as runtime and lane matrix
   - saved `render-proof` JSON now also carries both fields under `real_renderer_preview` and `renderer_runtime_after`, so downstream scripts do not need to recompose them
   - lane matrix summary now also carries `runtime_contract_brief`, so the same short contract string can be checked without reopening proof json
   - the same run now also emits `observation-template.md`, which is the shortest place to record the human-side outcome for `follow / drag / click / hold / scroll` without losing the matching machine summary
   - current stable preflight failure codes include:
     - `renderer_plugin_manifest_io_error`
     - `renderer_plugin_manifest_json_parse_error`
     - `renderer_plugin_manifest_invalid`
     - `renderer_plugin_metadata_io_error`
     - `renderer_plugin_metadata_json_parse_error`
     - `renderer_plugin_metadata_invalid`
     - `renderer_plugin_metadata_lane_mismatch`
     - `renderer_plugin_metadata_missing_appearance_semantics`
     - `renderer_plugin_metadata_appearance_mode_unsupported`
     - `renderer_plugin_metadata_combo_preset_unsupported`
     - `renderer_plugin_metadata_tuning_out_of_range`
     - `renderer_plugin_metadata_missing_appearance_semantics_payload`
     - `renderer_plugin_metadata_appearance_payload_invalid`
     - `renderer_plugin_metadata_appearance_payload_out_of_range`
     - `renderer_plugin_manifest_missing_effects_surface`
     - `renderer_plugin_manifest_requires_frame_tick`

## Renderer Lane Matrix
1. Use the dedicated lane matrix entry first:
   - `F:\language\cpp\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-renderer-lane-matrix.cmd`
2. Treat `builtin` as the control lane.
3. Compare `builtin_passthrough` against `builtin` first.
4. Compare `wasm_v1` against `builtin` second.
5. During `follow`, check:
   - lift height
   - ear spread
   - tail swing
   - whether the whole pet feels lighter or more planted
6. During `drag`, check:
   - lean direction
   - reach posture
   - eye/brow focus
7. During `click / hold`, check:
   - squash and rebound
   - blush/highlight response
   - whether upbeat response reads rounder or tighter than baseline
8. During `scroll`, check:
   - tail lift
   - glow/shadow/pedestal mood shift
9. Current checked-in sample expectation:
   - `builtin`: control baseline
   - `builtin_passthrough`: slightly dreamier, lighter, more elastic than baseline
   - `wasm_v1`: cooler, tighter, slightly more agile than baseline
10. Suggested manual record:
   - `builtin_passthrough`: `pass|fail`, plus `stronger|weaker|same` versus builtin
   - `wasm_v1`: `pass|fail`, plus `stronger|weaker|same` versus builtin
11. If all three lanes feel nearly identical after `follow / drag / click / hold / scroll`, record:
   - `fail (lane readability too weak)`
12. If `builtin_passthrough` and `wasm_v1` are both readable but only mainly during motion, record:
   - `pass (dynamic-biased lane delta)`
13. After the run, keep the generated `summary.md` as the shortest replay artifact:
   - it already records lane order, backend/plugin/semantics mode, and the recommended manual compare focus
   - it now also records a machine-side `recommended_default_lane` candidate, but that recommendation remains low-confidence until the manual observation template is filled
   - it also records `rollout_contract_status`; treat `candidate_pending_manual_confirmation` as a hint only, not a ship/no-ship decision
   - if you also want to keep the human judgement in the same artifact bundle, fill in `observation-template.md` instead of writing free-form notes elsewhere
   - that same template now also has final decision fields for:
     - `best lane for current Win pet`
     - `recommended default lane now`
     - `machine style focus`
     - `machine candidate tier`
     - `machine runtime default lane`
     - `machine recommended sample tier`
     - `manual confirmation result`
     - `recommended next tuning target`
14. Use the compact lane verdict as the shortest machine summary first:
   - format: `backend/plugin/mode/pass|fail`
   - example reading:
     - `real/native_builtin//pass`
     - `real/wasm/builtin_passthrough/pass`
     - `real/wasm/wasm_v1/pass`

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
- `default_lane_candidate`
- `default_lane_source`
- `default_lane_rollout_status`
- `default_lane_style_intent`
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
   - for a reusable Windows-native entry, run:
     - `D:\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-render-proof.cmd -BaseUrl <url> -Token <token> -Route sweep`
     - single-event proof is also supported:
       - `D:\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-render-proof.cmd -BaseUrl <url> -Token <token> -Route proof -Event click`
     - when validating the gated real preview path, prefer:
       - `D:\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-render-proof.cmd -Preset real-preview-smoke`
       - `D:\code\MFCMouseEffect\tools\platform\manual\run-windows-mouse-companion-render-proof.cmd -BaseUrl <url> -Token <token> -Preset real-preview-smoke`
     - when `-BaseUrl/-Token` are omitted, the native entry now auto-reads `%APPDATA%\MFCMouseEffect\websettings_runtime_auto.json`
     - that handoff file is now written automatically when the app opens WebSettings, so the shortest Windows flow no longer needs manual copy/paste of the loopback URL and token
     - the `.cmd` entry forwards into a PowerShell implementation, so Windows bring-up no longer depends on Git Bash just to issue proof/sweep checks
     - the Windows-native entry exits non-zero if proof expectations are missed, so it can be used as a simple bring-up gate instead of only a logging helper
   - for a reusable Git Bash entry on Windows, run:
     - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-windows-mouse-companion-render-proof.sh --base-url <url> --token <token> --route sweep`
     - single-event proof is also supported:
       - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-windows-mouse-companion-render-proof.sh --base-url <url> --token <token> --route proof --event click`
     - when validating the gated real preview path, prefer:
       - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-windows-mouse-companion-render-proof.sh --base-url <url> --token <token> --route sweep --expected-backend real --expect-preview-active true`
       - or use the shortest preset form:
         - `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/tools/platform/manual/run-windows-mouse-companion-render-proof.sh --base-url <url> --token <token> --preset real-preview-smoke`
       - the preset now also prints a short human-readable expectation checklist before issuing the sweep, so a manual bring-up run can quickly confirm env/gate assumptions before reading the JSON-backed result rows
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
       - whiskers should no longer stay frozen:
         - `click` should spread them a bit more
         - `drag/scroll` should tilt them slightly with directional intent
         - `hold` should keep them tighter than `click`
         - `follow` should look more alert
       - head/body should read more connected:
         - a light neck bridge should make the head feel attached instead of floating as a separate sticker
       - front/rear depth should read more clearly too:
         - shoulder and hip patches should make the body mass feel less flat without turning into noisy shading blobs
       - small appendage/material accents should now read more coherently too:
         - tail tip and paw pads should look attached to the same creature palette instead of like unrelated hardcoded stickers
       - tail/body attachment should also read cleaner:
         - a light tail-root cuff should make the tail feel seated into the body instead of pasted on as a separate oval
       - head/ear attachment should also read cleaner:
         - light ear-root cuffs should make the ears feel seated into the head instead of pasted on as separate triangles
       - lower-head silhouette should also read cleaner:
         - cheek/jaw contour cues should make the face feel less like a plain circle with features stamped on top
       - nose/mouth placement should also read cleaner:
         - a light muzzle pad should make the nose and mouth feel seated on a front face plane instead of floating directly on the head circle
       - brow/eye zone should also read cleaner:
         - a light forehead pad should make the upper face feel like a surface, not just brows and eye highlights laid on a flat circle
       - eye-socket side structure should also read cleaner:
         - light temple contours should make the eyes feel seated into a face volume instead of floating in open space
       - eye-socket lower structure should also read cleaner:
         - light under-eye contours should make the eye area feel continuous instead of ending abruptly under the eyeballs
       - face centerline should also read cleaner:
         - a light nose bridge should make the forehead and muzzle feel connected instead of like two separate face islands
      - torso mass should also read cleaner:
        - light belly/flank contours should make the body feel less like one flat oval and help balance the newer face-detail work
      - torso centerline should also read cleaner:
        - a light sternum contour should make the chest-to-belly transition feel continuous instead of like separate upper/lower body islands
      - upper torso should also read cleaner:
        - a light upper-torso contour should make the shoulder/chest area feel more continuous instead of relying only on shoulder patches plus a chest oval
      - rear torso should also read cleaner:
        - light back contours should make the body feel less like only front-facing patches layered on one torso oval
      - limb attachment should also read cleaner:
        - light hand/leg root cuffs should make the limbs feel more anchored into the torso instead of pasted directly onto it
      - limb outer silhouette should also read cleaner:
        - light hand/leg silhouette bridges should make the limb-to-torso transition feel smoother instead of like a pile of local connection patches
      - whole silhouette should also read more state-aware:
        - `follow` should feel slightly more alert with a larger head/readier stance
        - `hold` should feel slightly tucked with a tighter hand/leg stance
        - `click / drag` should lightly bias head or hand reach instead of relying only on overlays
      - torso block should also read more state-aware:
        - `follow` should feel a little narrower/taller and slightly lifted
        - `hold` should still feel wider/lower than the other states
        - `click / drag` should lightly bias torso shape instead of leaving the body block visually unchanged
      - appendages should also read more state-aware:
        - `follow` should feel a bit more alert in both tail and ears
        - `hold` should feel slightly more tucked in the appendages
        - `click / scroll` should lightly bias ear lift or tail energy instead of leaving appendages on fully default proportions
      - atmosphere grounding should also read more state-aware:
        - `follow` should feel a little lighter/cleaner under the pet
        - `hold` should feel a little more planted in shadow/pedestal
        - `drag` should lightly bias the ambient block instead of leaving glow/shadow/pedestal fully static
      - atmosphere grounding should also shift slightly in placement:
        - `follow / drag` should not feel perfectly centered under every pose
        - `hold` should feel slightly heavier/lower in its landing without changing the visible contract outside test mode
      - atmosphere grounding should also shift slightly in weight:
        - `follow` should read a little lighter through shadow/pedestal opacity
        - `hold` should read a little more planted through shadow/pedestal opacity
        - `drag` should get a mild weight bias instead of staying fully neutral
      - upper atmosphere should also read more state-aware:
        - `follow` should feel a little more lifted/airy above the pet
        - `hold` should feel slightly tighter/denser above the pet
        - `drag` should lightly bias the glow cap direction instead of leaving it perfectly centered
      - `click / scroll` should also affect atmosphere now:
        - `click` should lightly tighten/brighten the upper atmosphere instead of only changing the pet body
        - `scroll` should lightly energize both the upper atmosphere and the grounding lane instead of leaving them mostly neutral
      - atmosphere color should also feel more unified:
        - shadow/pedestal should lightly pick up the same action color family instead of staying on a separate neutral track while glow changes above
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

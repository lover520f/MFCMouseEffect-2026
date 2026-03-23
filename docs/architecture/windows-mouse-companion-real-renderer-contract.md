# Windows Mouse Companion Real Renderer Contract

## Purpose
- Define the interface boundary for the future Windows `Mouse Companion` real renderer.
- Ensure the current placeholder path remains a replaceable renderer backend instead of becoming a permanent special case.
- Keep renderer choice open while locking the data contract early.

## Problem Classification
- `Design behavior / future renderer interface contract`
- Short evidence:
  - Windows Phase1.5 already consumes shared inputs such as `model`, `action_library`, `appearance_profile`, `PetPoseFrame`, and runtime action state.
  - What is still missing is a renderer-facing contract that states how a future richer Windows backend plugs in without rewriting `AppController` or replacing the host lifecycle again.

## Non-Goal
This contract does **not** choose the final Windows rendering technology yet.
Examples intentionally left open:
- Direct2D
- DirectComposition
- Direct3D
- sprite-based hybrid renderer
- CPU model preview path

The goal is interface stability first, backend choice later.

## Renderer Position In The Stack
Expected long-term stack:
- shared controller/runtime
- platform visual host
- presenter/runtime coordinator
- renderer backend

Conceptually:
- `asset coordinator -> presenter/runtime -> renderer backend`

## Stable Upstream Inputs
A future Windows real renderer must be able to consume the same conceptual inputs already carried by the current host contract.

### 1. Model Lane
Input meaning:
- resolved model asset path
- resolved source format
- load success/failure diagnostics

Current source examples:
- `pet-main.usdz`
- future Windows-native converted/runtime-ready asset forms

Contract rule:
- renderer backend may choose its own runtime representation, but upstream lanes must still describe one canonical resolved model asset input

### 2. Action-Library Lane
Input meaning:
- action library path
- action clip selection state
- sampled action runtime state

Contract rule:
- action selection/sampling should remain outside the renderer where possible
- renderer should consume either sampled pose/output or compact clip-selection/runtime state, not own controller semantics

### 3. Appearance-Profile Lane
Input meaning:
- appearance profile path
- resolved color/material/accessory semantics
- load success/failure diagnostics

Contract rule:
- appearance profile remains a first-class asset lane independent of full model reload
- renderer consumes resolved appearance intent, not controller-specific branching
- Windows Phase1.5 real preview still stays in stylized 2D geometry space rather than macOS SceneKit model rendering, so the near-term parity target is asset-intent alignment instead of renderer-form-factor parity
- resolved appearance intent should drive a coherent preview family (body/head/ear/accessory/pedestal palette decisions), not only a single body-fill override, so Windows can approximate macOS asset-theme feedback before full model rendering exists
- resolved appearance intent may also bias lightweight frame/face proportions inside renderer-owned builders (for example body/head ratio, muzzle/jaw/cheek emphasis), so `skinVariantId` affects more than color while still staying below full-model geometry replacement
- the same lightweight appearance-structure bias may extend into appendage and patch layers too (for example ear scale, tail thickness, shoulder/hip patch fullness), so Windows can express a family look across the whole pet without breaking the Phase1.5 stylized-preview contract
- appearance intent may also bias lightweight expression tuning (for example brow tilt, pupil focus, mouth reactivity, highlight/whisker emphasis) inside renderer-owned face builders, so different skins can feel slightly different under the same action without forking controller action semantics
- the same lightweight appearance intent may bias appendage/body action cadence too (for example follow reach, hold stance, ear spread, tail width/height response), so different skins can carry slightly different movement character while action ownership still remains in shared runtime/controller layers
- if needed, that appearance-driven movement character may also reach renderer-owned motion synthesis itself (for example follow lift, click squash, drag lean, head nod, tail swing scaling), but only as a lightweight bias layered on top of shared action semantics rather than a new action taxonomy
- once appearance intent reaches palette/frame/face/appendage/motion together, those branches should be centralized behind one renderer-owned semantics source instead of being re-expanded independently in each builder; current Phase1.5 now expects that consolidation through `Win32MouseCompanionRealRendererAppearanceSemantics`
- accessory semantics should stay renderer-owned but asset-driven; `appearanceAccessoryIds` may map to distinct preview adornment families (for example star / moon / leaf / ribbon) instead of collapsing every accessory into one generic badge glyph
- accessory-family selection should also carry a small amount of shape-specific staging (anchor/offset/bounds tuning) inside renderer contracts, so Windows preview can approximate asset-specific attachment semantics without leaking adornment branching back into controller layers
- accessory-family differentiation should reach lightweight internal detail too (for example moon inset highlight, leaf vein, ribbon fold lines), so asset intent is visible as family-specific structure instead of only a silhouette swap
- accessory-family semantics may also bias lightweight motion/staging within renderer-owned adornment contracts (for example moon lift, leaf sway, ribbon bounce) as long as controller-visible action semantics stay unchanged and the behavior remains a presentation concern
- once skin and accessory semantics both exist, their combination should also be allowed to bias renderer-owned presentation lightly (for example head/face emphasis, appendage cadence, motion softness/sharpness) through the same centralized semantics source, so Windows preview can consume `skinVariantId + accessory family` as one appearance system instead of two disconnected override lanes
- that same centralized combination semantics may also bias preview mood/grounding contracts lightly (for example glow/accent tint strength, shadow/pedestal weight, overlay emphasis) as long as those changes remain renderer-owned presentation effects and do not create a new controller-visible action/state taxonomy
- high-value appearance combinations may also resolve to stable renderer-owned persona presets (for example dreamy/agile/charming) as long as those presets remain centralized inside the renderer semantics source and bias only renderer-owned presentation layers rather than introducing a new shared runtime taxonomy
- that renderer-owned semantics source should now be treated as plugin-hosted rather than permanently hardcoded in builders; Phase1.5 currently uses a builtin native plugin behind `Win32MouseCompanionRenderPluginHost`, and future wasm adoption should replace the provider behind that seam instead of reopening every builder

### 4. Pose-Frame Lane
Input meaning:
- latest `PetPoseFrame`
- per-bone semantic transforms / pose sample data
- optional pose binding state

Contract rule:
- pose transport stays shared and platform-neutral
- renderer may bind it to real bones, sprites, rig handles, or future adapters, but the upstream `PetPoseFrame` lane must not fork into a Windows-only schema

### 5. Runtime Action Lane
Input meaning:
- current semantic action (`idle / follow / click_react / drag / hold_react / scroll_react`)
- direction/facing hints
- action intensity / transient pulse state
- placement/follow anchors already normalized by presenter/runtime layers

Contract rule:
- renderer consumes action state as presentation intent
- controller and host keep ownership of action semantics and placement policy

## Required Downstream Responsibilities
A future real renderer backend is expected to own only renderer-local concerns:
- visual resource creation/destruction
- scene/mesh/sprite/model instance ownership
- per-frame draw/update application
- renderer-local material/shader/scene graph details
- renderer-local animation interpolation if needed

It should **not** own:
- top-level placement policy
- monitor/clamp rules
- controller config parsing
- launch/show/hide lifecycle policy
- cross-platform runtime semantics

## Renderer Plugin Host Contract

### Situation
- `appearance / accessory family / combo persona / motion bias` 这类语义属于 renderer-owned presentation。
- 若这些语义长期散在 builder/painter 内部，后续 wasm 接入会被迫再次拆层。

### Target
- renderer plugin host 负责：
  - 选择 provider
  - 调用 provider
  - 暴露统一 diagnostics
- provider 负责：
  - 把共享 runtime/style/appearance 输入扩展成 renderer-owned semantics

### Action
- 当前输入保持收敛：
  - `SceneRuntime`
  - `StyleProfile`
  - appearance asset context
- 当前输出保持结构化：
  - `theme`
  - `frame/face/appendage/motion/adornment/mood semantics`
  - `combo preset`
- wasm 正确挂载方式应为：
  - `wasm module -> wasm adapter -> renderer plugin host -> renderer builders`
- 当前 Windows-first skeleton 选择面先固定为环境变量：
  - `MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN=auto|builtin|wasm`
  - `MFX_WIN32_MOUSE_COMPANION_RENDER_PLUGIN_WASM_MANIFEST=<manifest path>`
  - `wasm` 请求失败时必须自动回退到 builtin provider，不能让 renderer bring-up 因 provider 装载失败而整体不可用
- 当前 renderer-plugin wasm manifest 预检合同：
  - manifest 必须声明 `effects` surface
  - manifest 必须启用 `frame_tick`
  - 若 manifest 同目录存在可选 sidecar `<manifest>.mouse_companion_renderer.json`（replace-extension 语义），则其中必须声明：
    - `schema_version >= 1`
    - `renderer_lane = mouse_companion_renderer`
    - `supports_appearance_semantics = true`
    - `appearance_semantics_mode = builtin_passthrough|wasm_v1`
    - 当 mode=`builtin_passthrough` 时：
      - optional `combo_preset_override = none|dreamy|agile|charming`
      - optional controlled tuning floats (`0.5 ~ 1.5`):
        - `follow_lift_scale`
        - `click_squash_scale`
        - `drag_lean_scale`
        - `highlight_alpha_scale`
        - `follow_tail_swing_scale`
        - `hold_head_nod_scale`
        - `scroll_tail_lift_scale`
        - `follow_head_nod_scale`
    - 当 mode=`wasm_v1` 时：
      - 仍可带 `combo_preset_override`
      - 必须声明 `appearance_semantics` object
      - 当前受控字段为：
        - `theme.glow_color / body_stroke / head_fill / accent_fill / accessory_fill / pedestal_fill`
        - `frame.body_width_scale / body_height_scale / head_width_scale / head_height_scale`
        - `face.blush_width_scale / muzzle_width_scale / forehead_width_scale / brow_tilt_scale / mouth_reactive_scale / pupil_focus_scale / highlight_alpha_scale / whisker_spread_scale`
        - `appendage.ear_scale / tail_width_scale / tail_height_scale / follow_tail_width_scale / follow_ear_spread_scale / follow_leg_stance_scale / hold_leg_stance_scale / drag_hand_reach_scale / click_ear_lift_scale`
        - `motion.follow_state_lift_scale / click_squash_scale / drag_lean_scale / body_forward_scale / hold_head_nod_scale / scroll_tail_lift_scale / follow_head_nod_scale`
        - `mood.glow_tint_mix_scale / accent_tint_mix_scale / shadow_tint_mix_scale / pedestal_tint_mix_scale / shadow_alpha_bias / pedestal_alpha_bias / click_ring_alpha_scale / hold_band_alpha_scale / scroll_arc_alpha_scale / drag_line_alpha_scale / follow_trail_alpha_scale`
      - scale 类字段当前受控范围默认仍为 `0.5 ~ 1.5`
      - `shadow_alpha_bias / pedestal_alpha_bias` 当前受控范围为 `-24 ~ 24`
      - host 当前应按固定层次应用这些 patch：
        - `theme`
        - `shape`（`frame / face / appendage`）
        - `motion`
        - `mood`
      - 目标是让 `wasm_v1` 继续保持 bounded semantics patch，而不是把 host 重新退化成一大段无序字段覆盖代码
  - 任何预检失败都按 provider attach 失败处理，并通过统一 fallback diagnostics 暴露给 `/api/state` / render-proof
  - 这组预检与失败归因应集中维护在独立 contract/helper 文件中，而不是继续散落在 renderer host 与 builder 内部
  - failure reason 应优先输出稳定 code，例如 `renderer_plugin_manifest_io_error / renderer_plugin_manifest_json_parse_error / renderer_plugin_manifest_invalid / renderer_plugin_metadata_io_error / renderer_plugin_metadata_json_parse_error / renderer_plugin_metadata_invalid / renderer_plugin_metadata_lane_mismatch / renderer_plugin_metadata_missing_appearance_semantics / renderer_plugin_metadata_appearance_mode_unsupported / renderer_plugin_metadata_combo_preset_unsupported / renderer_plugin_metadata_tuning_out_of_range / renderer_plugin_metadata_missing_appearance_semantics_payload / renderer_plugin_metadata_appearance_payload_invalid / renderer_plugin_metadata_appearance_payload_out_of_range / renderer_plugin_manifest_missing_effects_surface / renderer_plugin_manifest_requires_frame_tick`

### Result
- 当前与后续 provider 都应复用同一组运行时诊断：
  - `appearance_plugin_id`
  - `appearance_plugin_kind`
  - `appearance_plugin_source`
  - `appearance_plugin_selection_reason`
  - `appearance_plugin_failure_reason`
  - `appearance_plugin_manifest_path`
  - `appearance_plugin_runtime_backend`
  - `appearance_plugin_metadata_path`
  - `appearance_plugin_metadata_schema_version`
  - `appearance_plugin_appearance_semantics_mode`
  - `default_lane_candidate`
  - `default_lane_source`
  - `default_lane_rollout_status`
  - `default_lane_style_intent`
  - `appearance_plugin_sample_tier`
  - `appearance_plugin_contract_brief`
  - `default_lane_source` 当前稳定值应优先使用短机器码，例如：
    - `runtime_builtin_default`
    - `env_builtin_forced`
    - `env_wasm_candidate`
    - `env_wasm_fallback_builtin`
    - `runtime_plugin_candidate`
  - `default_lane_style_intent` 当前稳定值应优先使用短机器码，例如：
    - `style_candidate:none`
    - `style_candidate:builtin_passthrough_baseline`
    - `style_candidate:balanced_default_candidate`
    - `style_candidate:agile_follow_drag`
    - `style_candidate:dreamy_follow_scroll`
    - `style_candidate:charming_click_hold`
  - renderer sidecar metadata may now declare optional `style_intent`; when present and the runtime actually nominates a non-builtin lane, host/runtime diagnostics should prefer that explicit value over combo-preset-only inference
  - renderer sidecar metadata may now also declare optional `sample_tier`; runtime diagnostics should surface it as `appearance_plugin_sample_tier` without folding it into `default_lane_*`, because it describes the checked-in sample contract rather than the rollout decision itself
  - runtime may additionally expose `default_lane_candidate_tier` as the host-side interpretation of current rollout semantics:
    - `builtin_shipped_default`
    - `baseline_reference_candidate`
    - `ship_default_candidate`
    - `experimental_style_candidate`
    - `unclassified_candidate`
  - runtime may also expose `appearance_plugin_contract_brief = semantics_mode/style_intent/sample_tier`, so higher-level tools can reuse one short summary instead of recomposing those fields
  - `Win32MouseCompanionRealRendererSceneRuntime` now also carries a cached `poseAdapterProfile`; builder/painter/backend code should consume that shared profile instead of recomputing pose adapter influence/readability independently
  - `Win32MouseCompanionRealRendererSceneRuntime` should now also carry cached `modelAssetSourceProfile`, `modelAssetManifestProfile`, `modelAssetCatalogProfile`, `modelAssetBindingTableProfile`, `modelAssetRegistryProfile`, `modelAssetLoadProfile`, `modelAssetDecodeProfile`, and `modelAssetResidencyProfile`; runtime/proof/WebUI may expose:
    - `scene_runtime_model_asset_source_state`
    - `scene_runtime_model_asset_source_brief = source_state/source_format/model:0|1/action:0|1/appearance:0|1`
    - `scene_runtime_model_asset_manifest_state`
    - `scene_runtime_model_asset_manifest_brief = manifest_state/entry_count/resolved_entry_count`
    - `scene_runtime_model_asset_catalog_state`
    - `scene_runtime_model_asset_catalog_brief = catalog_state/entry_count/resolved_entry_count`
    - `scene_runtime_model_asset_binding_table_state`
    - `scene_runtime_model_asset_binding_table_brief = binding_table_state/entry_count/resolved_entry_count`
    - `scene_runtime_model_asset_registry_state`
    - `scene_runtime_model_asset_registry_brief = registry_state/entry_count/resolved_entry_count`
    - `scene_runtime_model_asset_load_state`
    - `scene_runtime_model_asset_load_brief = load_state/entry_count/resolved_entry_count`
    - `scene_runtime_model_asset_decode_state`
    - `scene_runtime_model_asset_decode_brief = decode_state/entry_count/resolved_entry_count`
    - `scene_runtime_model_asset_residency_state`
    - `scene_runtime_model_asset_residency_brief = residency_state/entry_count/resolved_entry_count`
  - `Win32MouseCompanionRealRendererSceneRuntime` should also carry a cached `modelSceneAdapterProfile`; runtime/proof/WebUI may expose:
    - `scene_runtime_model_scene_adapter_state`
    - `scene_runtime_model_scene_seam_readiness`
    - `scene_runtime_model_scene_adapter_brief = seam_state/source_format/adapter_mode`
  - `Win32MouseCompanionRealRendererSceneRuntime` should also carry a cached `modelNodeAdapterProfile`; runtime/proof/WebUI may expose:
    - `scene_runtime_model_node_adapter_influence`
    - `scene_runtime_model_node_adapter_brief = seam_state/influence`
    - `scene_runtime_model_node_channel_brief = body:x|face:y|appendage:z|overlay:w|grounding:v`
  - `Win32MouseCompanionRealRendererSceneRuntime` should also carry a cached `modelNodeGraphProfile`; runtime/proof/WebUI may expose:
    - `scene_runtime_model_node_graph_state`
    - `scene_runtime_model_node_graph_node_count`
    - `scene_runtime_model_node_graph_bound_node_count`
    - `scene_runtime_model_node_graph_brief = graph_state/node_count/bound_node_count`
  - `Win32MouseCompanionRealRendererSceneRuntime` should also carry a cached `modelNodeBindingProfile`; runtime/proof/WebUI may expose:
    - `scene_runtime_model_node_binding_state`
    - `scene_runtime_model_node_binding_entry_count`
    - `scene_runtime_model_node_binding_bound_entry_count`
    - `scene_runtime_model_node_binding_brief = binding_state/entry_count/bound_entry_count`
    - `scene_runtime_model_node_binding_weight_brief = body:x|head:y|appendage:z|overlay:w|grounding:v`
  - `Win32MouseCompanionRealRendererSceneRuntime` should also carry a cached `modelNodeSlotProfile`; runtime/proof/WebUI may expose:
    - `scene_runtime_model_node_slot_state`
    - `scene_runtime_model_node_slot_count`
    - `scene_runtime_model_node_ready_slot_count`
    - `scene_runtime_model_node_slot_brief = slot_state/slot_count/ready_slot_count`
    - `scene_runtime_model_node_slot_name_brief = body:body_root|head:head_anchor|appendage:appendage_anchor|overlay:overlay_anchor|grounding:grounding_anchor`
  - `Win32MouseCompanionRealRendererSceneRuntime` should also carry a cached `modelNodeRegistryProfile`; runtime/proof/WebUI may expose:
    - `scene_runtime_model_node_registry_state`
    - `scene_runtime_model_node_registry_entry_count`
    - `scene_runtime_model_node_registry_resolved_entry_count`
    - `scene_runtime_model_node_registry_brief = registry_state/entry_count/resolved_entry_count`
    - `scene_runtime_model_node_registry_asset_node_brief = body:asset.body.root|head:asset.head.anchor|appendage:asset.appendage.anchor|overlay:asset.overlay.anchor|grounding:asset.grounding.anchor`
    - `scene_runtime_model_node_registry_weight_brief = body:x|head:y|appendage:z|overlay:w|grounding:v`
  - `Win32MouseCompanionRealRendererSceneRuntime` should also carry a cached `assetNodeBindingProfile`; runtime/proof/WebUI may expose:
    - `scene_runtime_asset_node_binding_state`
    - `scene_runtime_asset_node_binding_entry_count`
    - `scene_runtime_asset_node_binding_resolved_entry_count`
    - `scene_runtime_asset_node_binding_brief = asset_binding_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_binding_path_brief = body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding`
    - `scene_runtime_asset_node_binding_weight_brief = body:x|head:y|appendage:z|overlay:w|grounding:v`
  - `Win32MouseCompanionRealRendererSceneRuntime` should also carry a cached `assetNodeTransformProfile`; runtime/proof/WebUI may expose:
    - `scene_runtime_asset_node_transform_state`
    - `scene_runtime_asset_node_transform_entry_count`
    - `scene_runtime_asset_node_transform_resolved_entry_count`
    - `scene_runtime_asset_node_transform_brief = transform_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_transform_path_brief = body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding`
    - `scene_runtime_asset_node_transform_value_brief = body:(x,y,s)|head:(x,y,s)|appendage:(x,y,s)|overlay:(x,y,s)|grounding:(x,y,s)`
  - `Win32MouseCompanionRealRendererSceneRuntime` should also carry a cached `assetNodeResolverProfile`; runtime/proof/WebUI may expose:
    - `scene_runtime_asset_node_resolver_state`
    - `scene_runtime_asset_node_resolver_entry_count`
    - `scene_runtime_asset_node_resolver_resolved_entry_count`
    - `scene_runtime_asset_node_resolver_brief = resolver_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_resolver_parent_brief = body:root|head:body|appendage:body|overlay:head|grounding:body`
    - `scene_runtime_asset_node_resolver_value_brief = body:(x,y,s)|head:(x,y,s)|appendage:(x,y,s)|overlay:(x,y,s)|grounding:(x,y,s)`
  - `Win32MouseCompanionRealRendererSceneRuntime` should also carry a cached `assetNodeParentSpaceProfile`; runtime/proof/WebUI may expose:
    - `scene_runtime_asset_node_parent_space_state`
    - `scene_runtime_asset_node_parent_space_entry_count`
    - `scene_runtime_asset_node_parent_space_resolved_entry_count`
    - `scene_runtime_asset_node_parent_space_brief = parent_space_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_parent_space_parent_brief = body:root|head:body|appendage:body|overlay:head|grounding:body`
    - `scene_runtime_asset_node_parent_space_value_brief = body:(x,y,s)|head:(x,y,s)|appendage:(x,y,s)|overlay:(x,y,s)|grounding:(x,y,s)`
  - `Win32MouseCompanionRealRendererSceneRuntime` should also carry a cached `assetNodeTargetProfile`; runtime/proof/WebUI may expose:
    - `scene_runtime_asset_node_target_state`
    - `scene_runtime_asset_node_target_entry_count`
    - `scene_runtime_asset_node_target_resolved_entry_count`
    - `scene_runtime_asset_node_target_brief = target_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_target_kind_brief = body:body_target|head:head_target|appendage:appendage_target|overlay:overlay_target|grounding:grounding_target`
    - `scene_runtime_asset_node_target_value_brief = body:(x,y,s)|head:(x,y,s)|appendage:(x,y,s)|overlay:(x,y,s)|grounding:(x,y,s)`
  - `Win32MouseCompanionRealRendererSceneRuntime` should also carry a cached `assetNodeTargetResolverProfile`; runtime/proof/WebUI may expose:
    - `scene_runtime_asset_node_target_resolver_state`
    - `scene_runtime_asset_node_target_resolver_entry_count`
    - `scene_runtime_asset_node_target_resolver_resolved_entry_count`
    - `scene_runtime_asset_node_target_resolver_brief = resolver_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_target_resolver_path_brief = body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding`
    - `scene_runtime_asset_node_target_resolver_value_brief = body:(x,y,s)|head:(x,y,s)|appendage:(x,y,s)|overlay:(x,y,s)|grounding:(x,y,s)`
  - after `scene` build, the backend should also derive an `assetNodeWorldSpaceProfile`; runtime/proof/WebUI may expose:
    - `scene_runtime_asset_node_world_space_state`
    - `scene_runtime_asset_node_world_space_entry_count`
    - `scene_runtime_asset_node_world_space_resolved_entry_count`
    - `scene_runtime_asset_node_world_space_brief = world_space_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_world_space_path_brief = body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding`
    - `scene_runtime_asset_node_world_space_value_brief = body:(x,y,s)|head:(x,y,s)|appendage:(x,y,s)|overlay:(x,y,s)|grounding:(x,y,s)`
    - `scene_runtime_asset_node_pose_state`
    - `scene_runtime_asset_node_pose_entry_count`
    - `scene_runtime_asset_node_pose_resolved_entry_count`
    - `scene_runtime_asset_node_pose_brief = pose_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_pose_path_brief = body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding`
    - `scene_runtime_asset_node_pose_value_brief = body:(x,y,s,tilt)|head:(x,y,s,tilt)|appendage:(x,y,s,tilt)|overlay:(x,y,s,tilt)|grounding:(x,y,s,tilt)`
    - `scene_runtime_asset_node_pose_resolver_state`
    - `scene_runtime_asset_node_pose_resolver_entry_count`
    - `scene_runtime_asset_node_pose_resolver_resolved_entry_count`
    - `scene_runtime_asset_node_pose_resolver_brief = resolver_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_pose_resolver_path_brief = body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding`
    - `scene_runtime_asset_node_pose_resolver_value_brief = body:(x,y,s,tilt)|head:(x,y,s,tilt)|appendage:(x,y,s,tilt)|overlay:(x,y,s,tilt)|grounding:(x,y,s,tilt)`
    - `scene_runtime_asset_node_pose_registry_state`
    - `scene_runtime_asset_node_pose_registry_entry_count`
    - `scene_runtime_asset_node_pose_registry_resolved_entry_count`
    - `scene_runtime_asset_node_pose_registry_brief = registry_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_pose_registry_node_brief = body:pose.body.root|head:pose.head.anchor|appendage:pose.appendage.anchor|overlay:pose.overlay.anchor|grounding:pose.grounding.anchor`
    - `scene_runtime_asset_node_pose_registry_weight_brief = body:w|head:w|appendage:w|overlay:w|grounding:w`
    - `scene_runtime_asset_node_pose_channel_state`
    - `scene_runtime_asset_node_pose_channel_entry_count`
    - `scene_runtime_asset_node_pose_channel_resolved_entry_count`
    - `scene_runtime_asset_node_pose_channel_brief = channel_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_pose_channel_name_brief = body:channel.body.posture|head:channel.head.expression|appendage:channel.appendage.motion|overlay:channel.overlay.fx|grounding:channel.grounding.shadow`
    - `scene_runtime_asset_node_pose_channel_weight_brief = body:w|head:w|appendage:w|overlay:w|grounding:w`
    - `scene_runtime_asset_node_pose_constraint_state`
    - `scene_runtime_asset_node_pose_constraint_entry_count`
    - `scene_runtime_asset_node_pose_constraint_resolved_entry_count`
    - `scene_runtime_asset_node_pose_constraint_brief = constraint_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_pose_constraint_name_brief = body:constraint.body.posture|head:constraint.head.expression|appendage:constraint.appendage.motion|overlay:constraint.overlay.fx|grounding:constraint.grounding.shadow`
    - `scene_runtime_asset_node_pose_constraint_value_brief = body:(strength,x,y,scale,tilt)|...`
    - `scene_runtime_asset_node_pose_solve_state`
    - `scene_runtime_asset_node_pose_solve_entry_count`
    - `scene_runtime_asset_node_pose_solve_resolved_entry_count`
    - `scene_runtime_asset_node_pose_solve_brief = solve_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_pose_solve_path_brief = body:/pet/body/root|head:/pet/body/head|appendage:/pet/body/appendage|overlay:/pet/fx/overlay|grounding:/pet/fx/grounding`
    - `scene_runtime_asset_node_pose_solve_value_brief = body:(weight,x,y,scale,tilt)|...`
    - `scene_runtime_asset_node_joint_hint_state`
    - `scene_runtime_asset_node_joint_hint_entry_count`
    - `scene_runtime_asset_node_joint_hint_resolved_entry_count`
    - `scene_runtime_asset_node_joint_hint_brief = hint_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_joint_hint_name_brief = body:joint.body.spine|head:joint.head.look|appendage:joint.appendage.reach|overlay:joint.overlay.fx|grounding:joint.grounding.balance`
    - `scene_runtime_asset_node_joint_hint_value_brief = body:(weight,reach,spread,tilt)|...`
    - `scene_runtime_asset_node_articulation_state`
    - `scene_runtime_asset_node_articulation_entry_count`
    - `scene_runtime_asset_node_articulation_resolved_entry_count`
    - `scene_runtime_asset_node_articulation_brief = articulation_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_articulation_name_brief = body:articulation.body.spine|head:articulation.head.look|appendage:articulation.appendage.reach|overlay:articulation.overlay.fx|grounding:articulation.grounding.balance`
    - `scene_runtime_asset_node_articulation_value_brief = body:(weight,bend,stretch,twist)|...`
    - `scene_runtime_asset_node_local_joint_registry_state`
    - `scene_runtime_asset_node_local_joint_registry_entry_count`
    - `scene_runtime_asset_node_local_joint_registry_resolved_entry_count`
    - `scene_runtime_asset_node_local_joint_registry_brief = registry_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_local_joint_registry_joint_brief = body:local.body.spine|head:local.head.look|appendage:local.appendage.reach|overlay:local.overlay.fx|grounding:local.grounding.balance`
    - `scene_runtime_asset_node_local_joint_registry_weight_brief = body:weight|...`
    - `scene_runtime_asset_node_articulation_map_state`
    - `scene_runtime_asset_node_articulation_map_entry_count`
    - `scene_runtime_asset_node_articulation_map_resolved_entry_count`
    - `scene_runtime_asset_node_articulation_map_brief = map_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_articulation_map_name_brief = body:map.body.spine|head:map.head.look|appendage:map.appendage.reach|overlay:map.overlay.fx|grounding:map.grounding.balance`
    - `scene_runtime_asset_node_articulation_map_value_brief = body:(weight,rotation,translation)|...`
    - `scene_runtime_asset_node_control_rig_hint_state`
    - `scene_runtime_asset_node_control_rig_hint_entry_count`
    - `scene_runtime_asset_node_control_rig_hint_resolved_entry_count`
    - `scene_runtime_asset_node_control_rig_hint_brief = hint_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_control_rig_hint_name_brief = body:rig.body.spine|head:rig.head.look|appendage:rig.appendage.reach|overlay:rig.overlay.fx|grounding:rig.grounding.balance`
    - `scene_runtime_asset_node_control_rig_hint_value_brief = body:(weight,drive,damping)|...`
    - `scene_runtime_asset_node_rig_channel_state`
    - `scene_runtime_asset_node_rig_channel_entry_count`
    - `scene_runtime_asset_node_rig_channel_resolved_entry_count`
    - `scene_runtime_asset_node_rig_channel_brief = channel_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_rig_channel_name_brief = body:rig.channel.body.spine|head:rig.channel.head.look|appendage:rig.channel.appendage.reach|overlay:rig.channel.overlay.fx|grounding:rig.channel.grounding.balance`
    - `scene_runtime_asset_node_rig_channel_value_brief = body:(weight,amplitude,response)|...`
    - `scene_runtime_asset_node_control_surface_state`
    - `scene_runtime_asset_node_control_surface_entry_count`
    - `scene_runtime_asset_node_control_surface_resolved_entry_count`
    - `scene_runtime_asset_node_control_surface_brief = surface_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_control_surface_name_brief = body:surface.body.spine|head:surface.head.look|appendage:surface.appendage.reach|overlay:surface.overlay.fx|grounding:surface.grounding.balance`
    - `scene_runtime_asset_node_control_surface_value_brief = body:(weight,alpha,stroke)|...`
  - scene build/runtime diagnostics should also expose a derived `assetNodeAnchorProfile` over the same five logical lanes:
    - `scene_runtime_asset_node_anchor_state`
    - `scene_runtime_asset_node_anchor_entry_count`
    - `scene_runtime_asset_node_anchor_resolved_entry_count`
    - `scene_runtime_asset_node_anchor_brief = anchor_state/entry_count/resolved_entry_count`
    - `scene_runtime_asset_node_anchor_point_brief = body:(x,y)|head:(x,y)|appendage:(x,y)|overlay:(x,y)|grounding:(x,y)`
    - `scene_runtime_asset_node_anchor_scale_brief = body:s|head:s|appendage:s|overlay:s|grounding:s`
  - current preview seam should now layer `modelNodeAdapterProfile -> modelNodeGraphProfile -> modelNodeBindingProfile`, so later real asset-node binding can replace the binding-profile generator without rewriting downstream builders
  - current preview seam should now also layer `modelNodeBindingProfile -> modelNodeSlotProfile`, so later true model assets can plug real node names/slots into one stable contract before any real graph traversal replaces preview-owned offsets
  - current real-renderer bring-up should now start from `modelAssetSourceProfile -> modelAssetManifestProfile -> modelSceneAdapterProfile`, so the first true 3D step is no longer “just another preview seam”; it now has a dedicated asset-entry contract for model path, action library, and appearance-profile readiness before node/pose/rig layers consume anything
  - current preview seam should now also layer `modelNodeSlotProfile -> modelNodeRegistryProfile`, so later true asset-node resolution can replace registry generation while keeping builder-facing semantics, diagnostics, and proof vocabulary stable
  - current preview seam should now also layer `modelNodeRegistryProfile -> assetNodeBindingProfile`, so later true asset-node-path lookup can replace this binding-table generator while preserving builder-facing weights and runtime/proof vocabulary
  - current preview seam should now also layer `assetNodeBindingProfile -> assetNodeTransformProfile`, so later real node-local transform resolution can replace this transform-table generator while preserving builder-facing geometry/readability seams and runtime/proof vocabulary
  - current preview seam should now also layer `assetNodeTransformProfile -> assetNodeResolverProfile`, so later true parent-space or hierarchy-aware node resolution can replace this resolver generator while keeping builder-facing local transform semantics and runtime/proof vocabulary stable
  - current preview seam should now also layer `assetNodeResolverProfile -> assetNodeParentSpaceProfile`, so later true node-tree accumulation can replace this parent-space generator while keeping builder-facing hierarchy transforms, diagnostics, WebUI, and proof vocabulary stable
  - current preview seam should now also layer `assetNodeParentSpaceProfile -> assetNodeTargetProfile -> assetNodeTargetResolverProfile`, so later true node-target resolution can replace this target-table generator while preserving builder-facing target semantics, diagnostics, WebUI, and proof vocabulary
  - after builder-owned anchor generation, current preview seam should also derive `assetNodeWorldSpaceProfile`, so later true model node world transforms can replace this world-table generator while preserving painter-facing diagnostics and proof vocabulary
  - current preview seam should now also layer `assetNodeWorldSpaceProfile -> assetNodePoseProfile -> assetNodePoseResolverProfile -> assetNodePoseRegistryProfile -> assetNodePoseChannelProfile -> assetNodePoseConstraintProfile -> assetNodePoseSolveProfile -> assetNodeJointHintProfile -> assetNodeArticulationProfile -> assetNodeLocalJointRegistryProfile -> assetNodeArticulationMapProfile -> assetNodeControlRigHintProfile -> assetNodeRigChannelProfile -> assetNodeControlSurfaceProfile -> assetNodeRigDriverProfile -> assetNodeSurfaceDriverProfile`, so later true model pose/control-rig consumption can replace these generators one seam at a time while preserving runtime/proof/WebUI vocabulary and builder-facing readability contracts
  - current preview seam should now also layer `assetNodeTransformProfile -> assetNodeAnchorProfile`, so later true node-local anchor resolution can replace the preview anchor generator while keeping builder-facing centers/scales, diagnostics, WebUI, and proof vocabulary stable
  - builder code should now prefer consuming the resolver seam over reading raw asset-node transform/binding tables directly, so later real model-node hierarchy work reopens the resolver generator instead of re-fragmenting builder-local math
  - builder code may now also prefer consuming the parent-space seam over raw local-transform tables whenever hierarchy accumulation matters, so later real model-node graph traversal reopens the parent-space generator instead of scattering parent-child math back into builders
  - builder code may now also prefer consuming the target seam over raw parent-space values whenever presentation intent is “target node” rather than “intermediate hierarchy transform”, so later real model-node target resolution reopens the target generator instead of re-fragmenting target math back into builders
  - host-side default-lane style-intent inference and metadata support lists should reuse the same helper, so `style_intent` / `sample_tier` machine vocab does not split between validation and runtime
  - runtime/preview diagnostics should also expose scene-runtime adapter state explicitly instead of only `pose_frame_available / pose_binding_configured` booleans:
    - `scene_runtime_adapter_mode = runtime_only|pose_unbound|pose_bound`
    - `scene_runtime_pose_sample_count`
    - `scene_runtime_bound_pose_sample_count`
  - those fields are runtime-state diagnostics, not metadata contracts: they describe how much of the future model-driven pose lane is actually live in the current frame
  - current Phase1.5 behavior should treat adapter mode as an actual renderer input, not diagnostics-only noise:
    - `runtime_only`: no pose-sample influence
    - `pose_unbound`: reduced pose influence is allowed for lightweight preview continuity
    - `pose_bound`: full bounded pose influence is allowed for appendage/pose-consumer builders
  - adapter mode may also expose one shared readability-bias scale for painter/adornment/overlay consumers, so `runtime_only / pose_unbound / pose_bound` keep one consistent three-tier emphasis contract instead of drifting per builder
  - runtime/proof/summary layers may additionally expose normalized pose-adapter metrics directly:
    - `scene_runtime_pose_adapter_influence`
    - `scene_runtime_pose_readability_bias`
    - `scene_runtime_pose_adapter_brief = mode/influence/readability`
  - current bounded pose influence is allowed to reach:
    - appendage stance/offset lanes directly
    - lightweight head/body expression bias (`body_forward`, `head_nod`, `pupil/brow focus`)
    - lightweight frame/face anchor bias (head/body center, eye/nose/mouth placement, blush/whisker staging)
    - lightweight adornment/overlay/grounding bias (pose-badge visibility, accessory anchor, click/hold/scroll/drag/follow overlay staging, shadow/pedestal grounding)
    - lightweight painter/readability bias (shadow/pedestal alpha, pose-badge confidence, accessory alpha/stroke emphasis)
    - one shared model-node seam for frame/face/adornment/overlay/grounding offsets, with channel-specific influence instead of one undifferentiated node bias
    - downstream rig/surface driver seams that bias anchor scales, tilt, overlay stroke, and painter alpha/stroke without reopening builder-local ad-hoc math
  - but it still must remain a bounded preview aid, not a separate controller-visible action taxonomy
- 新增 renderer-owned semantics 时，应优先扩展 plugin output，而不是把 builder 继续当作事实上的插件层；当前 `wasm_v1` 就是第一步 bounded patch 协议，而不是继续往 `builtin_passthrough` 堆更多 ad-hoc tuning key
- 当前默认 lane rollout 合同：
  - lane matrix 的机器摘要最多只能产出 `recommended_default_lane` candidate
  - machine candidate 只基于：
    - lane proof 通过
    - `appearance_plugin_failure_reason` 为空
    - 该 lane 相比 `builtin` 在 machine compare 中确实存在差异
  - 仅凭 machine candidate 不能直接切换默认 lane
  - 真正允许从 `builtin` 切到更强 lane，仍需后续人工 observation 明确记录：
    - `best lane for current Win pet`
    - `recommended default lane now`
    - `manual confirmation result = approve_default_switch`
  - 在人工确认前，machine recommendation 的 `rollout_contract_status` 必须视为：
    - `candidate_pending_manual_confirmation`
    - 或 `stay_on_builtin`
- checked-in `wasm_v1` sidecar samples may now be curated by readability intent instead of only by raw field coverage:
  - `agile`-leaning sample for sharper `follow / drag`
  - `dreamy`-leaning sample for floatier `follow / scroll`
  - `charming`-leaning sample for rounder `click / hold`
  - future sample additions should stay bounded and contract-compatible rather than introducing new ad-hoc metadata keys

## Recommended Internal Interface Split
The future Windows real renderer can be implemented behind three narrow concepts.

### Asset Loader / Resource Adapter
Responsibility:
- translate resolved model/action/appearance inputs into backend resources
- cache/reload backend resources
- report load failures back to host/runtime diagnostics

### Scene Runtime Adapter
Responsibility:
- receive runtime action + pose + facing inputs
- map them onto backend scene state
- keep renderer-frame state independent from host lifecycle glue

### Renderer Backend
Responsibility:
- perform final visual update/draw
- manage backend-specific device/context/swapchain/composition details

## Compatibility Rule With Placeholder
The current placeholder path should remain a valid backend implementation of the same high-level contract.

This means:
- placeholder and future real renderer should both be able to sit behind the same host/presenter/runtime flow
- switching renderer backend should not require changing `AppController` semantics
- switching renderer backend should not require redefining model/action/appearance/pose inputs

## Exit Criteria For Starting Real Renderer Work
Real renderer implementation work should begin only when all of the following are true:
1. Phase1.5 placeholder has been declared stable enough by `/Users/sunqin/study/language/cpp/code/MFCMouseEffect/docs/architecture/windows-mouse-companion-phase15-exit-contract.md`
2. shared input lanes are already observable through diagnostics
3. presenter/runtime layers are no longer growing because of placeholder-specific draw concerns
4. there is a clear decision whether the next milestone targets:
   - higher-quality 2D replacement, or
   - true model-driven renderer parity

## First Implementation Recommendation
Before touching GPU-specific code, the safest first step is:
1. add a renderer-facing adapter interface under `Platform/windows/Pet`
2. make the current placeholder renderer conform to that interface
3. prove renderer swap does not change host/controller contracts
4. only then add a second backend implementation

## Current Progress
- The first adapter seam is now active:
  - `IWin32MouseCompanionRendererBackend`
  - `Win32MouseCompanionRendererBackendFactory`
- Built-in backend routing no longer ends at one hardcoded constructor:
  - `Win32MouseCompanionRendererBackendRegistry`
  - placeholder backend explicit registration during default-factory setup
- `Win32MouseCompanionWindow` now depends on the renderer interface/factory seam instead of directly owning `Win32MouseCompanionPlaceholderRenderer` as a concrete-only dependency.
- A renderer-facing input seam is now active too:
  - `Win32MouseCompanionRendererInput`
  - `Win32MouseCompanionRendererInputBuilder`
- The current placeholder motion/posture/action-profile/scene path now trends toward renderer-input consumption instead of reading the whole host visual state directly.
- The placeholder backend has also split its pure paint path out:
  - `Win32MouseCompanionPlaceholderRenderer` is now the backend coordinator
  - `Win32MouseCompanionPlaceholderSceneBuilder` owns placeholder scene assembly
  - `Win32MouseCompanionPlaceholderPainter` owns GDI+ drawing only
- Runtime interpretation is now moving behind a reusable adapter too:
  - `Win32MouseCompanionRendererRuntime`
  - placeholder helper layers now consume a normalized runtime view instead of repeatedly decoding raw renderer input
- Backend selection is now observable through runtime diagnostics:
  - `preferred_renderer_backend_source`
  - `preferred_renderer_backend`
  - `selected_renderer_backend`
  - `renderer_backend_selection_reason`
  - `renderer_backend_failure_reason`
  - `available_renderer_backends`
  - `unavailable_renderer_backends`
  - `renderer_backend_catalog`
  - `real_renderer_preview`
  - `renderer_runtime_*`
  - `configured_renderer_backend_preference_effective`
  - `configured_renderer_backend_preference_status`
  - this gives future backend bring-up a stable way to verify host/factory/registry routing before any visual parity work starts
  - `unavailable_renderer_backends` is reserved for registered-but-currently-unavailable backends so future real renderers can publish runtime gating reasons without faking constructor/start failures
  - the first concrete use of that lane now exists: backend name `real` is registered with a complete internal preview pipeline, but default availability is still gated as `rollout_disabled` so factory/diagnostics behavior can be exercised before wider rollout
  - `renderer_backend_catalog` is the structured backend inventory contract; future renderer rollout should extend it instead of adding more ad-hoc string lists
  - `real_renderer_preview` is the rollout-facing preview summary contract; it should remain stable enough for manual verification and AI-IDE consumption without requiring direct access to renderer-private scene objects
- `renderer_runtime_*` is the backend-owned runtime snapshot contract; it should reflect the last successful renderer input/render state instead of forcing diagnostics to reconstruct preview state only from higher controller layers
- renderer-owned appearance semantics are now also expected to publish plugin provenance in that same runtime snapshot:
  - `appearance_plugin_id`
  - `appearance_plugin_kind`
  - `appearance_plugin_source`
  - future wasm renderer plugins should reuse these keys instead of inventing a second plugin-diagnostics surface
  - `renderer_runtime_frame_count`, `renderer_runtime_last_render_tick_ms`, and renderer surface-size fields are part of the render-proof subset of that contract; future real-backend rollout should preserve their semantics so test automation can detect actual rendered-frame advancement
  - the current test route contract now includes `renderer_runtime_before / renderer_runtime_after / renderer_runtime_delta`; future renderer bring-up should preserve this diff-friendly proof shape instead of forcing callers to reconstruct transitions manually
  - the test route also accepts bounded wait/expect parameters (`wait_for_frame_ms`, `expect_frame_advance`) so proof-of-render can tolerate short asynchronous frame delays while staying explicit and machine-readable
  - a compact `/api/mouse-companion/test-render-proof` route now exists beside dispatch testing; it should remain focused on renderer proof + preview summary, and it now shares the same optional backend/preview expectation checks as the sweep path while `/api/mouse-companion/test-dispatch` remains the heavier end-to-end dispatch contract
  - a compact `/api/mouse-companion/test-render-proof-sweep` route now also exists for bring-up sequencing; it should remain focused on a small fixed proof sequence and reuse the same render-proof/result semantics instead of inventing a second diagnostics schema
  - Windows bring-up now also has a native `.cmd -> .ps1` entrypoint beside the Git Bash helper; future proof-route changes should keep both frontends aligned so Windows testing does not depend on Git Bash availability
  - WebSettings runtime handoff is now a machine-readable contract too: when the app opens WebSettings it writes `%APPDATA%\\MFCMouseEffect\\websettings_runtime_auto.json` with `url/base_url/token/port`, and the Windows-native proof helper should keep consuming that file so Windows bring-up does not depend on manual URL/token copy steps
  - preview palette emphasis is renderer-owned too; future visual tuning should keep action-family tinting behind palette/style contracts instead of scattering ad-hoc color shifts through painter code
  - face focus detailing is renderer-owned too; pupil offset and eye-highlight intensity should remain face-builder inputs derived from runtime motion state rather than controller-side flags or painter-local heuristics
  - whisker focus detailing is renderer-owned too; whisker spread/tilt should remain face-builder outputs derived from runtime motion state rather than another controller-visible action-specific toggle list
  - lightweight body/head cohesion cues such as a neck bridge should remain frame/painter-owned preview details, not a reason to reopen host/runtime contracts or add controller-side geometry flags
  - lightweight front/rear depth cues such as shoulder/hip patches should also remain frame/painter-owned preview details, so silhouette readability improves without creating another controller-visible detail state surface
  - lightweight tail-tip and paw-pad accents should remain appendage/painter-owned preview details too, so material/detail readability improves without creating another controller-visible accent-state surface
  - lightweight tail-root attachment cues such as a rear cuff should remain appendage/painter-owned preview details too, so tail/body cohesion improves without creating another controller-visible attachment-state surface
  - lightweight ear-root attachment cues such as top cuffs should remain appendage/painter-owned preview details too, so ear/head cohesion improves without creating another controller-visible attachment-state surface
  - lightweight cheek/jaw contour cues should remain face-builder/painter-owned preview details too, so lower-head silhouette readability improves without creating another controller-visible contour-state surface
  - lightweight muzzle-pad cues should remain face-builder/painter-owned preview details too, so nose/mouth plane readability improves without creating another controller-visible muzzle-state surface
  - lightweight forehead-pad cues should remain face-builder/painter-owned preview details too, so brow/eye-zone readability improves without creating another controller-visible forehead-state surface
  - lightweight temple-contour cues should remain face-builder/painter-owned preview details too, so eye-socket readability improves without creating another controller-visible temple-state surface
  - lightweight under-eye contour cues should remain face-builder/painter-owned preview details too, so eye-socket continuity improves without creating another controller-visible under-eye-state surface
  - lightweight nose-bridge cues should remain face-builder/painter-owned preview details too, so brow-to-muzzle continuity improves without creating another controller-visible nose-bridge-state surface
  - lightweight belly/flank contour cues should remain frame-builder/painter-owned preview details too, so torso volume improves without creating another controller-visible body-contour-state surface
  - lightweight sternum/torso-centerline cues should remain frame-builder/painter-owned preview details too, so chest-to-belly continuity improves without creating another controller-visible torso-centerline-state surface
  - lightweight upper-torso contour cues should remain frame-builder/painter-owned preview details too, so shoulder-to-chest continuity improves without creating another controller-visible upper-torso-state surface
  - lightweight back-contour cues should remain frame-builder/painter-owned preview details too, so torso front/back readability improves without creating another controller-visible rear-torso-state surface
  - lightweight hand/leg root-cuff cues should remain appendage-builder/painter-owned preview details too, so limb attachment continuity improves without creating another controller-visible limb-root-state surface
  - lightweight hand/leg silhouette-bridge cues should remain appendage-builder/painter-owned preview details too, so limb outer-contour continuity improves without creating another controller-visible limb-silhouette-state surface
  - action-aware proportion tuning should remain frame/appendage-builder-owned, so head scale, hand reach, and leg stance can bias by runtime state without creating another controller-visible proportion-state surface
  - action-aware body-stance tuning should remain frame-builder-owned, so torso width/height/center lift can bias by runtime state without creating another controller-visible body-stance-state surface
  - action-aware appendage-proportion tuning should remain appendage-builder-owned, so tail width/height and ear spread/lift can bias by runtime state without creating another controller-visible appendage-proportion-state surface
  - action-aware atmosphere-grounding tuning should remain frame-builder-owned, so glow/shadow/pedestal scale can bias by runtime state without creating another controller-visible atmosphere-grounding-state surface
  - action-aware atmosphere-grounding offset tuning should remain frame-builder-owned, so shadow/pedestal placement can bias by runtime state without creating another controller-visible atmosphere-grounding-offset-state surface
  - action-aware atmosphere-grounding weight tuning should remain palette/frame-owned, so shadow/pedestal alpha can bias by runtime state without creating another controller-visible atmosphere-grounding-weight-state surface
  - action-aware upper-atmosphere tuning should remain frame-builder-owned, so glow alpha/offset can bias by runtime state without creating another controller-visible upper-atmosphere-state surface
  - click/scroll atmosphere tuning should remain frame/palette-owned, so glow/shadow/pedestal responses can expand to those states without creating another controller-visible atmosphere-state matrix
  - atmosphere tint tuning should remain palette-owned, so shadow/pedestal hue can bias by runtime state without creating another controller-visible atmosphere-tint-state surface
  - the first real-renderer requirement seam is now active too:
    - `Win32MouseCompanionRealRendererAssetResources`
    - it adapts shared `model / action_library / appearance_profile` lanes into a renderer-facing resource contract
  - the second real-renderer requirement seam is now active too:
    - `Win32MouseCompanionRealRendererSceneRuntime`
    - it adapts shared runtime action / pose / facing state into a renderer-facing scene-runtime contract
  - the third seam is now active too:
    - `Win32MouseCompanionRealRendererSceneBuilder`
    - `Win32MouseCompanionRealRendererPainter`
    - `Win32MouseCompanionRealRendererBackend::Render(...)`
    - the current output is still a preview backend, but it now renders a stylized pet-like scene with ears/limbs/face/accessory markers instead of only abstract readiness geometry
    - action-specific visual overlays are now part of that preview contract too, and they intentionally stay renderer-owned rather than leaking back into controller logic:
      - `click` -> ring overlay
      - `hold` -> grip band
      - `scroll` -> orbit arc
      - `follow` -> trail overlay
      - `drag` -> motion slash
      - overlay stroke/alpha emphasis is now also renderer-owned, so preview intensity changes remain visible without adding a second controller-side visual-strength contract
    - face expression is now part of the same renderer-owned preview contract:
      - brows tilt/lift per action
      - mouth arc shape changes per action
      - blush intensity can vary with reactive/click state
    - whole-body posture is now part of that renderer-owned preview contract too:
      - body center can lift/drop per action
      - head can nod/lean independently
      - tail can lift/sag
      - shadow scale can compress/spread with the apparent weight shift
      - limb placement can change with follow/hold cadence
    - silhouette emphasis is now part of that renderer-owned preview contract too:
      - body/head/limb stroke weight can vary with action intensity
      - chest emphasis can tighten/soften with action state
    - atmosphere emphasis is now part of that renderer-owned preview contract too:
      - glow size can expand with action intensity
      - shadow/pedestal alpha can respond to follow/hold emphasis
      - accent visibility can strengthen with action intensity
    - action rhythm is now part of that renderer-owned preview contract too:
      - click can carry a rebound pulse
      - hold can carry a squeeze cadence
      - scroll can carry a bob/swirl cadence
      - drag can carry a pull pulse
      - follow can carry a gait rhythm
    - appendage coordination is now part of that renderer-owned preview contract too:
      - ears, tail, hands, and legs can reuse those action phases instead of only reading independent static offsets
    - idle life rhythm is also part of the preview contract:
      - lightweight breathing can modulate body/shadow scale
      - ears can keep a subtle cadence
      - tail can sway slightly
      - hands can float slightly
      - this rhythm should remain renderer-local and reuse existing runtime ticks rather than requiring a second idle animation subsystem
    - motion tuning now has a dedicated `Win32MouseCompanionRealRendererMotionProfile` seam:
      - it owns action-strength curves and idle rhythm shaping
      - `SceneBuilder` should consume that profile rather than growing a second formula bucket
    - overlay geometry now has a dedicated `Win32MouseCompanionRealRendererActionOverlayBuilder` seam:
      - it owns `click / hold / scroll / drag / follow` overlay placement and shape rules
      - `SceneBuilder` should delegate overlay assembly there so body/head/limb layout stays the stable core scene seam
    - face geometry now has a dedicated `Win32MouseCompanionRealRendererFaceBuilder` seam:
      - it owns brow/eye/mouth/blush placement derived from the motion profile
      - `SceneBuilder` should delegate expressive face assembly there so posture/layout and expression tuning evolve independently
    - accessory/badge assembly now has a dedicated `Win32MouseCompanionRealRendererAdornmentBuilder` seam:
      - it owns lane badges, pose badge, and accessory marker placement
      - `SceneBuilder` should delegate those adornment concerns there so the core scene seam stays centered on body/head/limb geometry
    - palette assignment now has a dedicated `Win32MouseCompanionRealRendererPaletteBuilder` seam:
      - it owns skin/theme/status-derived colors and material-like fill/stroke choices
      - `SceneBuilder` should delegate those presentation choices there so geometry and visual theming remain independently tunable
      - renderer-owned palette tokens now also travel through `Win32MouseCompanionRealRendererPaletteProfile`, so theme constants can evolve without reintroducing color literals into the builder itself
    - appendage geometry now has a dedicated `Win32MouseCompanionRealRendererAppendageBuilder` seam:
      - it owns tail/ear/hand/leg placement driven by pose samples and the motion profile
      - `SceneBuilder` should delegate that appendage assembly there so the core scene seam stays focused on body/head/frame layout
    - core frame geometry now has a dedicated `Win32MouseCompanionRealRendererFrameBuilder` seam:
      - it owns body/head/shadow/pedestal layout derived from runtime-facing momentum and the motion profile
      - `SceneBuilder` should delegate that frame assembly there so top-level orchestration remains a composition layer rather than another long-lived geometry bucket
    - shared layout metrics now have a dedicated `Win32MouseCompanionRealRendererLayoutMetrics` contract:
      - builder seams should exchange stable body/head sizing conventions through that struct rather than passing multiple bare float dimensions
      - this keeps future preview geometry evolution localized and makes builder signatures less brittle
    - shared visual ratios now have a dedicated `Win32MouseCompanionRealRendererStyleProfile` contract:
      - builder seams should consume common ratio/scale defaults there rather than duplicating ring/face/body/appendage/adornment sizing constants in each file
      - this keeps future Windows preview tuning centralized and avoids re-coupling geometry seams through copied magic numbers
      - current frame/appendage/face/adornment/overlay builders are expected to keep migrating new visual ratios into that style contract instead of reintroducing local hardcoded scale clusters
      - current face-expression anchors, accessory-star offset rules, and frame/palette tuning values now also belong to that style contract, so preview tuning can stay centralized without reopening builder-local geometry literals
  - current rollout rule:
    - default `real` availability is gated by `MFX_WIN32_MOUSE_COMPANION_REAL_RENDERER_ENABLE`
    - when unset, diagnostics should report `unavailable_reason=rollout_disabled`
    - when set to `1/true/on/yes`, the preview backend becomes selectable for explicit testing
  - both `renderer_backend_catalog[*].unmet_requirements` and top-level `real_renderer_unmet_requirements` should stay stable enough for future automation/manual validation
  - current test-friendly preference source is `env:MFX_WIN32_MOUSE_COMPANION_RENDERER_BACKEND`; when unset, diagnostics report `preferred_renderer_backend_source=default`
  - configured-preference diagnostics tell us whether config-backed preference resolution is currently active; they do not imply that the final selected backend matched the preferred backend
  - runtime-config preference updates now re-enter backend selection after host start; if a replacement backend cannot be created, the current backend stays alive so the host does not drop rendering during preference experiments
  - backend preference normalization is now separated from the factory path, so future config-backed preference sources can reuse the same canonical `default -> auto` and lowercase/trim rules
- Backend lifecycle is now part of the contract too:
  - renderer backend construction alone is not treated as readiness
  - backends are expected to implement `Start()`, `Shutdown()`, `IsReady()`, and `LastErrorReason()`
  - future real backends should surface initialization failures through `LastErrorReason()` so factory fallback remains local to backend selection
- Backend preference source routing is now explicit too:
  - preference resolution no longer assumes only one hardcoded env source
  - current built-ins are ordered as `configured_request -> env -> default`
  - future config-backed preference sources should plug into the registry layer instead of expanding factory-side conditionals
  - explicit caller-provided preference requests now reuse that same registry path, so source precedence stays uniform across test/debug/runtime entrypoints
  - Windows visual host now forwards an internal runtime-config preference request before backend creation; this is intentionally an internal seam first, not a user-facing schema flag yet
  - hidden config/json fields now persist backend preference source/name so future settings-backed preference can be enabled without changing runtime/request plumbing again
  - hidden settings/apply/diagnostics lanes now round-trip those fields too, so future renderer preference rollout no longer depends on adding a new transport path
- Current limitation remains explicit:
  - the placeholder renderer is still the only backend implementation
  - built-in backend selection now routes through an explicit backend registry + registration step, so future Windows backends no longer need another factory/window rewrite
  - presenter/runtime/host behavior is unchanged
  - this step is an architectural handoff seam, not a visual-capability upgrade yet

## Regression Rule
Any future Windows real-renderer landing must preserve:
- shared `IPetVisualHost` usage from `AppController`
- current placement/clamp semantics
- current diagnostics field meanings
- current asset-lane separation (`model / action_library / appearance_profile`)
- current runtime semantic action labels

export function getMouseCompanionSectionMarkup() {
  return `
    <section class="mouse-companion-panel">
      <div class="grid">
        <label for="mc_enabled" class="label-with-tip"><span data-i18n="label_mouse_companion_enabled">Enable Mouse Companion</span></label>
        <label class="startup-toggle" for="mc_enabled">
          <input id="mc_enabled" class="startup-toggle__input" type="checkbox" />
          <span class="startup-toggle__switch" aria-hidden="true"><span class="startup-toggle__thumb"></span></span>
          <span id="mc_enabled_text" class="startup-toggle__text" data-i18n="text_mouse_companion_off">Disabled</span>
        </label>

        <label for="mc_size_px" data-i18n="label_mouse_companion_size_px">Companion Size (px)</label>
        <input id="mc_size_px" type="number" />

        <label for="mc_position_mode" data-i18n="label_mouse_companion_position_mode">Position Mode</label>
        <select id="mc_position_mode"></select>

        <label for="mc_edge_clamp_mode" data-i18n="label_mouse_companion_edge_clamp_mode">Edge Clamp Mode</label>
        <select id="mc_edge_clamp_mode"></select>
      </div>
    </section>

    <details class="mouse-companion-details" open>
      <summary>Runtime Diagnostics</summary>
      <div class="grid">
        <label>Lane Verdict</label>
        <output id="mc_runtime_default_lane_verdict">-</output>

        <label>Default Lane Candidate</label>
        <output id="mc_runtime_default_lane_candidate">-</output>

        <label>Default Lane Source</label>
        <output id="mc_runtime_default_lane_source">-</output>

        <label>Rollout Status</label>
        <output id="mc_runtime_default_lane_rollout_status">-</output>

        <label>Style Intent</label>
        <output id="mc_runtime_default_lane_style_intent">-</output>

        <label>Candidate Tier</label>
        <output id="mc_runtime_default_lane_candidate_tier">-</output>

        <label>Sample Tier</label>
        <output id="mc_runtime_appearance_plugin_sample_tier">-</output>

        <label>Contract Brief</label>
        <output id="mc_runtime_appearance_plugin_contract_brief">-</output>

        <label>Scene Runtime Adapter</label>
        <output id="mc_runtime_scene_runtime_adapter_mode">-</output>

        <label>Model Asset Source</label>
        <output id="mc_runtime_scene_runtime_model_asset_source_brief">-</output>

        <label>Model Asset Paths</label>
        <output id="mc_runtime_scene_runtime_model_asset_source_path_brief">-</output>

        <label>Model Asset Manifest</label>
        <output id="mc_runtime_scene_runtime_model_asset_manifest_brief">-</output>

        <label>Model Manifest Entries</label>
        <output id="mc_runtime_scene_runtime_model_asset_manifest_entry_brief">-</output>

        <label>Model Asset Catalog</label>
        <output id="mc_runtime_scene_runtime_model_asset_catalog_brief">-</output>

        <label>Model Catalog Entries</label>
        <output id="mc_runtime_scene_runtime_model_asset_catalog_entry_brief">-</output>

        <label>Model Asset Binding</label>
        <output id="mc_runtime_scene_runtime_model_asset_binding_table_brief">-</output>

        <label>Model Binding Slots</label>
        <output id="mc_runtime_scene_runtime_model_asset_binding_table_slot_brief">-</output>

        <label>Model Asset Registry</label>
        <output id="mc_runtime_scene_runtime_model_asset_registry_brief">-</output>

        <label>Model Registry Assets</label>
        <output id="mc_runtime_scene_runtime_model_asset_registry_asset_brief">-</output>

        <label>Model Asset Load</label>
        <output id="mc_runtime_scene_runtime_model_asset_load_brief">-</output>

        <label>Model Load Plan</label>
        <output id="mc_runtime_scene_runtime_model_asset_load_plan_brief">-</output>

        <label>Model Asset Decode</label>
        <output id="mc_runtime_scene_runtime_model_asset_decode_brief">-</output>

        <label>Model Decode Pipeline</label>
        <output id="mc_runtime_scene_runtime_model_asset_decode_pipeline_brief">-</output>

        <label>Model Asset Residency</label>
        <output id="mc_runtime_scene_runtime_model_asset_residency_brief">-</output>

        <label>Model Residency Cache</label>
        <output id="mc_runtime_scene_runtime_model_asset_residency_cache_brief">-</output>

        <label>Model Asset Instance</label>
        <output id="mc_runtime_scene_runtime_model_asset_instance_brief">-</output>

        <label>Model Instance Slots</label>
        <output id="mc_runtime_scene_runtime_model_asset_instance_slot_brief">-</output>

        <label>Model Asset Activation</label>
        <output id="mc_runtime_scene_runtime_model_asset_activation_brief">-</output>

        <label>Model Activation Route</label>
        <output id="mc_runtime_scene_runtime_model_asset_activation_route_brief">-</output>

        <label>Model Asset Session</label>
        <output id="mc_runtime_scene_runtime_model_asset_session_brief">-</output>

        <label>Model Session Route</label>
        <output id="mc_runtime_scene_runtime_model_asset_session_session_brief">-</output>

        <label>Model Asset Bind Ready</label>
        <output id="mc_runtime_scene_runtime_model_asset_bind_ready_brief">-</output>

        <label>Model Bind Ready</label>
        <output id="mc_runtime_scene_runtime_model_asset_bind_ready_binding_brief">-</output>

        <label>Model Asset Handle</label>
        <output id="mc_runtime_scene_runtime_model_asset_handle_brief">-</output>

        <label>Model Handle Route</label>
        <output id="mc_runtime_scene_runtime_model_asset_handle_handle_brief">-</output>

        <label>Model Scene Adapter</label>
        <output id="mc_runtime_scene_runtime_model_scene_adapter_brief">-</output>

        <label>Model Scene Readiness</label>
        <output id="mc_runtime_scene_runtime_model_scene_seam_readiness">-</output>

        <label>Model Asset Scene Hook</label>
        <output id="mc_runtime_scene_runtime_model_asset_scene_hook_brief">-</output>

        <label>Model Scene Hook</label>
        <output id="mc_runtime_scene_runtime_model_asset_scene_hook_hook_brief">-</output>

        <label>Model Asset Scene Binding</label>
        <output id="mc_runtime_scene_runtime_model_asset_scene_binding_brief">-</output>

        <label>Model Scene Binding</label>
        <output id="mc_runtime_scene_runtime_model_asset_scene_binding_binding_brief">-</output>

        <label>Model Node Adapter</label>
        <output id="mc_runtime_scene_runtime_model_node_adapter_brief">-</output>

        <label>Model Node Influence</label>
        <output id="mc_runtime_scene_runtime_model_node_adapter_influence">-</output>

        <label>Model Node Channels</label>
        <output id="mc_runtime_scene_runtime_model_node_channel_brief">-</output>

        <label>Model Asset Node Attach</label>
        <output id="mc_runtime_scene_runtime_model_asset_node_attach_brief">-</output>

        <label>Model Node Attach</label>
        <output id="mc_runtime_scene_runtime_model_asset_node_attach_attach_brief">-</output>

        <label>Model Node Graph</label>
        <output id="mc_runtime_scene_runtime_model_node_graph_brief">-</output>

        <label>Bound Model Nodes</label>
        <output id="mc_runtime_scene_runtime_model_node_graph_bound_count">-</output>

        <label>Model Node Binding</label>
        <output id="mc_runtime_scene_runtime_model_node_binding_brief">-</output>

        <label>Bound Model Bindings</label>
        <output id="mc_runtime_scene_runtime_model_node_binding_bound_count">-</output>

        <label>Binding Weights</label>
        <output id="mc_runtime_scene_runtime_model_node_binding_weight_brief">-</output>

        <label>Model Node Slots</label>
        <output id="mc_runtime_scene_runtime_model_node_slot_brief">-</output>

        <label>Ready Node Slots</label>
        <output id="mc_runtime_scene_runtime_model_node_ready_slot_count">-</output>

        <label>Node Slot Names</label>
        <output id="mc_runtime_scene_runtime_model_node_slot_name_brief">-</output>

        <label>Model Node Registry</label>
        <output id="mc_runtime_scene_runtime_model_node_registry_brief">-</output>

        <label>Resolved Registry Entries</label>
        <output id="mc_runtime_scene_runtime_model_node_registry_resolved_count">-</output>

        <label>Registry Asset Nodes</label>
        <output id="mc_runtime_scene_runtime_model_node_registry_asset_node_brief">-</output>

        <label>Registry Weights</label>
        <output id="mc_runtime_scene_runtime_model_node_registry_weight_brief">-</output>

        <label>Asset Node Binding</label>
        <output id="mc_runtime_scene_runtime_asset_node_binding_brief">-</output>

        <label>Resolved Asset Bindings</label>
        <output id="mc_runtime_scene_runtime_asset_node_binding_resolved_count">-</output>

        <label>Asset Node Paths</label>
        <output id="mc_runtime_scene_runtime_asset_node_binding_path_brief">-</output>

        <label>Asset Binding Weights</label>
        <output id="mc_runtime_scene_runtime_asset_node_binding_weight_brief">-</output>

        <label>Asset Node Transform</label>
        <output id="mc_runtime_scene_runtime_asset_node_transform_brief">-</output>

        <label>Resolved Asset Transforms</label>
        <output id="mc_runtime_scene_runtime_asset_node_transform_resolved_count">-</output>

        <label>Asset Transform Paths</label>
        <output id="mc_runtime_scene_runtime_asset_node_transform_path_brief">-</output>

        <label>Asset Transform Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_transform_value_brief">-</output>

        <label>Asset Node Anchor</label>
        <output id="mc_runtime_scene_runtime_asset_node_anchor_brief">-</output>

        <label>Resolved Asset Anchors</label>
        <output id="mc_runtime_scene_runtime_asset_node_anchor_resolved_count">-</output>

        <label>Asset Anchor Points</label>
        <output id="mc_runtime_scene_runtime_asset_node_anchor_point_brief">-</output>

        <label>Asset Anchor Scales</label>
        <output id="mc_runtime_scene_runtime_asset_node_anchor_scale_brief">-</output>

        <label>Asset Node Resolver</label>
        <output id="mc_runtime_scene_runtime_asset_node_resolver_brief">-</output>

        <label>Resolved Node Resolvers</label>
        <output id="mc_runtime_scene_runtime_asset_node_resolver_resolved_count">-</output>

        <label>Node Resolver Parents</label>
        <output id="mc_runtime_scene_runtime_asset_node_resolver_parent_brief">-</output>

        <label>Node Resolver Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_resolver_value_brief">-</output>

        <label>Asset Parent Space</label>
        <output id="mc_runtime_scene_runtime_asset_node_parent_space_brief">-</output>

        <label>Resolved Parent Space Nodes</label>
        <output id="mc_runtime_scene_runtime_asset_node_parent_space_resolved_count">-</output>

        <label>Parent Space Parents</label>
        <output id="mc_runtime_scene_runtime_asset_node_parent_space_parent_brief">-</output>

        <label>Parent Space Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_parent_space_value_brief">-</output>

        <label>Asset Node Target</label>
        <output id="mc_runtime_scene_runtime_asset_node_target_brief">-</output>

        <label>Resolved Target Nodes</label>
        <output id="mc_runtime_scene_runtime_asset_node_target_resolved_count">-</output>

        <label>Target Kinds</label>
        <output id="mc_runtime_scene_runtime_asset_node_target_kind_brief">-</output>

        <label>Target Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_target_value_brief">-</output>

        <label>Asset Target Resolver</label>
        <output id="mc_runtime_scene_runtime_asset_node_target_resolver_brief">-</output>

        <label>Resolved Target Resolver Nodes</label>
        <output id="mc_runtime_scene_runtime_asset_node_target_resolver_resolved_count">-</output>

        <label>Target Resolver Paths</label>
        <output id="mc_runtime_scene_runtime_asset_node_target_resolver_path_brief">-</output>

        <label>Target Resolver Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_target_resolver_value_brief">-</output>

        <label>Asset World Space</label>
        <output id="mc_runtime_scene_runtime_asset_node_world_space_brief">-</output>

        <label>Resolved World Space Nodes</label>
        <output id="mc_runtime_scene_runtime_asset_node_world_space_resolved_count">-</output>

        <label>World Space Paths</label>
        <output id="mc_runtime_scene_runtime_asset_node_world_space_path_brief">-</output>

        <label>World Space Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_world_space_value_brief">-</output>

        <label>Asset Node Pose</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_brief">-</output>

        <label>Resolved Node Poses</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_resolved_count">-</output>

        <label>Node Pose Paths</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_path_brief">-</output>

        <label>Node Pose Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_value_brief">-</output>

        <label>Pose Resolver</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_resolver_brief">-</output>

        <label>Resolved Pose Resolvers</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_resolver_resolved_count">-</output>

        <label>Pose Resolver Paths</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_resolver_path_brief">-</output>

        <label>Pose Resolver Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_resolver_value_brief">-</output>

        <label>Pose Registry</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_registry_brief">-</output>

        <label>Resolved Pose Registry Nodes</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_registry_resolved_count">-</output>

        <label>Pose Registry Nodes</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_registry_node_brief">-</output>

        <label>Pose Registry Weights</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_registry_weight_brief">-</output>

        <label>Pose Channel</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_channel_brief">-</output>

        <label>Resolved Pose Channels</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_channel_resolved_count">-</output>

        <label>Pose Channel Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_channel_name_brief">-</output>

        <label>Pose Channel Weights</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_channel_weight_brief">-</output>

        <label>Pose Constraint</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_constraint_brief">-</output>

        <label>Resolved Pose Constraints</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_constraint_resolved_count">-</output>

        <label>Pose Constraint Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_constraint_name_brief">-</output>

        <label>Pose Constraint Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_constraint_value_brief">-</output>

        <label>Pose Solve</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_solve_brief">-</output>

        <label>Resolved Pose Solves</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_solve_resolved_count">-</output>

        <label>Pose Solve Paths</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_solve_path_brief">-</output>

        <label>Pose Solve Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_solve_value_brief">-</output>

        <label>Joint Hint</label>
        <output id="mc_runtime_scene_runtime_asset_node_joint_hint_brief">-</output>

        <label>Resolved Joint Hints</label>
        <output id="mc_runtime_scene_runtime_asset_node_joint_hint_resolved_count">-</output>

        <label>Joint Hint Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_joint_hint_name_brief">-</output>

        <label>Joint Hint Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_joint_hint_value_brief">-</output>

        <label>Articulation</label>
        <output id="mc_runtime_scene_runtime_asset_node_articulation_brief">-</output>

        <label>Resolved Articulations</label>
        <output id="mc_runtime_scene_runtime_asset_node_articulation_resolved_count">-</output>

        <label>Articulation Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_articulation_name_brief">-</output>

        <label>Articulation Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_articulation_value_brief">-</output>

        <label>Local Joint Registry</label>
        <output id="mc_runtime_scene_runtime_asset_node_local_joint_registry_brief">-</output>

        <label>Resolved Local Joints</label>
        <output id="mc_runtime_scene_runtime_asset_node_local_joint_registry_resolved_count">-</output>

        <label>Local Joint Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_local_joint_registry_joint_brief">-</output>

        <label>Local Joint Weights</label>
        <output id="mc_runtime_scene_runtime_asset_node_local_joint_registry_weight_brief">-</output>

        <label>Articulation Map</label>
        <output id="mc_runtime_scene_runtime_asset_node_articulation_map_brief">-</output>

        <label>Resolved Articulation Maps</label>
        <output id="mc_runtime_scene_runtime_asset_node_articulation_map_resolved_count">-</output>

        <label>Articulation Map Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_articulation_map_name_brief">-</output>

        <label>Articulation Map Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_articulation_map_value_brief">-</output>

        <label>Control Rig Hint</label>
        <output id="mc_runtime_scene_runtime_asset_node_control_rig_hint_brief">-</output>

        <label>Resolved Control Rig Hints</label>
        <output id="mc_runtime_scene_runtime_asset_node_control_rig_hint_resolved_count">-</output>

        <label>Control Rig Hint Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_control_rig_hint_name_brief">-</output>

        <label>Control Rig Hint Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_control_rig_hint_value_brief">-</output>

        <label>Rig Channel</label>
        <output id="mc_runtime_scene_runtime_asset_node_rig_channel_brief">-</output>

        <label>Resolved Rig Channels</label>
        <output id="mc_runtime_scene_runtime_asset_node_rig_channel_resolved_count">-</output>

        <label>Rig Channel Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_rig_channel_name_brief">-</output>

        <label>Rig Channel Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_rig_channel_value_brief">-</output>

        <label>Control Surface</label>
        <output id="mc_runtime_scene_runtime_asset_node_control_surface_brief">-</output>

        <label>Resolved Control Surfaces</label>
        <output id="mc_runtime_scene_runtime_asset_node_control_surface_resolved_count">-</output>

        <label>Control Surface Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_control_surface_name_brief">-</output>

        <label>Control Surface Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_control_surface_value_brief">-</output>

        <label>Rig Driver</label>
        <output id="mc_runtime_scene_runtime_asset_node_rig_driver_brief">-</output>

        <label>Resolved Rig Drivers</label>
        <output id="mc_runtime_scene_runtime_asset_node_rig_driver_resolved_count">-</output>

        <label>Rig Driver Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_rig_driver_name_brief">-</output>

        <label>Rig Driver Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_rig_driver_value_brief">-</output>

        <label>Surface Driver</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_driver_brief">-</output>

        <label>Resolved Surface Drivers</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_driver_resolved_count">-</output>

        <label>Surface Driver Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_driver_name_brief">-</output>

        <label>Surface Driver Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_driver_value_brief">-</output>

        <label>Pose Bus</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_bus_brief">-</output>

        <label>Resolved Pose Bus Entries</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_bus_resolved_count">-</output>

        <label>Pose Bus Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_bus_name_brief">-</output>

        <label>Pose Bus Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_pose_bus_value_brief">-</output>

        <label>Controller Table</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_table_brief">-</output>

        <label>Resolved Controller Entries</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_table_resolved_count">-</output>

        <label>Controller Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_table_name_brief">-</output>

        <label>Controller Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_table_value_brief">-</output>

        <label>Controller Registry</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_registry_brief">-</output>

        <label>Resolved Controller Registry Entries</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_registry_resolved_count">-</output>

        <label>Controller Registry Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_registry_name_brief">-</output>

        <label>Controller Registry Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_registry_value_brief">-</output>

        <label>Driver Bus</label>
        <output id="mc_runtime_scene_runtime_asset_node_driver_bus_brief">-</output>

        <label>Resolved Driver Bus Entries</label>
        <output id="mc_runtime_scene_runtime_asset_node_driver_bus_resolved_count">-</output>

        <label>Driver Bus Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_driver_bus_name_brief">-</output>

        <label>Driver Bus Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_driver_bus_value_brief">-</output>

        <label>Controller Driver Registry</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_driver_registry_brief">-</output>

        <label>Resolved Controller Driver Entries</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_driver_registry_resolved_count">-</output>

        <label>Controller Driver Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_driver_registry_name_brief">-</output>

        <label>Controller Driver Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_driver_registry_value_brief">-</output>

        <label>Execution Lane</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_lane_brief">-</output>

        <label>Resolved Execution Lanes</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_lane_resolved_count">-</output>

        <label>Execution Lane Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_lane_name_brief">-</output>

        <label>Execution Lane Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_lane_value_brief">-</output>

        <label>Controller Phase</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_phase_brief">-</output>

        <label>Resolved Controller Phases</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_phase_resolved_count">-</output>

        <label>Controller Phase Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_phase_name_brief">-</output>

        <label>Controller Phase Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_phase_value_brief">-</output>

        <label>Execution Surface</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_surface_brief">-</output>

        <label>Resolved Execution Surfaces</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_surface_resolved_count">-</output>

        <label>Execution Surface Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_surface_name_brief">-</output>

        <label>Execution Surface Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_surface_value_brief">-</output>

        <label>Controller Phase Registry</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_phase_registry_brief">-</output>

        <label>Resolved Phase Registry Entries</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_phase_registry_resolved_count">-</output>

        <label>Controller Phase Registry Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_phase_registry_name_brief">-</output>

        <label>Controller Phase Registry Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_controller_phase_registry_value_brief">-</output>

        <label>Surface Composition Bus</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_composition_bus_brief">-</output>

        <label>Resolved Surface Bus Entries</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_composition_bus_resolved_count">-</output>

        <label>Surface Composition Bus Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_composition_bus_name_brief">-</output>

        <label>Surface Composition Bus Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_composition_bus_value_brief">-</output>

        <label>Execution Stack</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_stack_brief">-</output>

        <label>Resolved Execution Stack Nodes</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_stack_resolved_count">-</output>

        <label>Execution Stack Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_stack_name_brief">-</output>

        <label>Execution Stack Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_stack_value_brief">-</output>

        <label>Execution Stack Router</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_stack_router_brief">-</output>

        <label>Resolved Execution Stack Routers</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_stack_router_resolved_count">-</output>

        <label>Execution Stack Router Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_stack_router_name_brief">-</output>

        <label>Execution Stack Router Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_stack_router_value_brief">-</output>

        <label>Execution Stack Router Registry</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_stack_router_registry_brief">-</output>

        <label>Resolved Execution Stack Router Registries</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_stack_router_registry_resolved_count">-</output>

        <label>Execution Stack Router Registry Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_stack_router_registry_name_brief">-</output>

        <label>Execution Stack Router Registry Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_stack_router_registry_value_brief">-</output>

        <label>Composition Registry</label>
        <output id="mc_runtime_scene_runtime_asset_node_composition_registry_brief">-</output>

        <label>Resolved Composition Registry Nodes</label>
        <output id="mc_runtime_scene_runtime_asset_node_composition_registry_resolved_count">-</output>

        <label>Composition Registry Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_composition_registry_name_brief">-</output>

        <label>Composition Registry Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_composition_registry_value_brief">-</output>

        <label>Surface Route</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_brief">-</output>

        <label>Resolved Surface Routes</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_resolved_count">-</output>

        <label>Surface Route Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_name_brief">-</output>

        <label>Surface Route Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_value_brief">-</output>

        <label>Surface Route Registry</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_registry_brief">-</output>

        <label>Resolved Surface Route Registries</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_registry_resolved_count">-</output>

        <label>Surface Route Registry Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_registry_name_brief">-</output>

        <label>Surface Route Registry Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_registry_value_brief">-</output>

        <label>Surface Route Router Bus</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_router_bus_brief">-</output>

        <label>Resolved Surface Route Router Buses</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_router_bus_resolved_count">-</output>

        <label>Surface Route Router Bus Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_router_bus_name_brief">-</output>

        <label>Surface Route Router Bus Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_router_bus_value_brief">-</output>

        <label>Surface Route Bus Registry</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_registry_brief">-</output>

        <label>Resolved Surface Route Bus Registries</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_registry_resolved_count">-</output>

        <label>Surface Route Bus Registry Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_registry_name_brief">-</output>

        <label>Surface Route Bus Registry Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_registry_value_brief">-</output>

        <label>Surface Route Bus Driver</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_brief">-</output>

        <label>Resolved Surface Route Bus Drivers</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_resolved_count">-</output>

        <label>Surface Route Bus Driver Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_name_brief">-</output>

        <label>Surface Route Bus Driver Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_value_brief">-</output>

        <label>Surface Route Bus Driver Registry</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_brief">-</output>

        <label>Resolved Surface Route Bus Driver Registries</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_resolved_count">-</output>

        <label>Surface Route Bus Driver Registry Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_name_brief">-</output>

        <label>Surface Route Bus Driver Registry Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_value_brief">-</output>

        <label>Surface Route Bus Driver Registry Router</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_brief">-</output>

        <label>Resolved Surface Route Bus Driver Registry Routers</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_resolved_count">-</output>

        <label>Surface Route Bus Driver Registry Router Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_name_brief">-</output>

        <label>Surface Route Bus Driver Registry Router Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_surface_route_bus_driver_registry_router_value_brief">-</output>

        <label>Execution Driver Table</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_table_brief">-</output>

        <label>Resolved Execution Drivers</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_table_resolved_count">-</output>

        <label>Execution Driver Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_table_name_brief">-</output>

        <label>Execution Driver Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_table_value_brief">-</output>

        <label>Execution Driver Router Table</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_table_brief">-</output>

        <label>Resolved Execution Driver Routers</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_table_resolved_count">-</output>

        <label>Execution Driver Router Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_table_name_brief">-</output>

        <label>Execution Driver Router Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_table_value_brief">-</output>

        <label>Execution Driver Router Registry</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_brief">-</output>

        <label>Resolved Execution Driver Router Registries</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_resolved_count">-</output>

        <label>Execution Driver Router Registry Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_name_brief">-</output>

        <label>Execution Driver Router Registry Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_value_brief">-</output>

        <label>Execution Driver Router Registry Bus</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_brief">-</output>

        <label>Resolved Execution Driver Router Registry Buses</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_resolved_count">-</output>

        <label>Execution Driver Router Registry Bus Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_name_brief">-</output>

        <label>Execution Driver Router Registry Bus Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_value_brief">-</output>

        <label>Execution Driver Router Registry Bus Registry</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_brief">-</output>

        <label>Resolved Execution Driver Router Registry Bus Registries</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_resolved_count">-</output>

        <label>Execution Driver Router Registry Bus Registry Names</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_name_brief">-</output>

        <label>Execution Driver Router Registry Bus Registry Values</label>
        <output id="mc_runtime_scene_runtime_asset_node_execution_driver_router_registry_bus_registry_value_brief">-</output>

        <label>Pose Adapter Brief</label>
        <output id="mc_runtime_scene_runtime_pose_adapter_brief">-</output>

        <label>Pose Adapter Influence</label>
        <output id="mc_runtime_scene_runtime_pose_adapter_influence">-</output>

        <label>Pose Readability Bias</label>
        <output id="mc_runtime_scene_runtime_pose_readability_bias">-</output>

        <label>Pose Samples</label>
        <output id="mc_runtime_scene_runtime_pose_sample_count">-</output>

        <label>Bound Pose Samples</label>
        <output id="mc_runtime_scene_runtime_bound_pose_sample_count">-</output>

        <label>Appearance Plugin Kind</label>
        <output id="mc_runtime_appearance_plugin_kind">-</output>

        <label>Appearance Semantics Mode</label>
        <output id="mc_runtime_appearance_semantics_mode">-</output>

        <label>Plugin Selection Reason</label>
        <output id="mc_runtime_appearance_plugin_selection_reason">-</output>
      </div>
    </details>

    <details class="mouse-companion-details" open>
      <summary data-i18n="summary_mouse_companion_placement">Placement</summary>
      <div class="grid">
        <label for="mc_offset_x" data-i18n="label_mouse_companion_offset_x">Offset X</label>
        <div id="mc_relative_offset_pair" class="grid" style="grid-template-columns: 1fr 1fr; gap: 8px;">
          <input id="mc_offset_x" type="number" />
          <input id="mc_offset_y" type="number" data-i18n-placeholder="label_mouse_companion_offset_y" placeholder="Offset Y" />
        </div>

        <label for="mc_absolute_x" data-i18n="label_mouse_companion_absolute_position">Absolute Position</label>
        <div id="mc_absolute_pair" class="grid" style="grid-template-columns: 1fr 1fr; gap: 8px;">
          <input id="mc_absolute_x" type="number" data-i18n-placeholder="placeholder_mouse_companion_absolute_x" placeholder="Absolute X" />
          <input id="mc_absolute_y" type="number" data-i18n-placeholder="placeholder_mouse_companion_absolute_y" placeholder="Absolute Y" />
        </div>

        <label for="mc_target_monitor" data-i18n="label_mouse_companion_target_monitor">Target Monitor</label>
        <select id="mc_target_monitor"></select>
      </div>
    </details>

    <details class="mouse-companion-details">
      <summary data-i18n="summary_mouse_companion_asset_paths">Asset Paths</summary>
      <div class="grid">
        <label for="mc_model_path" class="label-with-tip"><span data-i18n="label_mouse_companion_model_path">Model Path (USDZ/GLB)</span></label>
        <input id="mc_model_path" type="text" data-i18n-placeholder="placeholder_mouse_companion_model_path" placeholder="MFCMouseEffect/Assets/Pet3D/source/pet-main.glb" />

        <label for="mc_action_library_path" class="label-with-tip"><span data-i18n="label_mouse_companion_action_library_path">Action Library Path (JSON)</span></label>
        <input id="mc_action_library_path" type="text" data-i18n-placeholder="placeholder_mouse_companion_action_library_path" placeholder="MFCMouseEffect/Assets/Pet3D/source/pet-actions.json" />

        <label for="mc_appearance_profile_path" class="label-with-tip"><span data-i18n="label_mouse_companion_appearance_profile_path">Appearance Profile Path (JSON)</span></label>
        <input id="mc_appearance_profile_path" type="text" data-i18n-placeholder="placeholder_mouse_companion_appearance_profile_path" placeholder="MFCMouseEffect/Assets/Pet3D/source/pet-appearance.json" />
      </div>
    </details>

    <details class="mouse-companion-details">
      <summary data-i18n="summary_mouse_companion_advanced_motion">Advanced Motion</summary>
      <div class="grid">
        <label for="mc_press_lift_px" data-i18n="label_mouse_companion_press_lift_px">Press Lift (px)</label>
        <input id="mc_press_lift_px" type="number" />

        <label for="mc_smoothing_percent" data-i18n="label_mouse_companion_smoothing_percent">Follow Smoothing (%)</label>
        <input id="mc_smoothing_percent" type="number" />

        <label for="mc_follow_threshold_px" data-i18n="label_mouse_companion_follow_threshold_px">Follow Threshold (px)</label>
        <input id="mc_follow_threshold_px" type="number" />

        <label for="mc_release_hold_ms" data-i18n="label_mouse_companion_release_hold_ms">Release Hold (ms)</label>
        <input id="mc_release_hold_ms" type="number" />

        <label for="mc_click_streak_break_ms" data-i18n="label_mouse_companion_click_streak_break_ms">Click Streak Break (ms)</label>
        <input id="mc_click_streak_break_ms" type="number" />

        <label for="mc_head_tint_per_click" data-i18n="label_mouse_companion_head_tint_per_click">Head Tint Per Click (0~1)</label>
        <input id="mc_head_tint_per_click" type="number" step="0.01" min="0.01" max="1" />

        <label for="mc_head_tint_max" data-i18n="label_mouse_companion_head_tint_max">Head Tint Max (0~1)</label>
        <input id="mc_head_tint_max" type="number" step="0.01" min="0.01" max="1" />

        <label for="mc_head_tint_decay_per_second" data-i18n="label_mouse_companion_head_tint_decay_per_second">Head Tint Decay (/s)</label>
        <input id="mc_head_tint_decay_per_second" type="number" step="0.01" min="0.05" max="4" />
      </div>
    </details>
  `;
}

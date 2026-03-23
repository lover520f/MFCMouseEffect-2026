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

        <label>Sample Tier</label>
        <output id="mc_runtime_appearance_plugin_sample_tier">-</output>

        <label>Contract Brief</label>
        <output id="mc_runtime_appearance_plugin_contract_brief">-</output>

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

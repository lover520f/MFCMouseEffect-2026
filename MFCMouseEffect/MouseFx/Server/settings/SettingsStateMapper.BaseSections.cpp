#include "pch.h"
#include "SettingsStateMapper.BaseSections.h"

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Config/EffectConfigInternal.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Utils/StringUtils.h"

using json = nlohmann::json;

namespace mousefx {

void AppendBaseSettingsState(const EffectConfig& cfg, json* out) {
    if (!out) {
        return;
    }

    (*out)["ui_language"] = EnsureUtf8(cfg.uiLanguage);
    (*out)["theme"] = EnsureUtf8(cfg.theme);
    (*out)["theme_catalog_root_path"] = EnsureUtf8(cfg.themeCatalogRootPath);
    (*out)["overlay_target_fps"] = config_internal::SanitizeOverlayTargetFps(cfg.overlayTargetFps);
    (*out)["launch_at_startup"] = cfg.launchAtStartup;
    const ThemeCatalogRuntimeInfo themeCatalogRuntime = GetThemeCatalogRuntimeInfo();
    (*out)["theme_catalog_runtime"] = {
        {"configured_root_path", themeCatalogRuntime.configuredRootPath},
        {"built_in_theme_count", themeCatalogRuntime.builtInThemeCount},
        {"runtime_theme_count", themeCatalogRuntime.runtimeThemeCount},
        {"scanned_external_theme_files", themeCatalogRuntime.scannedExternalThemeFiles},
        {"external_theme_count", themeCatalogRuntime.externalThemeCount},
        {"rejected_external_theme_files", themeCatalogRuntime.rejectedExternalThemeFiles},
    };
    (*out)["hold_follow_mode"] = EnsureUtf8(cfg.holdFollowMode);
    (*out)["hold_presenter_backend"] = EnsureUtf8(cfg.holdPresenterBackend);
    {
        json arr = json::array();
        const auto apps = config_internal::SanitizeEffectsBlacklistApps(cfg.effectsBlacklistApps);
        for (const auto& app : apps) {
            arr.push_back(EnsureUtf8(app));
        }
        (*out)["effects_blacklist_apps"] = std::move(arr);
    }
    (*out)["active"] = {
        {"click", EnsureUtf8(cfg.active.click)},
        {"trail", EnsureUtf8(cfg.active.trail)},
        {"scroll", EnsureUtf8(cfg.active.scroll)},
        {"hold", EnsureUtf8(cfg.active.hold)},
        {"hover", EnsureUtf8(cfg.active.hover)},
    };
    const EffectSizeScaleConfig scales = config_internal::SanitizeEffectSizeScaleConfig(cfg.effectSizeScales);
    (*out)["effect_size_scales"] = {
        {"click", scales.click},
        {"trail", scales.trail},
        {"scroll", scales.scroll},
        {"hold", scales.hold},
        {"hover", scales.hover},
    };
    const EffectConflictPolicyConfig conflictPolicy =
        config_internal::SanitizeEffectConflictPolicyConfig(cfg.effectConflictPolicy);
    (*out)["effect_conflict_policy"] = {
        {"hold_move_policy", EnsureUtf8(conflictPolicy.holdMovePolicy)},
    };

    std::string text;
    for (size_t i = 0; i < cfg.textClick.texts.size(); ++i) {
        std::string utf8 = Utf16ToUtf8(cfg.textClick.texts[i].c_str());
        if (i > 0) {
            text += ",";
        }
        text += utf8;
    }
    (*out)["text_content"] = text;
    (*out)["text_font_size"] = cfg.textClick.fontSize;

    (*out)["trail_style"] = EnsureUtf8(cfg.trailStyle);
    (*out)["trail_line_width"] = cfg.trail.lineWidth;
    (*out)["trail_profiles"] = {
        {"line", {{"duration_ms", cfg.trailProfiles.line.durationMs}, {"max_points", cfg.trailProfiles.line.maxPoints}}},
        {"streamer", {{"duration_ms", cfg.trailProfiles.streamer.durationMs}, {"max_points", cfg.trailProfiles.streamer.maxPoints}}},
        {"electric", {{"duration_ms", cfg.trailProfiles.electric.durationMs}, {"max_points", cfg.trailProfiles.electric.maxPoints}}},
        {"meteor", {{"duration_ms", cfg.trailProfiles.meteor.durationMs}, {"max_points", cfg.trailProfiles.meteor.maxPoints}}},
        {"tubes", {{"duration_ms", cfg.trailProfiles.tubes.durationMs}, {"max_points", cfg.trailProfiles.tubes.maxPoints}}},
    };

    (*out)["trail_params"] = {
        {"streamer", {{"glow_width_scale", cfg.trailParams.streamer.glowWidthScale}, {"core_width_scale", cfg.trailParams.streamer.coreWidthScale}, {"head_power", cfg.trailParams.streamer.headPower}}},
        {"electric", {{"amplitude_scale", cfg.trailParams.electric.amplitudeScale}, {"fork_chance", cfg.trailParams.electric.forkChance}}},
        {"meteor", {{"spark_rate_scale", cfg.trailParams.meteor.sparkRateScale}, {"spark_speed_scale", cfg.trailParams.meteor.sparkSpeedScale}}},
        {"idle_fade_start_ms", cfg.trailParams.idleFade.startMs},
        {"idle_fade_end_ms", cfg.trailParams.idleFade.endMs},
    };

    const MouseCompanionConfig companion = config_internal::SanitizeMouseCompanionConfig(cfg.mouseCompanion);
    (*out)["mouse_companion"] = {
        {"enabled", companion.enabled},
        {"model_path", EnsureUtf8(companion.modelPath)},
        {"action_library_path", EnsureUtf8(companion.actionLibraryPath)},
        {"appearance_profile_path", EnsureUtf8(companion.appearanceProfilePath)},
        {"position_mode", EnsureUtf8(companion.positionMode)},
        {"edge_clamp_mode", EnsureUtf8(companion.edgeClampMode)},
        {"size_px", companion.sizePx},
        {"offset_x", companion.offsetX},
        {"offset_y", companion.offsetY},
        {"absolute_x", companion.absoluteX},
        {"absolute_y", companion.absoluteY},
        {"target_monitor", EnsureUtf8(companion.targetMonitor)},
        {"press_lift_px", companion.pressLiftPx},
        {"smoothing_percent", companion.smoothingPercent},
        {"follow_threshold_px", companion.followThresholdPx},
        {"release_hold_ms", companion.releaseHoldMs},
        {"face_pointer_enabled", companion.facePointerEnabled},
        {"click_streak_break_ms", companion.clickStreakBreakMs},
        {"head_tint_per_click", companion.headTintPerClick},
        {"head_tint_max", companion.headTintMax},
        {"head_tint_decay_per_second", companion.headTintDecayPerSecond},
        {"use_test_profile", companion.useTestProfile},
        {"test_press_lift_px", companion.testPressLiftPx},
        {"test_smoothing_percent", companion.testSmoothingPercent},
        {"test_click_streak_break_ms", companion.testClickStreakBreakMs},
        {"test_head_tint_per_click", companion.testHeadTintPerClick},
        {"test_head_tint_max", companion.testHeadTintMax},
        {"test_head_tint_decay_per_second", companion.testHeadTintDecayPerSecond},
    };

    (*out)["input_indicator"] = {
        {"enabled", cfg.inputIndicator.enabled},
        {"keyboard_enabled", cfg.inputIndicator.keyboardEnabled},
        {"render_mode", EnsureUtf8(cfg.inputIndicator.renderMode)},
        {"wasm_fallback_to_native", cfg.inputIndicator.wasmFallbackToNative},
        {"wasm_manifest_path", EnsureUtf8(cfg.inputIndicator.wasmManifestPath)},
        {"position_mode", EnsureUtf8(cfg.inputIndicator.positionMode)},
        {"offset_x", cfg.inputIndicator.offsetX},
        {"offset_y", cfg.inputIndicator.offsetY},
        {"absolute_x", cfg.inputIndicator.absoluteX},
        {"absolute_y", cfg.inputIndicator.absoluteY},
        {"target_monitor", EnsureUtf8(cfg.inputIndicator.targetMonitor)},
        {"key_display_mode", cfg.inputIndicator.keyDisplayMode},
        {"key_label_layout_mode", cfg.inputIndicator.keyLabelLayoutMode},
        {"per_monitor_overrides", [&]() {
            json j = json::object();
            for (auto& [k, v] : cfg.inputIndicator.perMonitorOverrides) {
                j[k] = {
                    {"enabled", v.enabled},
                    {"absolute_x", v.absoluteX},
                    {"absolute_y", v.absoluteY},
                };
            }
            return j;
        }()},
        {"size_px", cfg.inputIndicator.sizePx},
        {"duration_ms", cfg.inputIndicator.durationMs}
    };

    (*out)["automation"] = {
        {"enabled", cfg.automation.enabled},
        {"mouse_mappings", [&]() {
            json arr = json::array();
            for (const auto& binding : cfg.automation.mouseMappings) {
                json scopes = json::array();
                for (const auto& scope : binding.appScopes) {
                    scopes.push_back(EnsureUtf8(scope));
                }
                const std::string legacyScope = scopes.empty()
                    ? std::string("all")
                    : scopes.front().get<std::string>();
                arr.push_back({
                    {"enabled", binding.enabled},
                    {"trigger", EnsureUtf8(binding.trigger)},
                    {"app_scope", legacyScope},
                    {"app_scopes", scopes},
                    {"gesture_pattern", {
                        {"mode", EnsureUtf8(binding.gesturePattern.mode)},
                        {"match_threshold_percent", binding.gesturePattern.matchThresholdPercent},
                        {"custom_points", [&]() {
                            json points = json::array();
                            for (const auto& point : binding.gesturePattern.customPoints) {
                                points.push_back({
                                    {"x", point.x},
                                    {"y", point.y},
                                });
                            }
                            return points;
                        }()},
                    }},
                    {"modifiers", {
                        {"mode", EnsureUtf8(binding.modifiers.mode)},
                        {"primary", binding.modifiers.primary},
                        {"shift", binding.modifiers.shift},
                        {"alt", binding.modifiers.alt},
                    }},
                    {"keys", EnsureUtf8(binding.keys)},
                });
            }
            return arr;
        }()},
        {"gesture", {
            {"enabled", cfg.automation.gesture.enabled},
            {"trigger_button", EnsureUtf8(cfg.automation.gesture.triggerButton)},
            {"min_stroke_distance_px", cfg.automation.gesture.minStrokeDistancePx},
            {"sample_step_px", cfg.automation.gesture.sampleStepPx},
            {"max_directions", cfg.automation.gesture.maxDirections},
            {"mappings", [&]() {
                json arr = json::array();
                for (const auto& binding : cfg.automation.gesture.mappings) {
                    json scopes = json::array();
                    for (const auto& scope : binding.appScopes) {
                        scopes.push_back(EnsureUtf8(scope));
                    }
                    const std::string legacyScope = scopes.empty()
                        ? std::string("all")
                        : scopes.front().get<std::string>();
                    arr.push_back({
                        {"enabled", binding.enabled},
                        {"trigger", EnsureUtf8(binding.trigger)},
                        {"trigger_button", EnsureUtf8(binding.triggerButton)},
                        {"app_scope", legacyScope},
                        {"app_scopes", scopes},
                        {"gesture_pattern", {
                            {"mode", EnsureUtf8(binding.gesturePattern.mode)},
                            {"match_threshold_percent", binding.gesturePattern.matchThresholdPercent},
                            {"custom_points", [&]() {
                                json points = json::array();
                                for (const auto& point : binding.gesturePattern.customPoints) {
                                    points.push_back({
                                        {"x", point.x},
                                        {"y", point.y},
                                    });
                                }
                                return points;
                            }()},
                        }},
                        {"modifiers", {
                            {"mode", EnsureUtf8(binding.modifiers.mode)},
                            {"primary", binding.modifiers.primary},
                            {"shift", binding.modifiers.shift},
                            {"alt", binding.modifiers.alt},
                        }},
                        {"keys", EnsureUtf8(binding.keys)},
                    });
                }
                return arr;
            }()},
        }},
    };
}

} // namespace mousefx

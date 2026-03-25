#include "pch.h"

#include "CommandHandler.ApplySettings.Internal.h"

#include "AppController.h"
#include "MouseFx/Core/Config/EffectConfigInternal.h"
#include "MouseFx/Utils/MathUtils.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Settings/SettingsOptions.h"

#include <array>

namespace mousefx::command_handler_apply_settings {
namespace {

std::vector<std::wstring> ParseCsvUtf8TextList(const std::string& raw) {
    std::vector<std::wstring> texts;
    size_t start = 0;
    size_t end = raw.find(',');
    while (end != std::string::npos) {
        const std::string token = TrimAscii(raw.substr(start, end - start));
        if (!token.empty()) {
            texts.push_back(Utf8ToWString(token));
        }
        start = end + 1;
        end = raw.find(',', start);
    }
    const std::string last = TrimAscii(raw.substr(start));
    if (!last.empty()) {
        texts.push_back(Utf8ToWString(last));
    }
    return texts;
}

const EffectOption* ResolveEffectOptionByInput(EffectCategory category, const std::string& raw) {
    const std::string trimmed = TrimAscii(raw);
    if (trimmed.empty()) {
        return nullptr;
    }

    size_t count = 0;
    const EffectOption* options = nullptr;
    switch (category) {
    case EffectCategory::Click:
        options = mousefx::ClickMetadata(count);
        break;
    case EffectCategory::Trail:
        options = mousefx::TrailMetadata(count);
        break;
    case EffectCategory::Scroll:
        options = mousefx::ScrollMetadata(count);
        break;
    case EffectCategory::Hold:
        options = mousefx::HoldMetadata(count);
        break;
    case EffectCategory::Hover:
        options = mousefx::HoverMetadata(count);
        break;
    default:
        break;
    }
    if (!options || count == 0) {
        return nullptr;
    }

    const std::string lowered = ToLowerAscii(trimmed);
    for (size_t i = 0; i < count; ++i) {
        const EffectOption& opt = options[i];
        if (trimmed == opt.value) {
            return &opt;
        }
        if (opt.secondType && trimmed == opt.secondType) {
            return &opt;
        }
        if (lowered == ToLowerAscii(opt.value)) {
            return &opt;
        }
        if (opt.secondType && lowered == ToLowerAscii(opt.secondType)) {
            return &opt;
        }
        const std::string zh = Utf16ToUtf8(opt.displayZh ? opt.displayZh : L"");
        if (!zh.empty() && trimmed == zh) {
            return &opt;
        }
        const std::string en = Utf16ToUtf8(opt.displayEn ? opt.displayEn : L"");
        if (!en.empty() && trimmed == en) {
            return &opt;
        }
    }
    return nullptr;
}

void ApplyInputIndicatorFields(const json& source, InputIndicatorConfig* dst, bool includeAdvancedFields) {
    if (!dst) {
        return;
    }

    if (source.contains("enabled") && source["enabled"].is_boolean()) dst->enabled = source["enabled"].get<bool>();
    if (source.contains("keyboard_enabled") && source["keyboard_enabled"].is_boolean()) dst->keyboardEnabled = source["keyboard_enabled"].get<bool>();
    if (source.contains("render_mode") && source["render_mode"].is_string()) dst->renderMode = source["render_mode"].get<std::string>();
    if (source.contains("wasm_fallback_to_native") && source["wasm_fallback_to_native"].is_boolean()) {
        dst->wasmFallbackToNative = source["wasm_fallback_to_native"].get<bool>();
    }
    if (source.contains("wasm_manifest_path") && source["wasm_manifest_path"].is_string()) {
        dst->wasmManifestPath = source["wasm_manifest_path"].get<std::string>();
    }
    if (source.contains("position_mode") && source["position_mode"].is_string()) dst->positionMode = source["position_mode"].get<std::string>();
    if (source.contains("offset_x") && source["offset_x"].is_number_integer()) dst->offsetX = source["offset_x"].get<int>();
    if (source.contains("offset_y") && source["offset_y"].is_number_integer()) dst->offsetY = source["offset_y"].get<int>();
    if (source.contains("absolute_x") && source["absolute_x"].is_number_integer()) dst->absoluteX = source["absolute_x"].get<int>();
    if (source.contains("absolute_y") && source["absolute_y"].is_number_integer()) dst->absoluteY = source["absolute_y"].get<int>();
    if (source.contains("size_px") && source["size_px"].is_number_integer()) dst->sizePx = source["size_px"].get<int>();
    if (source.contains("duration_ms") && source["duration_ms"].is_number_integer()) dst->durationMs = source["duration_ms"].get<int>();
    if (source.contains("cursor_decoration") && source["cursor_decoration"].is_object()) {
        const json& cursorDecoration = source["cursor_decoration"];
        if (cursorDecoration.contains("enabled") && cursorDecoration["enabled"].is_boolean()) {
            dst->cursorDecoration.enabled = cursorDecoration["enabled"].get<bool>();
        }
        if (cursorDecoration.contains("plugin_id") && cursorDecoration["plugin_id"].is_string()) {
            dst->cursorDecoration.pluginId = cursorDecoration["plugin_id"].get<std::string>();
        }
        if (cursorDecoration.contains("color_hex") && cursorDecoration["color_hex"].is_string()) {
            dst->cursorDecoration.colorHex = cursorDecoration["color_hex"].get<std::string>();
        }
        if (cursorDecoration.contains("size_px") && cursorDecoration["size_px"].is_number_integer()) {
            dst->cursorDecoration.sizePx = cursorDecoration["size_px"].get<int>();
        }
        if (cursorDecoration.contains("alpha_percent") && cursorDecoration["alpha_percent"].is_number_integer()) {
            dst->cursorDecoration.alphaPercent = cursorDecoration["alpha_percent"].get<int>();
        }
    }

    if (!includeAdvancedFields) {
        return;
    }

    if (source.contains("target_monitor") && source["target_monitor"].is_string()) dst->targetMonitor = source["target_monitor"].get<std::string>();
    if (source.contains("key_display_mode") && source["key_display_mode"].is_string()) dst->keyDisplayMode = source["key_display_mode"].get<std::string>();
    if (source.contains("key_label_layout_mode") && source["key_label_layout_mode"].is_string()) dst->keyLabelLayoutMode = source["key_label_layout_mode"].get<std::string>();
    if (source.contains("per_monitor_overrides") && source["per_monitor_overrides"].is_object()) {
        dst->perMonitorOverrides.clear();
        for (auto& [key, value] : source["per_monitor_overrides"].items()) {
            if (!value.is_object()) {
                continue;
            }
            PerMonitorPosOverride ov;
            if (value.contains("absolute_x") && value["absolute_x"].is_number_integer()) ov.absoluteX = value["absolute_x"].get<int>();
            if (value.contains("absolute_y") && value["absolute_y"].is_number_integer()) ov.absoluteY = value["absolute_y"].get<int>();
            if (value.contains("enabled") && value["enabled"].is_boolean()) ov.enabled = value["enabled"].get<bool>();
            dst->perMonitorOverrides[key] = ov;
        }
    }
}

} // namespace

bool TryParsePayloadObject(const std::string& jsonCmd, json* outPayload) {
    if (!outPayload) {
        return false;
    }

    json root;
    try {
        root = json::parse(jsonCmd);
    } catch (...) {
        return false;
    }
    if (!root.contains("payload") || !root["payload"].is_object()) {
        return false;
    }

    *outPayload = root["payload"];
    return true;
}

void ApplyActiveSettings(const json& payload, AppController* controller) {
    if (!controller) {
        return;
    }
    if (!payload.contains("active") || !payload["active"].is_object()) {
        return;
    }

    struct ActiveSettingRoute {
        EffectCategory category;
        const char* key;
    };
    static constexpr std::array<ActiveSettingRoute, 5> kActiveSettingRoutes{{
        {EffectCategory::Click, "click"},
        {EffectCategory::Trail, "trail"},
        {EffectCategory::Scroll, "scroll"},
        {EffectCategory::Hold, "hold"},
        {EffectCategory::Hover, "hover"},
    }};

    bool activeChanged = false;
    const json& active = payload["active"];
    for (const auto& route : kActiveSettingRoutes) {
        if (!active.contains(route.key) || !active[route.key].is_string()) {
            continue;
        }

        std::string type = TrimAscii(active[route.key].get<std::string>());
        if (type.empty()) {
            continue;
        }
        if (const EffectOption* resolved = ResolveEffectOptionByInput(route.category, type)) {
            if (resolved->value && resolved->value[0] != '\0') {
                type = resolved->value;
            }
        }

        std::string reason;
        const std::string effectiveType = controller->ResolveRuntimeEffectType(route.category, type, &reason);
        if (effectiveType == "none") {
            controller->ClearEffect(route.category);
        } else {
            controller->SetEffect(route.category, type);
        }
        controller->SetActiveEffectType(route.category, effectiveType);
        activeChanged = true;
    }

    if (activeChanged) {
        controller->PersistConfig();
    }
}

void ApplyTextSettings(const json& payload, AppController* controller) {
    if (!controller) {
        return;
    }

    if (payload.contains("text_content") && payload["text_content"].is_string()) {
        const std::string raw = payload["text_content"].get<std::string>();
        controller->SetTextEffectContent(ParseCsvUtf8TextList(raw));
    }

    if (payload.contains("text_font_size") && payload["text_font_size"].is_number()) {
        controller->SetTextEffectFontSize(payload["text_font_size"].get<float>());
    }
}

void ApplyMouseCompanionSettings(const json& payload, AppController* controller) {
    if (!controller) {
        return;
    }
    if (!payload.contains("mouse_companion") || !payload["mouse_companion"].is_object()) {
        return;
    }

    MouseCompanionConfig companion = controller->Config().mouseCompanion;
    const json& source = payload["mouse_companion"];
    auto applyInt = [&](const char* key, int* dst) {
        if (!key || !dst || !source.contains(key)) {
            return;
        }
        if (source[key].is_number_integer()) {
            *dst = source[key].get<int>();
            return;
        }
        if (source[key].is_number()) {
            *dst = static_cast<int>(source[key].get<double>());
        }
    };
    auto applyDouble = [&](const char* key, double* dst) {
        if (!key || !dst || !source.contains(key) || !source[key].is_number()) {
            return;
        }
        *dst = source[key].get<double>();
    };

    if (source.contains("enabled") && source["enabled"].is_boolean()) {
        companion.enabled = source["enabled"].get<bool>();
    }
    if (source.contains("model_path") && source["model_path"].is_string()) {
        companion.modelPath = source["model_path"].get<std::string>();
    }
    if (source.contains("action_library_path") && source["action_library_path"].is_string()) {
        companion.actionLibraryPath = source["action_library_path"].get<std::string>();
    }
    if (source.contains("appearance_profile_path") && source["appearance_profile_path"].is_string()) {
        companion.appearanceProfilePath = source["appearance_profile_path"].get<std::string>();
    }
    if (source.contains("position_mode") && source["position_mode"].is_string()) {
        companion.positionMode = source["position_mode"].get<std::string>();
    }
    if (source.contains("edge_clamp_mode") && source["edge_clamp_mode"].is_string()) {
        companion.edgeClampMode = source["edge_clamp_mode"].get<std::string>();
    }
    applyInt("size_px", &companion.sizePx);
    applyInt("offset_x", &companion.offsetX);
    applyInt("offset_y", &companion.offsetY);
    applyInt("absolute_x", &companion.absoluteX);
    applyInt("absolute_y", &companion.absoluteY);
    if (source.contains("target_monitor") && source["target_monitor"].is_string()) {
        companion.targetMonitor = source["target_monitor"].get<std::string>();
    }
    applyInt("press_lift_px", &companion.pressLiftPx);
    applyInt("smoothing_percent", &companion.smoothingPercent);
    applyInt("follow_threshold_px", &companion.followThresholdPx);
    applyInt("release_hold_ms", &companion.releaseHoldMs);
    applyInt("click_streak_break_ms", &companion.clickStreakBreakMs);
    applyDouble("head_tint_per_click", &companion.headTintPerClick);
    applyDouble("head_tint_max", &companion.headTintMax);
    applyDouble("head_tint_decay_per_second", &companion.headTintDecayPerSecond);
    if (source.contains("renderer_backend_preference_source") && source["renderer_backend_preference_source"].is_string()) {
        companion.rendererBackendPreferenceSource = source["renderer_backend_preference_source"].get<std::string>();
    }
    if (source.contains("renderer_backend_preference_name") && source["renderer_backend_preference_name"].is_string()) {
        companion.rendererBackendPreferenceName = source["renderer_backend_preference_name"].get<std::string>();
    }
    if (source.contains("use_test_profile") && source["use_test_profile"].is_boolean()) {
        companion.useTestProfile = source["use_test_profile"].get<bool>();
    }
    applyInt("test_press_lift_px", &companion.testPressLiftPx);
    applyInt("test_smoothing_percent", &companion.testSmoothingPercent);
    applyInt("test_click_streak_break_ms", &companion.testClickStreakBreakMs);
    applyDouble("test_head_tint_per_click", &companion.testHeadTintPerClick);
    applyDouble("test_head_tint_max", &companion.testHeadTintMax);
    applyDouble("test_head_tint_decay_per_second", &companion.testHeadTintDecayPerSecond);

    controller->SetMouseCompanionConfig(config_internal::SanitizeMouseCompanionConfig(companion));
}

void ApplyInputIndicatorSettings(const json& payload, AppController* controller) {
    if (!controller) {
        return;
    }

    const EffectConfig& config = controller->Config();
    if (payload.contains("input_indicator") && payload["input_indicator"].is_object()) {
        InputIndicatorConfig indicator = config.inputIndicator;
        ApplyInputIndicatorFields(payload["input_indicator"], &indicator, true);
        controller->SetInputIndicatorConfig(indicator);
        return;
    }

    if (payload.contains("mouse_indicator") && payload["mouse_indicator"].is_object()) {
        // Legacy fallback.
        InputIndicatorConfig indicator = config.inputIndicator;
        ApplyInputIndicatorFields(payload["mouse_indicator"], &indicator, false);
        controller->SetInputIndicatorConfig(indicator);
    }
}

void ApplyTrailTuningSettings(const json& payload, AppController* controller) {
    if (!controller) {
        return;
    }

    const EffectConfig& cfg = controller->Config();
    bool trailTouched = false;
    std::string style = cfg.trailStyle.empty() ? "default" : cfg.trailStyle;
    TrailProfilesConfig profiles = cfg.trailProfiles;
    TrailRendererParamsConfig params = cfg.trailParams;
    float lineWidth = cfg.trail.lineWidth;

    if (payload.contains("trail_style") && payload["trail_style"].is_string()) {
        style = payload["trail_style"].get<std::string>();
        trailTouched = true;
    }
    if (payload.contains("trail_line_width") && payload["trail_line_width"].is_number()) {
        lineWidth = ClampFloat(payload["trail_line_width"].get<float>(), 1.0f, 18.0f);
    }

    auto applyProfile = [&](const char* key, TrailHistoryProfile* dst) {
        if (!dst) {
            return;
        }
        if (!payload.contains("trail_profiles") || !payload["trail_profiles"].is_object()) {
            return;
        }

        const json& profileSet = payload["trail_profiles"];
        if (!profileSet.contains(key) || !profileSet[key].is_object()) {
            return;
        }
        const json& obj = profileSet[key];
        if (obj.contains("duration_ms") && obj["duration_ms"].is_number_integer()) {
            dst->durationMs = ClampInt(obj["duration_ms"].get<int>(), 80, 2000);
            trailTouched = true;
        }
        if (obj.contains("max_points") && obj["max_points"].is_number_integer()) {
            dst->maxPoints = ClampInt(obj["max_points"].get<int>(), 2, 240);
            trailTouched = true;
        }
    };

    struct TrailProfileRoute {
        const char* key;
        TrailHistoryProfile* profile;
    };
    const std::array<TrailProfileRoute, 5> profileRoutes{{
        {"line", &profiles.line},
        {"streamer", &profiles.streamer},
        {"electric", &profiles.electric},
        {"meteor", &profiles.meteor},
        {"tubes", &profiles.tubes},
    }};
    for (const auto& route : profileRoutes) {
        applyProfile(route.key, route.profile);
    }

    if (payload.contains("trail_params") && payload["trail_params"].is_object()) {
        const json& trailParams = payload["trail_params"];
        if (trailParams.contains("streamer") && trailParams["streamer"].is_object()) {
            const json& s = trailParams["streamer"];
            if (s.contains("glow_width_scale") && s["glow_width_scale"].is_number()) {
                params.streamer.glowWidthScale = ClampFloat(s["glow_width_scale"].get<float>(), 0.5f, 4.0f);
                trailTouched = true;
            }
            if (s.contains("core_width_scale") && s["core_width_scale"].is_number()) {
                params.streamer.coreWidthScale = ClampFloat(s["core_width_scale"].get<float>(), 0.2f, 2.0f);
                trailTouched = true;
            }
            if (s.contains("head_power") && s["head_power"].is_number()) {
                params.streamer.headPower = ClampFloat(s["head_power"].get<float>(), 0.8f, 3.0f);
                trailTouched = true;
            }
        }
        if (trailParams.contains("electric") && trailParams["electric"].is_object()) {
            const json& e = trailParams["electric"];
            if (e.contains("amplitude_scale") && e["amplitude_scale"].is_number()) {
                params.electric.amplitudeScale = ClampFloat(e["amplitude_scale"].get<float>(), 0.2f, 3.0f);
                trailTouched = true;
            }
            if (e.contains("fork_chance") && e["fork_chance"].is_number()) {
                params.electric.forkChance = ClampFloat(e["fork_chance"].get<float>(), 0.0f, 0.5f);
                trailTouched = true;
            }
        }
        if (trailParams.contains("meteor") && trailParams["meteor"].is_object()) {
            const json& m = trailParams["meteor"];
            if (m.contains("spark_rate_scale") && m["spark_rate_scale"].is_number()) {
                params.meteor.sparkRateScale = ClampFloat(m["spark_rate_scale"].get<float>(), 0.2f, 4.0f);
                trailTouched = true;
            }
            if (m.contains("spark_speed_scale") && m["spark_speed_scale"].is_number()) {
                params.meteor.sparkSpeedScale = ClampFloat(m["spark_speed_scale"].get<float>(), 0.2f, 4.0f);
                trailTouched = true;
            }
        }
        if (trailParams.contains("idle_fade_start_ms") && trailParams["idle_fade_start_ms"].is_number_integer()) {
            params.idleFade.startMs = ClampInt(trailParams["idle_fade_start_ms"].get<int>(), 0, 3000);
            trailTouched = true;
        }
        if (trailParams.contains("idle_fade_end_ms") && trailParams["idle_fade_end_ms"].is_number_integer()) {
            params.idleFade.endMs = ClampInt(trailParams["idle_fade_end_ms"].get<int>(), 0, 6000);
            trailTouched = true;
        }
    }

    if (trailTouched) {
        controller->SetTrailTuning(style, profiles, params);
    }
    if (std::fabs(cfg.trail.lineWidth - lineWidth) > 0.01f) {
        controller->SetTrailLineWidth(lineWidth);
    }
}

void ApplyEffectSizeScaleSettings(const json& payload, AppController* controller) {
    if (!controller) {
        return;
    }
    if (!payload.contains("effect_size_scales") || !payload["effect_size_scales"].is_object()) {
        return;
    }

    EffectSizeScaleConfig scales = controller->Config().effectSizeScales;
    const json& source = payload["effect_size_scales"];
    auto applyOne = [&](const char* key, int* dst) {
        if (!dst || !source.contains(key)) {
            return;
        }
        if (source[key].is_number_integer()) {
            *dst = source[key].get<int>();
            return;
        }
        if (source[key].is_number()) {
            *dst = static_cast<int>(source[key].get<double>());
        }
    };

    applyOne("click", &scales.click);
    applyOne("trail", &scales.trail);
    applyOne("scroll", &scales.scroll);
    applyOne("hold", &scales.hold);
    applyOne("hover", &scales.hover);
    controller->SetEffectSizeScales(config_internal::SanitizeEffectSizeScaleConfig(scales));
}

void ApplyEffectConflictPolicySettings(const json& payload, AppController* controller) {
    if (!controller) {
        return;
    }
    if (!payload.contains("effect_conflict_policy") || !payload["effect_conflict_policy"].is_object()) {
        return;
    }

    EffectConflictPolicyConfig policy = controller->Config().effectConflictPolicy;
    const json& source = payload["effect_conflict_policy"];
    auto applyString = [&](const char* key, std::string* dst) {
        if (!key || !dst || !source.contains(key) || !source[key].is_string()) {
            return;
        }
        *dst = source[key].get<std::string>();
    };

    applyString("hold_move_policy", &policy.holdMovePolicy);
    applyString("hold_move", &policy.holdMovePolicy);
    controller->SetEffectConflictPolicy(config_internal::SanitizeEffectConflictPolicyConfig(policy));
}

void ApplyEffectsBlacklistSettings(const json& payload, AppController* controller) {
    if (!controller) {
        return;
    }
    if (!payload.contains("effects_blacklist_apps") || !payload["effects_blacklist_apps"].is_array()) {
        return;
    }

    std::vector<std::string> apps;
    for (const auto& item : payload["effects_blacklist_apps"]) {
        if (item.is_string()) {
            apps.push_back(item.get<std::string>());
        }
    }
    controller->SetEffectsBlacklistApps(apps);
}

} // namespace mousefx::command_handler_apply_settings

// CommandHandler.cpp — JSON command routing extracted from AppController

#include "pch.h"
#include "CommandHandler.h"
#include "AppController.h"
#include "JsonLite.h"
#include "MouseFx/ThirdParty/json.hpp"
#include "MouseFx/Utils/MathUtils.h"
#include "MouseFx/Utils/StringUtils.h"

namespace mousefx {

using json = nlohmann::json;

CommandHandler::CommandHandler(AppController* controller)
    : controller_(controller) {}

void CommandHandler::Handle(const std::string& jsonCmd) {
    std::string cmd = ExtractJsonStringValue(jsonCmd, "cmd");

    if (cmd == "set_effect") {
        std::string category = ExtractJsonStringValue(jsonCmd, "category");
        std::string type = ExtractJsonStringValue(jsonCmd, "type");

        if (category.empty()) {
            // Legacy format: {"cmd": "set_effect", "type": "ripple"}
            std::string reason;
            const std::string effectiveType = controller_->ResolveRuntimeEffectType(EffectCategory::Click, type, &reason);
            controller_->SetEffect(EffectCategory::Click, type);
            controller_->SetActiveEffectType(EffectCategory::Click, effectiveType);
        } else {
            const auto cat = CategoryFromString(category);
            std::string reason;
            const std::string effectiveType = controller_->ResolveRuntimeEffectType(cat, type, &reason);
            controller_->SetEffect(cat, type);
            controller_->SetActiveEffectType(cat, effectiveType);
        }
        controller_->PersistConfig();
    } else if (cmd == "clear_effect") {
        std::string category = ExtractJsonStringValue(jsonCmd, "category");
        const auto cat = CategoryFromString(category);
        controller_->ClearEffect(cat);
        controller_->SetActiveEffectType(cat, "none");
        controller_->PersistConfig();
    } else if (cmd == "set_theme") {
        std::string theme = ExtractJsonStringValue(jsonCmd, "theme");
        controller_->SetTheme(theme);
    } else if (cmd == "set_ui_language") {
        std::string lang = ExtractJsonStringValue(jsonCmd, "lang");
        controller_->SetUiLanguage(lang);
    } else if (cmd == "effect_cmd") {
        std::string category = ExtractJsonStringValue(jsonCmd, "category");
        std::string command = ExtractJsonStringValue(jsonCmd, "command");
        std::string args = ExtractJsonStringValue(jsonCmd, "args");

        if (!category.empty()) {
            const auto cat = CategoryFromString(category);
            if (auto* effect = controller_->GetEffect(cat)) {
                effect->OnCommand(command, args);
            }
        }
    } else if (cmd == "reload_config") {
        controller_->ReloadConfigFromDisk();
    } else if (cmd == "reset_config") {
        controller_->ResetConfig();
    } else if (cmd == "apply_settings") {
        HandleApplySettings(jsonCmd);
    }
}

void CommandHandler::HandleApplySettings(const std::string& jsonCmd) {
    json root;
    try {
        root = json::parse(jsonCmd);
    } catch (...) {
        return;
    }
    if (!root.contains("payload") || !root["payload"].is_object()) return;
    const json& p = root["payload"];

    // Active effects first (theme application recreates themed effects).
    bool activeChanged = false;
    if (p.contains("active") && p["active"].is_object()) {
        auto applyActive = [&](EffectCategory category, const char* key) {
            const json& a = p["active"];
            if (!a.contains(key) || !a[key].is_string()) return;
            std::string type = a[key].get<std::string>();
            if (type.empty()) return;
            std::string reason;
            const std::string effectiveType = controller_->ResolveRuntimeEffectType(category, type, &reason);

            if (effectiveType == "none") {
                controller_->ClearEffect(category);
            } else {
                controller_->SetEffect(category, type);
            }
            controller_->SetActiveEffectType(category, effectiveType);
            activeChanged = true;
        };

        applyActive(EffectCategory::Click, "click");
        applyActive(EffectCategory::Trail, "trail");
        applyActive(EffectCategory::Scroll, "scroll");
        applyActive(EffectCategory::Hold, "hold");
        applyActive(EffectCategory::Hover, "hover");
    }
    if (activeChanged) {
        controller_->PersistConfig();
    }

    if (p.contains("ui_language") && p["ui_language"].is_string()) {
        controller_->SetUiLanguage(p["ui_language"].get<std::string>());
    }

    if (p.contains("text_content") && p["text_content"].is_string()) {
        std::vector<std::wstring> texts;
        std::string raw = p["text_content"].get<std::string>();
        size_t start = 0;
        size_t end = raw.find(',');
        while (end != std::string::npos) {
            std::string token = TrimAscii(raw.substr(start, end - start));
            if (!token.empty()) texts.push_back(Utf8ToWString(token));
            start = end + 1;
            end = raw.find(',', start);
        }
        std::string last = TrimAscii(raw.substr(start));
        if (!last.empty()) texts.push_back(Utf8ToWString(last));
        controller_->SetTextEffectContent(texts);
    }

    if (p.contains("text_font_size") && p["text_font_size"].is_number()) {
        controller_->SetTextEffectFontSize(p["text_font_size"].get<float>());
    }

    if (p.contains("input_indicator") && p["input_indicator"].is_object()) {
        InputIndicatorConfig mi = controller_->Config().inputIndicator;
        const json& o = p["input_indicator"];
        if (o.contains("enabled") && o["enabled"].is_boolean()) mi.enabled = o["enabled"].get<bool>();
        if (o.contains("keyboard_enabled") && o["keyboard_enabled"].is_boolean()) mi.keyboardEnabled = o["keyboard_enabled"].get<bool>();
        if (o.contains("position_mode") && o["position_mode"].is_string()) mi.positionMode = o["position_mode"].get<std::string>();
        if (o.contains("offset_x") && o["offset_x"].is_number_integer()) mi.offsetX = o["offset_x"].get<int>();
        if (o.contains("offset_y") && o["offset_y"].is_number_integer()) mi.offsetY = o["offset_y"].get<int>();
        if (o.contains("absolute_x") && o["absolute_x"].is_number_integer()) mi.absoluteX = o["absolute_x"].get<int>();
        if (o.contains("absolute_y") && o["absolute_y"].is_number_integer()) mi.absoluteY = o["absolute_y"].get<int>();
        if (o.contains("target_monitor") && o["target_monitor"].is_string()) mi.targetMonitor = o["target_monitor"].get<std::string>();
        if (o.contains("key_display_mode") && o["key_display_mode"].is_string()) mi.keyDisplayMode = o["key_display_mode"].get<std::string>();

        if (o.contains("per_monitor_overrides") && o["per_monitor_overrides"].is_object()) {
            mi.perMonitorOverrides.clear();
            for (auto& [key, val] : o["per_monitor_overrides"].items()) {
                if (val.is_object()) {
                    mousefx::PerMonitorPosOverride ov;
                    if (val.contains("absolute_x") && val["absolute_x"].is_number_integer()) ov.absoluteX = val["absolute_x"].get<int>();
                    if (val.contains("absolute_y") && val["absolute_y"].is_number_integer()) ov.absoluteY = val["absolute_y"].get<int>();
                    if (val.contains("enabled") && val["enabled"].is_boolean()) ov.enabled = val["enabled"].get<bool>();
                    mi.perMonitorOverrides[key] = ov;
                }
            }
        }

        if (o.contains("size_px") && o["size_px"].is_number_integer()) mi.sizePx = o["size_px"].get<int>();
        if (o.contains("duration_ms") && o["duration_ms"].is_number_integer()) mi.durationMs = o["duration_ms"].get<int>();
        controller_->SetInputIndicatorConfig(mi);
    } else if (p.contains("mouse_indicator") && p["mouse_indicator"].is_object()) {
        // Legacy fallback
        InputIndicatorConfig mi = controller_->Config().inputIndicator;
        const json& o = p["mouse_indicator"];
        if (o.contains("enabled") && o["enabled"].is_boolean()) mi.enabled = o["enabled"].get<bool>();
        if (o.contains("keyboard_enabled") && o["keyboard_enabled"].is_boolean()) mi.keyboardEnabled = o["keyboard_enabled"].get<bool>();
        if (o.contains("position_mode") && o["position_mode"].is_string()) mi.positionMode = o["position_mode"].get<std::string>();
        if (o.contains("offset_x") && o["offset_x"].is_number_integer()) mi.offsetX = o["offset_x"].get<int>();
        if (o.contains("offset_y") && o["offset_y"].is_number_integer()) mi.offsetY = o["offset_y"].get<int>();
        if (o.contains("absolute_x") && o["absolute_x"].is_number_integer()) mi.absoluteX = o["absolute_x"].get<int>();
        if (o.contains("absolute_y") && o["absolute_y"].is_number_integer()) mi.absoluteY = o["absolute_y"].get<int>();
        if (o.contains("size_px") && o["size_px"].is_number_integer()) mi.sizePx = o["size_px"].get<int>();
        if (o.contains("duration_ms") && o["duration_ms"].is_number_integer()) mi.durationMs = o["duration_ms"].get<int>();
        controller_->SetInputIndicatorConfig(mi);
    }

    if (p.contains("hold_follow_mode") && p["hold_follow_mode"].is_string()) {
        controller_->SetHoldFollowMode(p["hold_follow_mode"].get<std::string>());
    }

    // Trail tuning (optional fields).
    bool trailTouched = false;
    std::string style = controller_->Config().trailStyle.empty() ? "default" : controller_->Config().trailStyle;
    TrailProfilesConfig profiles = controller_->Config().trailProfiles;
    TrailRendererParamsConfig params = controller_->Config().trailParams;

    if (p.contains("trail_style") && p["trail_style"].is_string()) {
        style = p["trail_style"].get<std::string>();
        trailTouched = true;
    }

    auto applyProfile = [&](const char* key, TrailHistoryProfile& dst) {
        if (!p.contains("trail_profiles") || !p["trail_profiles"].is_object()) return;
        const json& tp = p["trail_profiles"];
        if (!tp.contains(key) || !tp[key].is_object()) return;
        const json& o = tp[key];
        if (o.contains("duration_ms") && o["duration_ms"].is_number_integer()) {
            dst.durationMs = ClampInt(o["duration_ms"].get<int>(), 80, 2000);
            trailTouched = true;
        }
        if (o.contains("max_points") && o["max_points"].is_number_integer()) {
            dst.maxPoints = ClampInt(o["max_points"].get<int>(), 2, 240);
            trailTouched = true;
        }
    };

    applyProfile("line", profiles.line);
    applyProfile("streamer", profiles.streamer);
    applyProfile("electric", profiles.electric);
    applyProfile("meteor", profiles.meteor);
    applyProfile("tubes", profiles.tubes);

    if (p.contains("trail_params") && p["trail_params"].is_object()) {
        const json& k = p["trail_params"];
        if (k.contains("streamer") && k["streamer"].is_object()) {
            const json& s = k["streamer"];
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
        if (k.contains("electric") && k["electric"].is_object()) {
            const json& e = k["electric"];
            if (e.contains("amplitude_scale") && e["amplitude_scale"].is_number()) {
                params.electric.amplitudeScale = ClampFloat(e["amplitude_scale"].get<float>(), 0.2f, 3.0f);
                trailTouched = true;
            }
            if (e.contains("fork_chance") && e["fork_chance"].is_number()) {
                params.electric.forkChance = ClampFloat(e["fork_chance"].get<float>(), 0.0f, 0.5f);
                trailTouched = true;
            }
        }
        if (k.contains("meteor") && k["meteor"].is_object()) {
            const json& m = k["meteor"];
            if (m.contains("spark_rate_scale") && m["spark_rate_scale"].is_number()) {
                params.meteor.sparkRateScale = ClampFloat(m["spark_rate_scale"].get<float>(), 0.2f, 4.0f);
                trailTouched = true;
            }
            if (m.contains("spark_speed_scale") && m["spark_speed_scale"].is_number()) {
                params.meteor.sparkSpeedScale = ClampFloat(m["spark_speed_scale"].get<float>(), 0.2f, 4.0f);
                trailTouched = true;
            }
        }
        if (k.contains("idle_fade_start_ms") && k["idle_fade_start_ms"].is_number_integer()) {
            params.idleFade.startMs = ClampInt(k["idle_fade_start_ms"].get<int>(), 0, 3000);
            trailTouched = true;
        }
        if (k.contains("idle_fade_end_ms") && k["idle_fade_end_ms"].is_number_integer()) {
            params.idleFade.endMs = ClampInt(k["idle_fade_end_ms"].get<int>(), 0, 6000);
            trailTouched = true;
        }
    }

    if (trailTouched) {
        controller_->SetTrailTuning(style, profiles, params);
    }

    // Theme last (recreates themed effects).
    if (p.contains("theme") && p["theme"].is_string()) {
        controller_->SetTheme(p["theme"].get<std::string>());
    }
}

} // namespace mousefx

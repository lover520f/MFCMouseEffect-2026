#include "pch.h"
#include "EffectConfig.h"
#include "json.hpp"

#include <fstream>
#include <sstream>

using json = nlohmann::json;

namespace mousefx {

// ============================================================================
// Argb Parsing
// ============================================================================

Argb ArgbFromHex(const std::string& hex) {
    if (hex.empty() || hex[0] != '#') {
        return Argb{0};
    }
    
    std::string h = hex.substr(1);
    uint32_t val = 0;
    
    try {
        if (h.length() == 6) {
            // #RRGGBB -> assume FF alpha
            val = 0xFF000000 | static_cast<uint32_t>(std::stoul(h, nullptr, 16));
        } else if (h.length() == 8) {
            // #AARRGGBB
            val = static_cast<uint32_t>(std::stoul(h, nullptr, 16));
        }
    } catch (...) {
        val = 0;
    }
    
    return Argb{val};
}

// ============================================================================
// Helper: Safe JSON value extraction
// ============================================================================

template<typename T>
static T GetOr(const json& j, const std::string& key, T defaultVal) {
    if (j.contains(key) && !j[key].is_null()) {
        try {
            return j[key].get<T>();
        } catch (...) {}
    }
    return defaultVal;
}

static Argb GetColorOr(const json& j, const std::string& key, Argb defaultVal) {
    if (j.contains(key) && j[key].is_string()) {
        return ArgbFromHex(j[key].get<std::string>());
    }
    return defaultVal;
}

// ============================================================================
// EffectConfig Implementation
// ============================================================================

EffectConfig EffectConfig::GetDefault() {
    return EffectConfig{};
}

EffectConfig EffectConfig::Load(const std::wstring& exeDir) {
    EffectConfig cfg = GetDefault();
    
    std::wstring configPath = exeDir + L"\\config.json";
    
    std::ifstream file(configPath);
    if (!file.is_open()) {
        // No config file, use defaults
#ifdef _DEBUG
        OutputDebugStringW(L"MouseFx: config.json not found, using defaults.\n");
#endif
        return cfg;
    }
    
    json root;
    try {
        file >> root;
    } catch (const json::exception& e) {
#ifdef _DEBUG
        std::wstringstream ss;
        ss << L"MouseFx: JSON parse error: " << e.what() << L"\n";
        OutputDebugStringW(ss.str().c_str());
#endif
        return cfg;
    }
    
    // Parse root level
    cfg.defaultEffect = GetOr<std::string>(root, "default_effect", cfg.defaultEffect);
    
    // Parse effects
    if (root.contains("effects") && root["effects"].is_object()) {
        const auto& effects = root["effects"];
        
        // --- Ripple ---
        if (effects.contains("ripple") && effects["ripple"].is_object()) {
            const auto& r = effects["ripple"];
            cfg.ripple.durationMs = GetOr<int>(r, "duration_ms", cfg.ripple.durationMs);
            cfg.ripple.startRadius = GetOr<float>(r, "start_radius", cfg.ripple.startRadius);
            cfg.ripple.endRadius = GetOr<float>(r, "end_radius", cfg.ripple.endRadius);
            cfg.ripple.strokeWidth = GetOr<float>(r, "stroke_width", cfg.ripple.strokeWidth);
            cfg.ripple.windowSize = GetOr<int>(r, "window_size", cfg.ripple.windowSize);
            
            if (r.contains("left_click") && r["left_click"].is_object()) {
                const auto& lc = r["left_click"];
                cfg.ripple.leftClick.fill = GetColorOr(lc, "fill", cfg.ripple.leftClick.fill);
                cfg.ripple.leftClick.stroke = GetColorOr(lc, "stroke", cfg.ripple.leftClick.stroke);
                cfg.ripple.leftClick.glow = GetColorOr(lc, "glow", cfg.ripple.leftClick.glow);
            }
            if (r.contains("right_click") && r["right_click"].is_object()) {
                const auto& rc = r["right_click"];
                cfg.ripple.rightClick.fill = GetColorOr(rc, "fill", cfg.ripple.rightClick.fill);
                cfg.ripple.rightClick.stroke = GetColorOr(rc, "stroke", cfg.ripple.rightClick.stroke);
                cfg.ripple.rightClick.glow = GetColorOr(rc, "glow", cfg.ripple.rightClick.glow);
            }
            if (r.contains("middle_click") && r["middle_click"].is_object()) {
                const auto& mc = r["middle_click"];
                cfg.ripple.middleClick.fill = GetColorOr(mc, "fill", cfg.ripple.middleClick.fill);
                cfg.ripple.middleClick.stroke = GetColorOr(mc, "stroke", cfg.ripple.middleClick.stroke);
                cfg.ripple.middleClick.glow = GetColorOr(mc, "glow", cfg.ripple.middleClick.glow);
            }
        }
        
        // --- Trail ---
        if (effects.contains("trail") && effects["trail"].is_object()) {
            const auto& t = effects["trail"];
            cfg.trail.durationMs = GetOr<int>(t, "duration_ms", cfg.trail.durationMs);
            cfg.trail.maxPoints = GetOr<int>(t, "max_points", cfg.trail.maxPoints);
            cfg.trail.lineWidth = GetOr<float>(t, "line_width", cfg.trail.lineWidth);
            cfg.trail.color = GetColorOr(t, "color", cfg.trail.color);
        }
        
        // --- Icon/Star ---
        if (effects.contains("icon_star") && effects["icon_star"].is_object()) {
            const auto& i = effects["icon_star"];
            cfg.icon.durationMs = GetOr<int>(i, "duration_ms", cfg.icon.durationMs);
            cfg.icon.startRadius = GetOr<float>(i, "start_radius", cfg.icon.startRadius);
            cfg.icon.endRadius = GetOr<float>(i, "end_radius", cfg.icon.endRadius);
            cfg.icon.strokeWidth = GetOr<float>(i, "stroke_width", cfg.icon.strokeWidth);
            cfg.icon.fillColor = GetColorOr(i, "fill", cfg.icon.fillColor);
            cfg.icon.strokeColor = GetColorOr(i, "stroke", cfg.icon.strokeColor);
        }
    }
    
#ifdef _DEBUG
    OutputDebugStringW(L"MouseFx: config.json loaded successfully.\n");
#endif
    return cfg;
}

} // namespace mousefx

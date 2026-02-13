#include "pch.h"
#include "EffectConfig.h"
#include "MouseFx/ThirdParty/json.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>

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

static std::string ArgbToHex(Argb c) {
    std::ostringstream ss;
    ss << '#' << std::uppercase << std::setfill('0') << std::hex << std::setw(8) << c.value;
    return ss.str();
}

static std::string WStringToUtf8(const std::wstring& ws) {
    if (ws.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string out(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, out.empty() ? nullptr : &out[0], len, nullptr, nullptr);
    return out;
}

static std::string ToLowerAsciiLocal(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return s;
}

static std::string NormalizeHoldFollowMode(std::string s) {
    s = ToLowerAsciiLocal(s);
    if (s == "precise") return "precise";
    if (s == "efficient") return "efficient";
    return "smooth";
}

static TrailHistoryProfile SanitizeTrailHistoryProfile(TrailHistoryProfile p) {
    if (p.durationMs < 80) p.durationMs = 80;
    if (p.durationMs > 2000) p.durationMs = 2000;
    if (p.maxPoints < 2) p.maxPoints = 2;
    if (p.maxPoints > 240) p.maxPoints = 240;
    return p;
}

static float ClampFloat(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static TrailRendererParamsConfig SanitizeTrailParams(TrailRendererParamsConfig p) {
    p.streamer.glowWidthScale = ClampFloat(p.streamer.glowWidthScale, 0.5f, 4.0f);
    p.streamer.coreWidthScale = ClampFloat(p.streamer.coreWidthScale, 0.2f, 2.0f);
    p.streamer.headPower = ClampFloat(p.streamer.headPower, 0.8f, 3.0f);

    p.electric.amplitudeScale = ClampFloat(p.electric.amplitudeScale, 0.2f, 3.0f);
    p.electric.forkChance = ClampFloat(p.electric.forkChance, 0.0f, 0.5f);

    p.meteor.sparkRateScale = ClampFloat(p.meteor.sparkRateScale, 0.2f, 4.0f);
    p.meteor.sparkSpeedScale = ClampFloat(p.meteor.sparkSpeedScale, 0.2f, 4.0f);

    if (p.idleFade.startMs < 0) p.idleFade.startMs = 0;
    if (p.idleFade.endMs < 0) p.idleFade.endMs = 0;
    if (p.idleFade.startMs > 3000) p.idleFade.startMs = 3000;
    if (p.idleFade.endMs > 6000) p.idleFade.endMs = 6000;

    return p;
}

// ============================================================================
// EffectConfig Implementation
// ============================================================================

// Helper to ensure UTF-8 content
static std::string ReadFileAsUtf8(const std::wstring& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file.is_open()) return "";

    std::stringstream ss;
    ss << file.rdbuf();
    std::string raw = ss.str();
    
    if (raw.empty()) return "";

    // 1. Remove BOM if present
    if (raw.size() >= 3 && 
        static_cast<unsigned char>(raw[0]) == 0xEF && 
        static_cast<unsigned char>(raw[1]) == 0xBB && 
        static_cast<unsigned char>(raw[2]) == 0xBF) {
        raw = raw.substr(3);
    }

    // 2. Try to validate as UTF-8
    int res = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, raw.c_str(), -1, nullptr, 0);
    bool isUtf8 = (res > 0);

    if (isUtf8) {
        return raw;
    }

    // 3. If failed, assume ACP (ANSI/GBK) and convert
    int wlen = MultiByteToWideChar(CP_ACP, 0, raw.c_str(), -1, nullptr, 0);
    if (wlen <= 0) return raw; // Failed completely, fallback

    std::wstring wstr(wlen, 0);
    MultiByteToWideChar(CP_ACP, 0, raw.c_str(), -1, &wstr[0], wlen);

    int ulen = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (ulen <= 0) return raw;

    std::string utf8(ulen - 1, 0); 
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &utf8[0], ulen, nullptr, nullptr);

    return utf8;
}

EffectConfig EffectConfig::GetDefault() {
    return EffectConfig{};
}

TrailHistoryProfile EffectConfig::GetTrailHistoryProfile(const std::string& type) const {
    std::string t = ToLowerAsciiLocal(type);
    if (t == "scifi" || t == "sci-fi" || t == "sci_fi") t = "tubes";

    if (t == "electric") return SanitizeTrailHistoryProfile(trailProfiles.electric);
    if (t == "streamer") return SanitizeTrailHistoryProfile(trailProfiles.streamer);
    if (t == "meteor") return SanitizeTrailHistoryProfile(trailProfiles.meteor);
    if (t == "tubes") return SanitizeTrailHistoryProfile(trailProfiles.tubes);
    return SanitizeTrailHistoryProfile(trailProfiles.line);
}

EffectConfig EffectConfig::Load(const std::wstring& exeDir) {
    EffectConfig cfg = GetDefault();
    
    std::wstring configPath = exeDir + L"\\config.json";
#ifdef _DEBUG
    OutputDebugStringW((L"MouseFx: Loading config from " + configPath + L"\n").c_str());
#endif
    
    std::string jsonContent = ReadFileAsUtf8(configPath);
    if (jsonContent.empty()) {
#ifdef _DEBUG
        OutputDebugStringW(L"MouseFx: config.json not found or empty, creating default.\n");
#endif
        Save(exeDir, cfg); // Create new default file
        return cfg;
    }
    
    json root;
    try {
        root = json::parse(jsonContent);
    } catch (const json::exception& e) {
#ifdef _DEBUG
        std::wstringstream ss;
        ss << L"MouseFx: JSON parse error: " << e.what() << L". Recreating config.\n";
        OutputDebugStringW(ss.str().c_str());
#endif
        // If parse error, maybe file is corrupted. Recreate it?
        // User asked: "config file really bad -> delete and new"
        Save(exeDir, cfg);
        return cfg;
    }
    
    // Parse root level
    cfg.defaultEffect = GetOr<std::string>(root, "default_effect", cfg.defaultEffect);
    cfg.theme = GetOr<std::string>(root, "theme", cfg.theme);
    cfg.uiLanguage = GetOr<std::string>(root, "ui_language", cfg.uiLanguage);
    cfg.holdFollowMode = NormalizeHoldFollowMode(GetOr<std::string>(root, "hold_follow_mode", cfg.holdFollowMode));
    cfg.fluxGpuV2D2dExperimental = GetOr<bool>(root, "flux_gpu_v2_d2d_experimental", cfg.fluxGpuV2D2dExperimental);
    if (root.contains("active_effects") && root["active_effects"].is_object()) {
        const auto& a = root["active_effects"];
        cfg.active.click = GetOr<std::string>(a, "click", cfg.active.click);
        cfg.active.trail = GetOr<std::string>(a, "trail", cfg.active.trail);
        cfg.active.scroll = GetOr<std::string>(a, "scroll", cfg.active.scroll);
        cfg.active.hover = GetOr<std::string>(a, "hover", cfg.active.hover);
        cfg.active.hold = GetOr<std::string>(a, "hold", cfg.active.hold);
    }

    cfg.trailStyle = GetOr<std::string>(root, "trail_style", cfg.trailStyle);

    // Parse trail renderer params (optional)
    if (root.contains("trail_params") && root["trail_params"].is_object()) {
        const auto& tp = root["trail_params"];

        if (tp.contains("streamer") && tp["streamer"].is_object()) {
            const auto& o = tp["streamer"];
            cfg.trailParams.streamer.glowWidthScale = GetOr<float>(o, "glow_width_scale", cfg.trailParams.streamer.glowWidthScale);
            cfg.trailParams.streamer.coreWidthScale = GetOr<float>(o, "core_width_scale", cfg.trailParams.streamer.coreWidthScale);
            cfg.trailParams.streamer.headPower = GetOr<float>(o, "head_power", cfg.trailParams.streamer.headPower);
        }
        if (tp.contains("electric") && tp["electric"].is_object()) {
            const auto& o = tp["electric"];
            cfg.trailParams.electric.amplitudeScale = GetOr<float>(o, "amplitude_scale", cfg.trailParams.electric.amplitudeScale);
            cfg.trailParams.electric.forkChance = GetOr<float>(o, "fork_chance", cfg.trailParams.electric.forkChance);
        }
        if (tp.contains("meteor") && tp["meteor"].is_object()) {
            const auto& o = tp["meteor"];
            cfg.trailParams.meteor.sparkRateScale = GetOr<float>(o, "spark_rate_scale", cfg.trailParams.meteor.sparkRateScale);
            cfg.trailParams.meteor.sparkSpeedScale = GetOr<float>(o, "spark_speed_scale", cfg.trailParams.meteor.sparkSpeedScale);
        }
        cfg.trailParams.idleFade.startMs = GetOr<int>(tp, "idle_fade_start_ms", cfg.trailParams.idleFade.startMs);
        cfg.trailParams.idleFade.endMs = GetOr<int>(tp, "idle_fade_end_ms", cfg.trailParams.idleFade.endMs);

        cfg.trailParams = SanitizeTrailParams(cfg.trailParams);
    }

    // Parse trail history profiles (optional)
    if (root.contains("trail_profiles") && root["trail_profiles"].is_object()) {
        const auto& tp = root["trail_profiles"];
        auto parseOne = [&](const char* key, TrailHistoryProfile& dst) {
            if (!tp.contains(key) || !tp[key].is_object()) return;
            const auto& o = tp[key];
            dst.durationMs = GetOr<int>(o, "duration_ms", dst.durationMs);
            dst.maxPoints = GetOr<int>(o, "max_points", dst.maxPoints);
            dst = SanitizeTrailHistoryProfile(dst);
        };

        parseOne("line", cfg.trailProfiles.line);
        parseOne("streamer", cfg.trailProfiles.streamer);
        parseOne("electric", cfg.trailProfiles.electric);
        parseOne("meteor", cfg.trailProfiles.meteor);
        parseOne("tubes", cfg.trailProfiles.tubes);
    }
    
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

        // --- Text Click ---
        if (effects.contains("text_click") && effects["text_click"].is_object()) {
            const auto& t = effects["text_click"];
            cfg.textClick.durationMs = GetOr<int>(t, "duration_ms", cfg.textClick.durationMs);
            cfg.textClick.floatDistance = GetOr<int>(t, "float_distance", cfg.textClick.floatDistance);
            cfg.textClick.fontSize = GetOr<float>(t, "font_size", cfg.textClick.fontSize);
            
            if (t.contains("font_family") && t["font_family"].is_string()) {
                std::string font = t["font_family"].get<std::string>();
                int len = MultiByteToWideChar(CP_UTF8, 0, font.c_str(), -1, nullptr, 0);
                if (len > 0) {
                    cfg.textClick.fontFamily.resize(len - 1);
                    MultiByteToWideChar(CP_UTF8, 0, font.c_str(), -1, &cfg.textClick.fontFamily[0], len);
                }
            }

            if (t.contains("texts") && t["texts"].is_array()) {
                cfg.textClick.texts.clear();
                for (const auto& item : t["texts"]) {
                    if (item.is_string()) {
                        std::string s = item.get<std::string>();
                        int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
                        if (len > 0) {
                            std::wstring ws(len - 1, L'\0');
                            MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, &ws[0], len);
                            cfg.textClick.texts.push_back(ws);
                        }
                    }
                }
            }

            if (t.contains("colors") && t["colors"].is_array()) {
                cfg.textClick.colors.clear();
                for (const auto& item : t["colors"]) {
                    if (item.is_string()) {
                        cfg.textClick.colors.push_back(ArgbFromHex(item.get<std::string>()));
                    }
                }
            }
        }
    }
    
#ifdef _DEBUG
    OutputDebugStringW(L"MouseFx: config.json loaded successfully.\n");
#endif
    return cfg;
}

bool EffectConfig::Save(const std::wstring& exeDir, const EffectConfig& cfg) {
    std::wstring configPath = exeDir + L"\\config.json";

    json root;
    root["default_effect"] = cfg.defaultEffect;
    root["theme"] = cfg.theme;
    root["ui_language"] = cfg.uiLanguage;
    root["hold_follow_mode"] = NormalizeHoldFollowMode(cfg.holdFollowMode);
    root["flux_gpu_v2_d2d_experimental"] = cfg.fluxGpuV2D2dExperimental;
    root["trail_style"] = cfg.trailStyle;

    // Trail history profiles (strategy-based trail renderers)
    {
        json tp;
        auto addOne = [&](const char* key, TrailHistoryProfile p) {
            p = SanitizeTrailHistoryProfile(p);
            json o;
            o["duration_ms"] = p.durationMs;
            o["max_points"] = p.maxPoints;
            tp[key] = o;
        };
        addOne("line", cfg.trailProfiles.line);
        addOne("streamer", cfg.trailProfiles.streamer);
        addOne("electric", cfg.trailProfiles.electric);
        addOne("meteor", cfg.trailProfiles.meteor);
        addOne("tubes", cfg.trailProfiles.tubes);
        root["trail_profiles"] = tp;
    }

    // Trail renderer params
    {
        const auto p = SanitizeTrailParams(cfg.trailParams);
        json tp;
        {
            json o;
            o["glow_width_scale"] = p.streamer.glowWidthScale;
            o["core_width_scale"] = p.streamer.coreWidthScale;
            o["head_power"] = p.streamer.headPower;
            tp["streamer"] = o;
        }
        {
            json o;
            o["amplitude_scale"] = p.electric.amplitudeScale;
            o["fork_chance"] = p.electric.forkChance;
            tp["electric"] = o;
        }
        {
            json o;
            o["spark_rate_scale"] = p.meteor.sparkRateScale;
            o["spark_speed_scale"] = p.meteor.sparkSpeedScale;
            tp["meteor"] = o;
        }
        if (p.idleFade.startMs > 0) tp["idle_fade_start_ms"] = p.idleFade.startMs;
        if (p.idleFade.endMs > 0) tp["idle_fade_end_ms"] = p.idleFade.endMs;
        root["trail_params"] = tp;
    }

    json active;
    active["click"] = cfg.active.click;
    active["trail"] = cfg.active.trail;
    active["scroll"] = cfg.active.scroll;
    active["hover"] = cfg.active.hover;
    active["hold"] = cfg.active.hold;
    root["active_effects"] = active;

    json effects;
    // Ripple
    {
        json r;
        r["duration_ms"] = cfg.ripple.durationMs;
        r["start_radius"] = cfg.ripple.startRadius;
        r["end_radius"] = cfg.ripple.endRadius;
        r["stroke_width"] = cfg.ripple.strokeWidth;
        r["window_size"] = cfg.ripple.windowSize;

        json lc;
        lc["fill"] = ArgbToHex(cfg.ripple.leftClick.fill);
        lc["stroke"] = ArgbToHex(cfg.ripple.leftClick.stroke);
        lc["glow"] = ArgbToHex(cfg.ripple.leftClick.glow);
        r["left_click"] = lc;

        json rc;
        rc["fill"] = ArgbToHex(cfg.ripple.rightClick.fill);
        rc["stroke"] = ArgbToHex(cfg.ripple.rightClick.stroke);
        rc["glow"] = ArgbToHex(cfg.ripple.rightClick.glow);
        r["right_click"] = rc;

        json mc;
        mc["fill"] = ArgbToHex(cfg.ripple.middleClick.fill);
        mc["stroke"] = ArgbToHex(cfg.ripple.middleClick.stroke);
        mc["glow"] = ArgbToHex(cfg.ripple.middleClick.glow);
        r["middle_click"] = mc;

        effects["ripple"] = r;
    }
    // Trail
    {
        json t;
        t["duration_ms"] = cfg.trail.durationMs;
        t["max_points"] = cfg.trail.maxPoints;
        t["line_width"] = cfg.trail.lineWidth;
        t["color"] = ArgbToHex(cfg.trail.color);
        effects["trail"] = t;
    }
    // Icon/Star
    {
        json i;
        i["duration_ms"] = cfg.icon.durationMs;
        i["start_radius"] = cfg.icon.startRadius;
        i["end_radius"] = cfg.icon.endRadius;
        i["stroke_width"] = cfg.icon.strokeWidth;
        i["fill"] = ArgbToHex(cfg.icon.fillColor);
        i["stroke"] = ArgbToHex(cfg.icon.strokeColor);
        effects["icon_star"] = i;
    }
    // Text Click
    {
        json t;
        t["duration_ms"] = cfg.textClick.durationMs;
        t["float_distance"] = cfg.textClick.floatDistance;
        t["font_size"] = cfg.textClick.fontSize;
        t["font_family"] = WStringToUtf8(cfg.textClick.fontFamily);

        json texts = json::array();
        for (const auto& ws : cfg.textClick.texts) {
            texts.push_back(WStringToUtf8(ws));
        }
        t["texts"] = texts;

        json colors = json::array();
        for (const auto& c : cfg.textClick.colors) {
            colors.push_back(ArgbToHex(c));
        }
        t["colors"] = colors;
        effects["text_click"] = t;
    }

    root["effects"] = effects;

    std::ofstream file(configPath);
    if (!file.is_open()) {
        return false;
    }
    file << root.dump(2);
    return true;
}

} // namespace mousefx

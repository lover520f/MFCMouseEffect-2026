#include "pch.h"
#include "ThemeStyle.h"
#include <algorithm>
#include <cmath>
#include <random>

namespace mousefx {

std::string ToLowerAscii(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        if (c >= 'A' && c <= 'Z') out.push_back(static_cast<char>(c - 'A' + 'a'));
        else out.push_back(c);
    }
    return out;
}

static RippleStyle MakeStyle(uint32_t durationMs, int windowSize, float startR, float endR, float stroke, Argb fill, Argb strokeColor, Argb glow) {
    RippleStyle s;
    s.durationMs = durationMs;
    s.windowSize = windowSize;
    s.startRadius = startR;
    s.endRadius = endR;
    s.strokeWidth = stroke;
    s.fill = fill;
    s.stroke = strokeColor;
    s.glow = glow;
    return s;
}

// Convert HSL (Hue 0-360, Sat 0-1, Light 0-1) to RGB
static void HslToRgb(float h, float s, float l, uint8_t& r, uint8_t& g, uint8_t& b) {
    auto hue2rgb = [](float p, float q, float t) {
        if (t < 0.0f) t += 1.0f;
        if (t > 1.0f) t -= 1.0f;
        if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
        if (t < 1.0f / 2.0f) return q;
        if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
        return p;
    };

    float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
    float p = 2.0f * l - q;
    
    float tr = h / 360.0f + 1.0f / 3.0f;
    float tg = h / 360.0f;
    float tb = h / 360.0f - 1.0f / 3.0f;

    r = static_cast<uint8_t>(hue2rgb(p, q, tr) * 255.0f);
    g = static_cast<uint8_t>(hue2rgb(p, q, tg) * 255.0f);
    b = static_cast<uint8_t>(hue2rgb(p, q, tb) * 255.0f);
}

// Global random engine
static std::mt19937 g_rng(std::random_device{}());

Argb MakeRandomColor(uint8_t alpha) {
    // Generate vibrant color: High Saturation (0.7-1.0), High Lightness (0.5-0.7)
    std::uniform_real_distribution<float> distHue(0.0f, 360.0f);
    std::uniform_real_distribution<float> distSat(0.7f, 1.0f);
    std::uniform_real_distribution<float> distLig(0.5f, 0.7f);

    uint8_t r, g, b;
    HslToRgb(distHue(g_rng), distSat(g_rng), distLig(g_rng), r, g, b);
    
    return { (uint32_t)((alpha << 24) | (r << 16) | (g << 8) | b) };
}

RippleStyle MakeRandomStyle(const RippleStyle& base) {
    RippleStyle s = base;
    
    // Generate a main random color
    uint8_t baseAlpha = (base.stroke.value >> 24) & 0xFF; // Use stroke alpha as reference
    Argb mainColor = MakeRandomColor(baseAlpha);
    
    // Apply to stroke
    s.stroke = mainColor;
    
    // Apply to fill (preserve original alpha ratio)
    uint8_t fillAlpha = (base.fill.value >> 24) & 0xFF;
    if (fillAlpha > 0) {
        // Use same color but with fill alpha
        s.fill = { (mainColor.value & 0x00FFFFFF) | ((uint32_t)fillAlpha << 24) };
    }
    
    // Apply to glow (preserve original alpha ratio)
    uint8_t glowAlpha = (base.glow.value >> 24) & 0xFF;
    if (glowAlpha > 0) {
        s.glow = { (mainColor.value & 0x00FFFFFF) | ((uint32_t)glowAlpha << 24) };
    }
    
    return s;
}

ThemePalette GetThemePalette(const std::string& themeName) {
    std::string t = ToLowerAscii(themeName);
    
    // Chromatic: return a template (e.g. Neon) but marked as chromatic?
    // Actually, Effects will check themeName "chromatic" and override.
    // So here we just return a sensible default for initialization.
    if (t == "chromatic") {
         // Return Neon as base
         t = "neon";
    }

    // Sci-Fi: cool cyan + electric blue
    if (t == "scifi" || t == "sci-fi" || t == "sci_fi") {
        return ThemePalette{
            MakeStyle(320, 200, 12.0f, 58.0f, 2.6f, {0x202EF2FF}, {0xFF5BD9FF}, {0x662EF2FF}),
            MakeStyle(320, 200, 12.0f, 52.0f, 2.4f, {0x2035E7FF}, {0xFF4FD9FF}, {0x6635E7FF}),
            MakeStyle(220, 180, 8.0f, 56.0f, 2.6f, {0x2035E7FF}, {0xFF4FD9FF}, {0x6635E7FF}),
            MakeStyle(900, 220, 10.0f, 68.0f, 2.8f, {0x00000000}, {0xFF66E0FF}, {0x6635C8FF}),
            MakeStyle(2600, 200, 6.0f, 60.0f, 2.2f, {0x00000000}, {0xFF5BD9FF}, {0x5535C8FF})
        };
    }

    // Minimal: clean white/soft gray, minimal glow
    if (t == "minimal") {
        return ThemePalette{
            MakeStyle(320, 200, 12.0f, 56.0f, 2.2f, {0x10FFFFFF}, {0xFFDADFE3}, {0x22FFFFFF}),
            MakeStyle(320, 200, 12.0f, 50.0f, 2.0f, {0x10FFFFFF}, {0xFFE6EBEF}, {0x22FFFFFF}),
            MakeStyle(200, 160, 8.0f, 48.0f, 2.0f, {0x10FFFFFF}, {0xFFDADFE3}, {0x22FFFFFF}),
            MakeStyle(850, 210, 10.0f, 64.0f, 2.2f, {0x00000000}, {0xFFE6EBEF}, {0x22FFFFFF}),
            MakeStyle(2400, 180, 6.0f, 54.0f, 1.8f, {0x00000000}, {0xFFDADFE3}, {0x22FFFFFF})
        };
    }

    // Game: vivid green + amber
    if (t == "game") {
        return ThemePalette{
            MakeStyle(320, 210, 12.0f, 60.0f, 2.8f, {0x2033FF77}, {0xFF33FF77}, {0x6633FF77}),
            MakeStyle(320, 210, 12.0f, 52.0f, 2.6f, {0x20FFB74D}, {0xFFFFB74D}, {0x66FFB74D}),
            MakeStyle(210, 190, 10.0f, 62.0f, 3.0f, {0x2033FF77}, {0xFF33FF77}, {0x6633FF77}),
            MakeStyle(820, 230, 12.0f, 72.0f, 3.0f, {0x00000000}, {0xFFFFB74D}, {0x66FFB74D}),
            MakeStyle(2400, 200, 8.0f, 58.0f, 2.4f, {0x00000000}, {0xFF7CFF7C}, {0x447CFF7C})
        };
    }

    // Neon (default): magenta + cyan
    return ThemePalette{
        MakeStyle(320, 210, 12.0f, 60.0f, 2.8f, {0x20FF5AF7}, {0xFFFF5AF7}, {0x66FF5AF7}),
        MakeStyle(320, 210, 12.0f, 52.0f, 2.6f, {0x204DF8FF}, {0xFF4DF8FF}, {0x664DF8FF}),
        MakeStyle(230, 180, 8.0f, 58.0f, 2.8f, {0x2032C8FF}, {0xFFFF5AF7}, {0x66FF5AF7}),
        MakeStyle(900, 220, 10.0f, 70.0f, 2.8f, {0x00000000}, {0xFF4DF8FF}, {0x664DF8FF}),
        MakeStyle(2600, 190, 6.0f, 56.0f, 2.2f, {0x00000000}, {0xFFFF5AF7}, {0x66FF5AF7})
    };
}

} // namespace mousefx

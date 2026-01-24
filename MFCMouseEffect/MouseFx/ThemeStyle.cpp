#include "pch.h"
#include "ThemeStyle.h"

namespace mousefx {

static std::string ToLowerAscii(const std::string& s) {
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

ThemePalette GetThemePalette(const std::string& themeName) {
    const std::string t = ToLowerAscii(themeName);

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

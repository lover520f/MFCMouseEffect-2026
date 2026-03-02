#include "pch.h"
#include "ThemeStyle.h"

#include "ThemeCatalogLoader.h"

#include <algorithm>
#include <cmath>
#include <mutex>
#include <random>
#include <vector>

namespace mousefx {
namespace {

struct ThemeCatalogRecord {
    std::string value;
    std::wstring labelZh;
    std::wstring labelEn;
    ThemePalette palette;
    bool builtIn = false;
};

struct ThemeCatalogState {
    std::vector<ThemeCatalogRecord> records;
    std::vector<ThemeOption> options;
    ThemeCatalogRuntimeInfo runtimeInfo;
};

RippleStyle MakeStyle(
    uint32_t durationMs,
    int windowSize,
    float startR,
    float endR,
    float stroke,
    Argb fill,
    Argb strokeColor,
    Argb glow) {
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

void HslToRgb(float h, float s, float l, uint8_t& r, uint8_t& g, uint8_t& b) {
    auto hue2rgb = [](float p, float q, float t) {
        if (t < 0.0f) t += 1.0f;
        if (t > 1.0f) t -= 1.0f;
        if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
        if (t < 1.0f / 2.0f) return q;
        if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
        return p;
    };

    const float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
    const float p = 2.0f * l - q;

    const float tr = h / 360.0f + 1.0f / 3.0f;
    const float tg = h / 360.0f;
    const float tb = h / 360.0f - 1.0f / 3.0f;

    r = static_cast<uint8_t>(hue2rgb(p, q, tr) * 255.0f);
    g = static_cast<uint8_t>(hue2rgb(p, q, tg) * 255.0f);
    b = static_cast<uint8_t>(hue2rgb(p, q, tb) * 255.0f);
}

std::string NormalizeThemeNameInternal(const std::string& themeName) {
    std::string normalized = ToLowerAscii(TrimAscii(themeName));
    if (normalized == "sci-fi" || normalized == "sci_fi") {
        normalized = "scifi";
    }
    return normalized;
}

std::vector<ThemeCatalogRecord> BuildBuiltInThemeRecords() {
    return {
        {
            "chromatic",
            L"\u70ab\u5f69",
            L"Chromatic",
            ThemePalette{
                MakeStyle(320, 210, 12.0f, 60.0f, 2.8f, {0x20FF5AF7}, {0xFFFF5AF7}, {0x66FF5AF7}),
                MakeStyle(320, 210, 12.0f, 52.0f, 2.6f, {0x204DF8FF}, {0xFF4DF8FF}, {0x664DF8FF}),
                MakeStyle(230, 180, 8.0f, 58.0f, 2.8f, {0x2032C8FF}, {0xFFFF5AF7}, {0x66FF5AF7}),
                MakeStyle(900, 220, 10.0f, 70.0f, 2.8f, {0x00000000}, {0xFF4DF8FF}, {0x664DF8FF}),
                MakeStyle(2600, 190, 6.0f, 56.0f, 2.2f, {0x00000000}, {0xFFFF5AF7}, {0x66FF5AF7}),
            },
            true,
        },
        {
            "neon",
            L"\u9713\u8679",
            L"Neon",
            ThemePalette{
                MakeStyle(320, 210, 12.0f, 60.0f, 2.8f, {0x20FF5AF7}, {0xFFFF5AF7}, {0x66FF5AF7}),
                MakeStyle(320, 210, 12.0f, 52.0f, 2.6f, {0x204DF8FF}, {0xFF4DF8FF}, {0x664DF8FF}),
                MakeStyle(230, 180, 8.0f, 58.0f, 2.8f, {0x2032C8FF}, {0xFFFF5AF7}, {0x66FF5AF7}),
                MakeStyle(900, 220, 10.0f, 70.0f, 2.8f, {0x00000000}, {0xFF4DF8FF}, {0x664DF8FF}),
                MakeStyle(2600, 190, 6.0f, 56.0f, 2.2f, {0x00000000}, {0xFFFF5AF7}, {0x66FF5AF7}),
            },
            true,
        },
        {
            "scifi",
            L"\u79d1\u5e7b",
            L"Sci-Fi",
            ThemePalette{
                MakeStyle(320, 200, 12.0f, 58.0f, 2.6f, {0x202EF2FF}, {0xFF5BD9FF}, {0x662EF2FF}),
                MakeStyle(320, 200, 12.0f, 52.0f, 2.4f, {0x2035E7FF}, {0xFF4FD9FF}, {0x6635E7FF}),
                MakeStyle(220, 180, 8.0f, 56.0f, 2.6f, {0x2035E7FF}, {0xFF4FD9FF}, {0x6635E7FF}),
                MakeStyle(900, 220, 10.0f, 68.0f, 2.8f, {0x00000000}, {0xFF66E0FF}, {0x6635C8FF}),
                MakeStyle(2600, 200, 6.0f, 60.0f, 2.2f, {0x00000000}, {0xFF5BD9FF}, {0x5535C8FF}),
            },
            true,
        },
        {
            "minimal",
            L"\u6781\u7b80",
            L"Minimal",
            ThemePalette{
                MakeStyle(320, 200, 12.0f, 56.0f, 2.2f, {0x10FFFFFF}, {0xFFDADFE3}, {0x22FFFFFF}),
                MakeStyle(320, 200, 12.0f, 50.0f, 2.0f, {0x10FFFFFF}, {0xFFE6EBEF}, {0x22FFFFFF}),
                MakeStyle(200, 160, 8.0f, 48.0f, 2.0f, {0x10FFFFFF}, {0xFFDADFE3}, {0x22FFFFFF}),
                MakeStyle(850, 210, 10.0f, 64.0f, 2.2f, {0x00000000}, {0xFFE6EBEF}, {0x22FFFFFF}),
                MakeStyle(2400, 180, 6.0f, 54.0f, 1.8f, {0x00000000}, {0xFFDADFE3}, {0x22FFFFFF}),
            },
            true,
        },
        {
            "game",
            L"\u6e38\u620f\u611f",
            L"Game",
            ThemePalette{
                MakeStyle(320, 210, 12.0f, 60.0f, 2.8f, {0x2033FF77}, {0xFF33FF77}, {0x6633FF77}),
                MakeStyle(320, 210, 12.0f, 52.0f, 2.6f, {0x20FFB74D}, {0xFFFFB74D}, {0x66FFB74D}),
                MakeStyle(210, 190, 10.0f, 62.0f, 3.0f, {0x2033FF77}, {0xFF33FF77}, {0x6633FF77}),
                MakeStyle(820, 230, 12.0f, 72.0f, 3.0f, {0x00000000}, {0xFFFFB74D}, {0x66FFB74D}),
                MakeStyle(2400, 200, 8.0f, 58.0f, 2.4f, {0x00000000}, {0xFF7CFF7C}, {0x447CFF7C}),
            },
            true,
        },
    };
}

std::vector<ThemeOption> BuildThemeOptions(const std::vector<ThemeCatalogRecord>& records) {
    std::vector<ThemeOption> options;
    options.reserve(records.size());
    for (const auto& record : records) {
        options.push_back({
            record.value,
            record.labelZh.empty() ? Utf8ToWString(record.value) : record.labelZh,
            record.labelEn.empty() ? Utf8ToWString(record.value) : record.labelEn,
        });
    }
    return options;
}

ThemeCatalogState BuildBuiltInThemeCatalogState() {
    ThemeCatalogState state;
    state.records = BuildBuiltInThemeRecords();
    state.options = BuildThemeOptions(state.records);
    state.runtimeInfo.builtInThemeCount = state.records.size();
    state.runtimeInfo.runtimeThemeCount = state.options.size();
    return state;
}

ThemeCatalogState& MutableThemeCatalogState() {
    static ThemeCatalogState state = BuildBuiltInThemeCatalogState();
    return state;
}

std::mutex& ThemeCatalogMutex() {
    static std::mutex lock;
    return lock;
}

const ThemeCatalogRecord* FindThemeRecord(const ThemeCatalogState& state, const std::string& normalizedValue) {
    const auto it = std::find_if(
        state.records.begin(),
        state.records.end(),
        [&](const ThemeCatalogRecord& record) {
            return record.value == normalizedValue;
        });
    if (it == state.records.end()) {
        return nullptr;
    }
    return &(*it);
}

std::string ResolveThemeNameFromState(const ThemeCatalogState& state, const std::string& themeName) {
    std::string normalized = NormalizeThemeNameInternal(themeName);

    if (const ThemeCatalogRecord* record = FindThemeRecord(state, normalized)) {
        return record->value;
    }
    if (const ThemeCatalogRecord* fallback = FindThemeRecord(state, "neon")) {
        return fallback->value;
    }
    if (!state.records.empty()) {
        return state.records.front().value;
    }
    return normalized.empty() ? std::string("neon") : normalized;
}

void MergeExternalThemeEntry(
    const ThemeCatalogExternalEntry& entry,
    std::vector<ThemeCatalogRecord>* records) {
    if (!records) {
        return;
    }

    const auto it = std::find_if(
        records->begin(),
        records->end(),
        [&](const ThemeCatalogRecord& current) {
            return current.value == entry.value;
        });

    ThemeCatalogRecord next{
        entry.value,
        entry.labelZh,
        entry.labelEn,
        entry.palette,
        false,
    };

    if (it != records->end()) {
        *it = std::move(next);
        return;
    }
    records->push_back(std::move(next));
}

std::mt19937 g_rng(std::random_device{}());

} // namespace

Argb MakeRandomColor(uint8_t alpha) {
    std::uniform_real_distribution<float> distHue(0.0f, 360.0f);
    std::uniform_real_distribution<float> distSat(0.7f, 1.0f);
    std::uniform_real_distribution<float> distLig(0.5f, 0.7f);

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    HslToRgb(distHue(g_rng), distSat(g_rng), distLig(g_rng), r, g, b);

    return {(uint32_t)((alpha << 24) | (r << 16) | (g << 8) | b)};
}

RippleStyle MakeRandomStyle(const RippleStyle& base) {
    RippleStyle s = base;

    const uint8_t baseAlpha = static_cast<uint8_t>((base.stroke.value >> 24) & 0xFF);
    const Argb mainColor = MakeRandomColor(baseAlpha);
    s.stroke = mainColor;

    const uint8_t fillAlpha = static_cast<uint8_t>((base.fill.value >> 24) & 0xFF);
    if (fillAlpha > 0) {
        s.fill = {(mainColor.value & 0x00FFFFFF) | (static_cast<uint32_t>(fillAlpha) << 24)};
    }

    const uint8_t glowAlpha = static_cast<uint8_t>((base.glow.value >> 24) & 0xFF);
    if (glowAlpha > 0) {
        s.glow = {(mainColor.value & 0x00FFFFFF) | (static_cast<uint32_t>(glowAlpha) << 24)};
    }

    return s;
}

std::string NormalizeThemeName(const std::string& themeName) {
    return NormalizeThemeNameInternal(themeName);
}

std::string ResolveRuntimeThemeName(const std::string& themeName) {
    std::lock_guard<std::mutex> guard(ThemeCatalogMutex());
    const ThemeCatalogState& state = MutableThemeCatalogState();
    return ResolveThemeNameFromState(state, themeName);
}

void ReloadThemeCatalogFromRootPath(const std::string& rootPathUtf8) {
    ThemeCatalogState nextState = BuildBuiltInThemeCatalogState();
    const size_t builtInCount = nextState.records.size();

    ThemeCatalogLoader loader;
    const ThemeCatalogLoadResult loadResult = loader.LoadFromRootPath(rootPathUtf8);
    size_t mergeRejectedFiles = loadResult.rejectedFiles;
    for (const auto& entry : loadResult.entries) {
        const ThemeCatalogRecord* existing = FindThemeRecord(nextState, entry.value);
        if (existing != nullptr && existing->builtIn) {
            // External packages are not allowed to override built-in theme semantics.
            ++mergeRejectedFiles;
            continue;
        }
        MergeExternalThemeEntry(entry, &nextState.records);
    }

    nextState.options = BuildThemeOptions(nextState.records);
    nextState.runtimeInfo.configuredRootPath = loadResult.configuredRootPath;
    nextState.runtimeInfo.scannedExternalThemeFiles = loadResult.scannedFiles;
    nextState.runtimeInfo.externalThemeCount = 0;
    for (const auto& record : nextState.records) {
        if (!record.builtIn) {
            ++nextState.runtimeInfo.externalThemeCount;
        }
    }
    nextState.runtimeInfo.rejectedExternalThemeFiles = mergeRejectedFiles;
    nextState.runtimeInfo.builtInThemeCount = builtInCount;
    nextState.runtimeInfo.runtimeThemeCount = nextState.options.size();

    std::lock_guard<std::mutex> guard(ThemeCatalogMutex());
    MutableThemeCatalogState() = std::move(nextState);
}

std::vector<ThemeOption> GetThemeOptions() {
    std::lock_guard<std::mutex> guard(ThemeCatalogMutex());
    return MutableThemeCatalogState().options;
}

ThemeCatalogRuntimeInfo GetThemeCatalogRuntimeInfo() {
    std::lock_guard<std::mutex> guard(ThemeCatalogMutex());
    return MutableThemeCatalogState().runtimeInfo;
}

ThemePalette GetThemePalette(const std::string& themeName) {
    std::lock_guard<std::mutex> guard(ThemeCatalogMutex());
    const ThemeCatalogState& state = MutableThemeCatalogState();
    const std::string resolvedTheme = ResolveThemeNameFromState(state, themeName);

    if (const ThemeCatalogRecord* record = FindThemeRecord(state, resolvedTheme)) {
        return record->palette;
    }
    if (const ThemeCatalogRecord* fallback = FindThemeRecord(state, "neon")) {
        return fallback->palette;
    }
    if (!state.records.empty()) {
        return state.records.front().palette;
    }
    return ThemePalette{};
}

} // namespace mousefx

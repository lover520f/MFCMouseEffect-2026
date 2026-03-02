#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include "RippleStyle.h"
#include "MouseFx/Utils/StringUtils.h"

namespace mousefx {

struct ThemePalette {
    RippleStyle click;
    RippleStyle icon;
    RippleStyle scroll;
    RippleStyle hold;
    RippleStyle hover;
};

struct ThemeOption {
    std::string value;
    std::wstring labelZh;
    std::wstring labelEn;
};

struct ThemeCatalogRuntimeInfo {
    std::string configuredRootPath;
    size_t builtInThemeCount = 0;
    size_t runtimeThemeCount = 0;
    size_t scannedExternalThemeFiles = 0;
    size_t externalThemeCount = 0;
    size_t rejectedExternalThemeFiles = 0;
};

// Normalizes theme aliases to canonical values (e.g. sci-fi -> scifi).
std::string NormalizeThemeName(const std::string& themeName);

// Resolves configured theme into a runtime-available catalog value (fallback-safe).
std::string ResolveRuntimeThemeName(const std::string& themeName);

// Reloads theme catalog using built-in themes + optional external package directory.
void ReloadThemeCatalogFromRootPath(const std::string& rootPathUtf8);

// Returns runtime theme catalog in stable UI order.
std::vector<ThemeOption> GetThemeOptions();

ThemeCatalogRuntimeInfo GetThemeCatalogRuntimeInfo();

ThemePalette GetThemePalette(const std::string& themeName);

// Generates a random style based on the input template but with a random vibrant color.
RippleStyle MakeRandomStyle(const RippleStyle& base);

// Generates a random vibrant color (alpha is preserved from input arg if needed, else full).
Argb MakeRandomColor(uint8_t alpha = 255);

} // namespace mousefx

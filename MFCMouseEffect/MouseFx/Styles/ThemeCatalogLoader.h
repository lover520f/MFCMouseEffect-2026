#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "ThemeStyle.h"

namespace mousefx {

struct ThemeCatalogExternalEntry {
    std::string value;
    std::wstring labelZh;
    std::wstring labelEn;
    ThemePalette palette;
};

struct ThemeCatalogLoadResult {
    std::string configuredRootPath;
    std::vector<ThemeCatalogExternalEntry> entries;
    size_t scannedFiles = 0;
    size_t rejectedFiles = 0;
};

class ThemeCatalogLoader final {
public:
    ThemeCatalogLoadResult LoadFromRootPath(const std::string& rootPathUtf8) const;
};

} // namespace mousefx

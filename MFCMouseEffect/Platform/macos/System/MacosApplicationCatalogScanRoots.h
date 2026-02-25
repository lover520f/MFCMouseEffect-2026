#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace mousefx::platform::macos {

struct MacosApplicationCatalogScanRoot final {
    std::filesystem::path path;
    std::string source;
};

std::vector<MacosApplicationCatalogScanRoot> BuildMacosApplicationCatalogScanRoots();

} // namespace mousefx::platform::macos

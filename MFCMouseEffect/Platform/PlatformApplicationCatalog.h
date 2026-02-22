#pragma once

#include <string>
#include <vector>

namespace mousefx::platform {

struct ApplicationCatalogEntry final {
    std::string processName;
    std::string displayName;
    std::string source;
};

std::vector<ApplicationCatalogEntry> ScanApplicationCatalog();

} // namespace mousefx::platform

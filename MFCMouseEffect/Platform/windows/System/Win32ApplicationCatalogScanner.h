#pragma once

#include "Platform/PlatformApplicationCatalog.h"

#include <string>
#include <vector>

namespace mousefx::platform::windows {

// Scans local application entry points (Start Menu/Desktop) and resolves them
// to process executable names for automation scope selection.
class Win32ApplicationCatalogScanner final {
public:
    std::vector<ApplicationCatalogEntry> Scan() const;
};

} // namespace mousefx::platform::windows

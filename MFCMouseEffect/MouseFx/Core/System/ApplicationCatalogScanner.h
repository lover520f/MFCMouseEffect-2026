#pragma once

#include <string>
#include <vector>

namespace mousefx {

struct ApplicationCatalogEntry {
    std::string processName;
    std::string displayName;
    std::string source;
};

// Scans local application entry points (Start Menu/Desktop) and resolves them
// to process executable names for automation scope selection.
class ApplicationCatalogScanner final {
public:
    std::vector<ApplicationCatalogEntry> Scan() const;
};

} // namespace mousefx


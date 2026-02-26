#include "pch.h"

#include "Platform/macos/System/MacosApplicationCatalogScanWorkflow.Internal.h"

#include "Platform/macos/System/MacosApplicationCatalogEntryStore.h"
#include <string>
#include <unordered_map>
#include <vector>

namespace mousefx::platform::macos::application_catalog_scan_detail {

void ScanMacosApplicationCatalogRoot(
    const MacosApplicationCatalogScanRoot& root,
    std::vector<ApplicationCatalogEntry>* entries,
    std::unordered_map<std::string, size_t>* indexByProcess) {
    if (!entries || !indexByProcess) {
        return;
    }

    std::vector<std::string> bundlePaths;
    CollectMacosApplicationBundlePaths(root, &bundlePaths);
    for (const std::string& bundlePath : bundlePaths) {
        std::string processName;
        std::string displayName;
        if (!ResolveMacosApplicationCatalogEntryFromPath(bundlePath, &processName, &displayName)) {
            continue;
        }
        UpsertMacosApplicationCatalogEntry(processName, displayName, root.source, entries, indexByProcess);
    }
}

} // namespace mousefx::platform::macos::application_catalog_scan_detail

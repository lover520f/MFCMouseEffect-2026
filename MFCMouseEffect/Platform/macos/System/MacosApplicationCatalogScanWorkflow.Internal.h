#pragma once

#include "Platform/PlatformApplicationCatalog.h"
#include "Platform/macos/System/MacosApplicationCatalogScanRoots.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace mousefx::platform::macos::application_catalog_scan_detail {

bool ResolveMacosApplicationCatalogEntryFromPath(
    const std::string& bundlePath,
    std::string* processName,
    std::string* displayName);

void ScanMacosApplicationCatalogRoot(
    const MacosApplicationCatalogScanRoot& root,
    std::vector<ApplicationCatalogEntry>* entries,
    std::unordered_map<std::string, size_t>* indexByProcess);

void CollectMacosApplicationBundlePaths(
    const MacosApplicationCatalogScanRoot& root,
    std::vector<std::string>* bundlePaths);

} // namespace mousefx::platform::macos::application_catalog_scan_detail

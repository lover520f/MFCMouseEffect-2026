#pragma once

#include "Platform/PlatformApplicationCatalog.h"

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

namespace mousefx::platform::macos {

void UpsertMacosApplicationCatalogEntry(
    const std::string& processName,
    const std::string& displayName,
    const std::string& source,
    std::vector<ApplicationCatalogEntry>* entries,
    std::unordered_map<std::string, size_t>* indexByProcess);
void SortMacosApplicationCatalogEntries(std::vector<ApplicationCatalogEntry>* entries);

} // namespace mousefx::platform::macos

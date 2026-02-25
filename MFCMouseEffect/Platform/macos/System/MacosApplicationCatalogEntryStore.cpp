#include "pch.h"

#include "Platform/macos/System/MacosApplicationCatalogEntryStore.h"

#include "MouseFx/Utils/StringUtils.h"

#include <algorithm>
#include <string>
#include <utility>

namespace mousefx::platform::macos {

void UpsertMacosApplicationCatalogEntry(
    const std::string& processName,
    const std::string& displayName,
    const std::string& source,
    std::vector<ApplicationCatalogEntry>* entries,
    std::unordered_map<std::string, size_t>* indexByProcess) {
    if (!entries || !indexByProcess || processName.empty()) {
        return;
    }

    auto found = indexByProcess->find(processName);
    if (found != indexByProcess->end()) {
        ApplicationCatalogEntry& existing = (*entries)[found->second];
        if (existing.displayName == existing.processName && !displayName.empty()) {
            existing.displayName = displayName;
        }
        return;
    }

    ApplicationCatalogEntry entry;
    entry.processName = processName;
    entry.displayName = displayName.empty() ? processName : displayName;
    entry.source = source;

    (*indexByProcess)[entry.processName] = entries->size();
    entries->push_back(std::move(entry));
}

void SortMacosApplicationCatalogEntries(std::vector<ApplicationCatalogEntry>* entries) {
    if (!entries) {
        return;
    }

    std::sort(entries->begin(), entries->end(), [](const ApplicationCatalogEntry& lhs, const ApplicationCatalogEntry& rhs) {
        const std::string leftLabel = ToLowerAscii(lhs.displayName);
        const std::string rightLabel = ToLowerAscii(rhs.displayName);
        if (leftLabel == rightLabel) {
            return lhs.processName < rhs.processName;
        }
        return leftLabel < rightLabel;
    });
}

} // namespace mousefx::platform::macos

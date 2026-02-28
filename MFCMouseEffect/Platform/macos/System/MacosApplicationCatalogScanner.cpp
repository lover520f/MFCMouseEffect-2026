#include "pch.h"

#include "Platform/macos/System/MacosApplicationCatalogScanner.h"

#include "Platform/macos/System/MacosApplicationCatalogEntryStore.h"
#include "Platform/macos/System/MacosApplicationCatalogSwiftBridge.h"

#include <array>
#include <string>
#include <unordered_map>
#include <vector>

namespace mousefx::platform::macos {
namespace {

constexpr int32_t kErrorBufferCapacity = 2048;

struct ApplicationCatalogScanContext final {
    std::vector<ApplicationCatalogEntry>* entries = nullptr;
    std::unordered_map<std::string, size_t>* indexByProcess = nullptr;
};

void EmitApplicationCatalogEntry(
    const char* processNameUtf8,
    const char* displayNameUtf8,
    const char* sourceUtf8,
    void* context) {
    if (!context) {
        return;
    }
    ApplicationCatalogScanContext* scanContext =
        static_cast<ApplicationCatalogScanContext*>(context);
    if (!scanContext->entries || !scanContext->indexByProcess) {
        return;
    }

    const std::string processName = processNameUtf8 ? processNameUtf8 : "";
    if (processName.empty()) {
        return;
    }
    const std::string displayName = displayNameUtf8 ? displayNameUtf8 : "";
    const std::string source = sourceUtf8 ? sourceUtf8 : "";
    UpsertMacosApplicationCatalogEntry(
        processName,
        displayName,
        source,
        scanContext->entries,
        scanContext->indexByProcess);
}

} // namespace

std::vector<ApplicationCatalogEntry> MacosApplicationCatalogScanner::Scan() const {
#if !defined(__APPLE__)
    return {};
#else
    std::vector<ApplicationCatalogEntry> entries;
    std::unordered_map<std::string, size_t> indexByProcess;
    ApplicationCatalogScanContext context{};
    context.entries = &entries;
    context.indexByProcess = &indexByProcess;

    std::array<char, kErrorBufferCapacity> errorBuffer{};
    const int32_t outcome = mfx_macos_scan_application_catalog_v1(
        &EmitApplicationCatalogEntry,
        &context,
        errorBuffer.data(),
        kErrorBufferCapacity);
    if (outcome < 0) {
        return {};
    }

    SortMacosApplicationCatalogEntries(&entries);
    return entries;
#endif
}

} // namespace mousefx::platform::macos

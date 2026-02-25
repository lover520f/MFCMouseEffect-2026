#include "pch.h"

#include "Platform/macos/System/MacosApplicationCatalogScanRoots.h"

#if defined(__APPLE__)
#import <Foundation/Foundation.h>
#endif

#include "MouseFx/Utils/StringUtils.h"

#include <filesystem>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

namespace mousefx::platform::macos {
namespace {

std::string NormalizeToken(std::string value) {
    return ToLowerAscii(TrimAscii(std::move(value)));
}

#if defined(__APPLE__)
std::string NsStringToUtf8(NSString* text) {
    if (text == nil || text.length == 0) {
        return {};
    }
    const char* raw = [text UTF8String];
    if (!raw || raw[0] == '\0') {
        return {};
    }
    return std::string(raw);
}
#endif

bool DirectoryExists(const std::filesystem::path& path) {
    std::error_code ec;
    return std::filesystem::exists(path, ec) && std::filesystem::is_directory(path, ec) && !ec;
}

} // namespace

std::vector<MacosApplicationCatalogScanRoot> BuildMacosApplicationCatalogScanRoots() {
    std::vector<MacosApplicationCatalogScanRoot> roots;
    std::unordered_set<std::string> dedup;

    const auto addRoot = [&](std::filesystem::path path, const char* source) {
        if (!DirectoryExists(path)) {
            return;
        }
        path = std::filesystem::weakly_canonical(path);
        const std::string dedupKey = NormalizeToken(path.string());
        if (dedupKey.empty() || !dedup.insert(dedupKey).second) {
            return;
        }
        roots.push_back(MacosApplicationCatalogScanRoot{std::move(path), source ? source : ""});
    };

    addRoot("/Applications", "applications");
    addRoot("/System/Applications", "system");

#if defined(__APPLE__)
    NSString* homePath = NSHomeDirectory();
    if (homePath != nil && homePath.length > 0) {
        const std::string homeUtf8 = NsStringToUtf8(homePath);
        if (!homeUtf8.empty()) {
            addRoot(std::filesystem::path(homeUtf8) / "Applications", "home");
        }
    }
#endif

    return roots;
}

} // namespace mousefx::platform::macos

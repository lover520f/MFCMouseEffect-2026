#include "pch.h"

#include "Platform/macos/System/MacosApplicationCatalogScanWorkflow.Internal.h"

#if defined(__APPLE__)
#import <Foundation/Foundation.h>
#endif

#include "MouseFx/Utils/StringUtils.h"

#include <filesystem>
#include <string>

namespace mousefx::platform::macos::application_catalog_scan_detail {
namespace {

std::string NormalizeToken(std::string value) {
    return ToLowerAscii(TrimAscii(std::move(value)));
}

std::string PathStemLower(const std::filesystem::path& path) {
    return NormalizeToken(path.stem().string());
}

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

std::string ResolveProcessName(NSURL* bundleUrl) {
    if (bundleUrl == nil) {
        return {};
    }

    const std::string bundlePath = NsStringToUtf8(bundleUrl.path);
    if (!bundlePath.empty()) {
        const std::string bundleStem = PathStemLower(std::filesystem::path(bundlePath));
        if (!bundleStem.empty()) {
            return bundleStem + ".app";
        }
    }

    NSBundle* bundle = [NSBundle bundleWithURL:bundleUrl];
    if (bundle != nil) {
        NSURL* executableUrl = [bundle executableURL];
        if (executableUrl != nil) {
            const std::string executableName = NormalizeToken(NsStringToUtf8(executableUrl.lastPathComponent));
            if (!executableName.empty()) {
                return executableName + ".app";
            }
        }

        id executableField = [bundle objectForInfoDictionaryKey:@"CFBundleExecutable"];
        if ([executableField isKindOfClass:[NSString class]]) {
            const std::string executableName = NormalizeToken(NsStringToUtf8((NSString*)executableField));
            if (!executableName.empty()) {
                return executableName + ".app";
            }
        }
    }
    return {};
}

std::string ResolveDisplayName(NSURL* bundleUrl, const std::string& fallback) {
    if (bundleUrl == nil) {
        return fallback;
    }

    NSFileManager* manager = [NSFileManager defaultManager];
    NSString* displayName = [manager displayNameAtPath:bundleUrl.path];
    std::string display = TrimAscii(NsStringToUtf8(displayName));
    if (!display.empty()) {
        return display;
    }

    const std::string bundlePath = NsStringToUtf8(bundleUrl.path);
    if (!bundlePath.empty()) {
        display = TrimAscii(std::filesystem::path(bundlePath).stem().string());
        if (!display.empty()) {
            return display;
        }
    }
    return fallback;
}

} // namespace

bool ResolveMacosApplicationCatalogEntryFromPath(
    const std::string& bundlePath,
    std::string* processName,
    std::string* displayName) {
#if !defined(__APPLE__)
    (void)bundlePath;
    (void)processName;
    (void)displayName;
    return false;
#else
    if (!processName || !displayName || bundlePath.empty()) {
        return false;
    }

    NSString* bundlePathNs = [NSString stringWithUTF8String:bundlePath.c_str()];
    if (bundlePathNs == nil || bundlePathNs.length == 0) {
        return false;
    }

    NSURL* bundleUrl = [NSURL fileURLWithPath:bundlePathNs isDirectory:YES];
    if (bundleUrl == nil) {
        return false;
    }

    const std::string resolvedProcess = ResolveProcessName(bundleUrl);
    if (resolvedProcess.empty()) {
        return false;
    }
    *processName = resolvedProcess;
    *displayName = ResolveDisplayName(bundleUrl, resolvedProcess);
    return true;
#endif
}

} // namespace mousefx::platform::macos::application_catalog_scan_detail

#include "pch.h"

#include "Platform/macos/System/MacosApplicationCatalogScanWorkflow.h"

#include "Platform/macos/System/MacosApplicationCatalogEntryStore.h"
#include "Platform/macos/System/MacosApplicationCatalogScanRoots.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#endif

#include "MouseFx/Utils/StringUtils.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace mousefx::platform::macos {
namespace {

std::string NormalizeToken(std::string value) {
    return ToLowerAscii(TrimAscii(std::move(value)));
}

std::string PathStemLower(const std::filesystem::path& path) {
    return NormalizeToken(path.stem().string());
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

NSString* NsStringFromUtf8(const std::string& text) {
    if (text.empty()) {
        return nil;
    }
    return [NSString stringWithUTF8String:text.c_str()];
}
#endif

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

void ScanSingleRoot(
    const MacosApplicationCatalogScanRoot& root,
    std::vector<ApplicationCatalogEntry>* entries,
    std::unordered_map<std::string, size_t>* indexByProcess) {
    if (!entries || !indexByProcess) {
        return;
    }

    NSString* rootPath = NsStringFromUtf8(root.path.string());
    if (rootPath == nil || rootPath.length == 0) {
        return;
    }

    NSURL* rootUrl = [NSURL fileURLWithPath:rootPath isDirectory:YES];
    if (rootUrl == nil) {
        return;
    }

    NSFileManager* manager = [NSFileManager defaultManager];
    NSArray<NSURLResourceKey>* keys = @[
        NSURLIsDirectoryKey,
        NSURLIsApplicationKey,
        NSURLNameKey,
    ];

    NSDirectoryEnumerator<NSURL*>* enumerator = [manager enumeratorAtURL:rootUrl
                                                includingPropertiesForKeys:keys
                                                                   options:NSDirectoryEnumerationSkipsHiddenFiles
                                                              errorHandler:^BOOL(NSURL* _Nonnull url, NSError* _Nonnull error) {
        (void)url;
        (void)error;
        return YES;
    }];
    if (enumerator == nil) {
        return;
    }

    for (NSURL* candidate in enumerator) {
        if (candidate == nil) {
            continue;
        }

        NSNumber* isDirectory = nil;
        [candidate getResourceValue:&isDirectory forKey:NSURLIsDirectoryKey error:nil];
        if (isDirectory == nil || !isDirectory.boolValue) {
            continue;
        }

        NSNumber* isApplication = nil;
        [candidate getResourceValue:&isApplication forKey:NSURLIsApplicationKey error:nil];
        if (isApplication == nil || !isApplication.boolValue) {
            continue;
        }

        [enumerator skipDescendants];

        const std::string processName = ResolveProcessName(candidate);
        if (processName.empty()) {
            continue;
        }
        const std::string displayName = ResolveDisplayName(candidate, processName);
        UpsertMacosApplicationCatalogEntry(processName, displayName, root.source, entries, indexByProcess);
    }
}

} // namespace

std::vector<ApplicationCatalogEntry> ScanMacosApplicationCatalogEntries() {
#if !defined(__APPLE__)
    return {};
#else
    std::vector<ApplicationCatalogEntry> entries;
    std::unordered_map<std::string, size_t> indexByProcess;

    const std::vector<MacosApplicationCatalogScanRoot> roots = BuildMacosApplicationCatalogScanRoots();
    for (const MacosApplicationCatalogScanRoot& root : roots) {
        ScanSingleRoot(root, &entries, &indexByProcess);
    }

    SortMacosApplicationCatalogEntries(&entries);
    return entries;
#endif
}

} // namespace mousefx::platform::macos

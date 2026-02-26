#include "pch.h"

#include "Platform/macos/System/MacosApplicationCatalogScanWorkflow.Internal.h"

#include "Platform/macos/System/MacosApplicationCatalogEntryStore.h"

#if defined(__APPLE__)
#import <Foundation/Foundation.h>
#endif

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace mousefx::platform::macos::application_catalog_scan_detail {
namespace {

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

} // namespace

void ScanMacosApplicationCatalogRoot(
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

        const std::string bundlePath = NsStringToUtf8(candidate.path);
        std::string processName;
        std::string displayName;
        if (!ResolveMacosApplicationCatalogEntryFromPath(bundlePath, &processName, &displayName)) {
            continue;
        }
        UpsertMacosApplicationCatalogEntry(processName, displayName, root.source, entries, indexByProcess);
    }
}

} // namespace mousefx::platform::macos::application_catalog_scan_detail

#include "pch.h"

#include "Platform/macos/System/MacosApplicationCatalogScanWorkflow.Internal.h"

#if defined(__APPLE__)
#import <Foundation/Foundation.h>
#endif

#include <string>
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

bool IsApplicationDirectory(NSURL* candidate) {
    if (candidate == nil) {
        return false;
    }
    NSNumber* isDirectory = nil;
    [candidate getResourceValue:&isDirectory forKey:NSURLIsDirectoryKey error:nil];
    if (isDirectory == nil || !isDirectory.boolValue) {
        return false;
    }

    NSNumber* isApplication = nil;
    [candidate getResourceValue:&isApplication forKey:NSURLIsApplicationKey error:nil];
    return isApplication != nil && isApplication.boolValue;
}

} // namespace

void CollectMacosApplicationBundlePaths(
    const MacosApplicationCatalogScanRoot& root,
    std::vector<std::string>* bundlePaths) {
    if (!bundlePaths) {
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
        if (!IsApplicationDirectory(candidate)) {
            continue;
        }
        [enumerator skipDescendants];
        const std::string bundlePath = NsStringToUtf8(candidate.path);
        if (!bundlePath.empty()) {
            bundlePaths->push_back(bundlePath);
        }
    }
}

} // namespace mousefx::platform::macos::application_catalog_scan_detail

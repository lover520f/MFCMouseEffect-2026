#include "pch.h"

#include "Platform/macos/System/MacosOpenPanelFolderPicker.Internal.h"

#include "MouseFx/Utils/StringUtils.h"

namespace mousefx::platform::macos::open_panel_picker_detail {

NSString* WideToNSString(const std::wstring& text) {
    if (text.empty()) {
        return nil;
    }
    const std::string utf8 = Utf16ToUtf8(text.c_str());
    if (utf8.empty()) {
        return nil;
    }
    return [NSString stringWithUTF8String:utf8.c_str()];
}

std::wstring NSStringToWide(NSString* text) {
    if (text == nil || text.length == 0) {
        return {};
    }
    const char* raw = [text UTF8String];
    if (raw == nullptr || raw[0] == '\0') {
        return {};
    }
    return Utf8ToWString(raw);
}

NSURL* ResolveDirectoryUrlFromInitialPath(const std::wstring& initialPath) {
    NSString* initialPathText = WideToNSString(initialPath);
    if (initialPathText == nil || initialPathText.length == 0) {
        return nil;
    }
    BOOL isDirectory = NO;
    if (![[NSFileManager defaultManager] fileExistsAtPath:initialPathText isDirectory:&isDirectory] || !isDirectory) {
        return nil;
    }
    return [NSURL fileURLWithPath:initialPathText isDirectory:YES];
}

} // namespace mousefx::platform::macos::open_panel_picker_detail

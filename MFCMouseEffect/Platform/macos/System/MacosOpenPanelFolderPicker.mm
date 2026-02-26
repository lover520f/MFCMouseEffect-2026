#include "pch.h"

#include "Platform/macos/System/MacosOpenPanelFolderPicker.Internal.h"
#include "Platform/macos/System/MacosOpenPanelFolderPicker.h"

#if defined(__APPLE__)
#import <AppKit/AppKit.h>
#import <Foundation/Foundation.h>
#import <dispatch/dispatch.h>
#endif

#include <string>

namespace mousefx::platform::macos {
namespace {

class ScopedModalDialogActivation final {
public:
    ScopedModalDialogActivation() {
        [NSApplication sharedApplication];
        if (NSApp != nil) {
            [NSApp activateIgnoringOtherApps:YES];
        }
    }

    ~ScopedModalDialogActivation() = default;
};

NativeFolderPickResult ExecuteOpenPanelOnMainThread(
    const std::wstring& title,
    const std::wstring& initialPath) {
    NativeFolderPickResult out{};
    ScopedModalDialogActivation modalActivation;

    NSOpenPanel* panel = [NSOpenPanel openPanel];
    if (panel == nil) {
        out.error = "failed to create folder dialog";
        return out;
    }

    panel.canChooseFiles = NO;
    panel.canChooseDirectories = YES;
    panel.allowsMultipleSelection = NO;
    panel.resolvesAliases = YES;

    NSString* titleText = open_panel_picker_detail::WideToNSString(title);
    if (titleText != nil && titleText.length > 0) {
        panel.message = titleText;
    }

    NSURL* initialDirectoryUrl = open_panel_picker_detail::ResolveDirectoryUrlFromInitialPath(initialPath);
    if (initialDirectoryUrl != nil) {
        panel.directoryURL = initialDirectoryUrl;
    }

    const NSInteger resultCode = [panel runModal];
    if (resultCode == NSModalResponseOK) {
        NSURL* selectedUrl = panel.URL;
        if (selectedUrl == nil) {
            out.error = "selected folder path missing";
            return out;
        }
        out.ok = true;
        out.folderPath = open_panel_picker_detail::NSStringToWide(selectedUrl.path);
        if (out.folderPath.empty()) {
            out.ok = false;
            out.error = "selected folder path missing";
        }
        return out;
    }

    if (resultCode == NSModalResponseCancel) {
        out.cancelled = true;
        out.error = "cancelled";
        return out;
    }

    out.error = "failed to show folder dialog";
    return out;
}

} // namespace

NativeFolderPickResult PickFolderViaOpenPanel(
    const std::wstring& title,
    const std::wstring& initialPath) {
#if defined(__APPLE__)
    if ([NSThread isMainThread]) {
        return ExecuteOpenPanelOnMainThread(title, initialPath);
    }

    __block NativeFolderPickResult out{};
    dispatch_sync(dispatch_get_main_queue(), ^{
        out = ExecuteOpenPanelOnMainThread(title, initialPath);
    });
    return out;
#else
    (void)title;
    (void)initialPath;
    NativeFolderPickResult out{};
    out.error = "native_folder_picker_not_supported";
    return out;
#endif
}

} // namespace mousefx::platform::macos

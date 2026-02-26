#pragma once

#include <string>

#if defined(__APPLE__)
#import <Foundation/Foundation.h>
#endif

namespace mousefx::platform::macos::open_panel_picker_detail {

NSString* WideToNSString(const std::wstring& text);
std::wstring NSStringToWide(NSString* text);
NSURL* ResolveDirectoryUrlFromInitialPath(const std::wstring& initialPath);

} // namespace mousefx::platform::macos::open_panel_picker_detail

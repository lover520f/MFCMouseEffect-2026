#pragma once

#include "Platform/PlatformNativeFolderPicker.h"

#include <string>

namespace mousefx::platform::windows {

class Win32NativeFolderPicker final {
public:
    static NativeFolderPickResult PickFolder(
        const std::wstring& title,
        const std::wstring& initialPath = L"");
};

} // namespace mousefx::platform::windows

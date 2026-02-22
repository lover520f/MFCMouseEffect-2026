#include "pch.h"

#include "Platform/PlatformNativeFolderPicker.h"

#if defined(_WIN32)
#include "Platform/windows/System/Win32NativeFolderPicker.h"
#endif

namespace mousefx::platform {

NativeFolderPickResult PickFolder(const std::wstring& title, const std::wstring& initialPath) {
#if defined(_WIN32)
    return windows::Win32NativeFolderPicker::PickFolder(title, initialPath);
#else
    NativeFolderPickResult result{};
    result.ok = false;
    result.cancelled = false;
    result.error = "native_folder_picker_not_supported";
    return result;
#endif
}

} // namespace mousefx::platform

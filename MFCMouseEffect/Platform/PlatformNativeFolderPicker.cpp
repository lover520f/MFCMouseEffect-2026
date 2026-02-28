#include "pch.h"

#include "Platform/PlatformNativeFolderPicker.h"

#if defined(_WIN32)
#include "Platform/windows/System/Win32NativeFolderPicker.h"
#elif defined(__APPLE__)
#include "Platform/macos/System/MacosNativeFolderPickerSwiftBridge.h"
#endif

#include "MouseFx/Utils/StringUtils.h"

#include <array>
#include <string>

namespace mousefx::platform {

#if defined(__APPLE__)
namespace {

NativeFolderPickResult PickFolderViaSwiftBridge(
    const std::wstring& title,
    const std::wstring& initialPath) {
    constexpr int32_t kPathBufferCapacity = 32768;
    constexpr int32_t kErrorBufferCapacity = 2048;

    std::array<char, kPathBufferCapacity> selectedPathBuffer{};
    std::array<char, kErrorBufferCapacity> errorBuffer{};
    const std::string titleUtf8 = Utf16ToUtf8(title.c_str());
    const std::string initialPathUtf8 = Utf16ToUtf8(initialPath.c_str());

    const int32_t outcomeCode = mfx_macos_pick_folder_v1(
        titleUtf8.c_str(),
        initialPathUtf8.c_str(),
        selectedPathBuffer.data(),
        kPathBufferCapacity,
        errorBuffer.data(),
        kErrorBufferCapacity);

    NativeFolderPickResult out{};
    if (outcomeCode > 0) {
        out.ok = true;
        out.folderPath = Utf8ToWString(selectedPathBuffer.data());
        if (out.folderPath.empty()) {
            out.ok = false;
            out.error = "selected folder path missing";
        }
        return out;
    }

    out.cancelled = (outcomeCode == 0);
    out.error = errorBuffer.data();
    if (out.error.empty()) {
        out.error = out.cancelled ? "cancelled" : "folder_picker_failed";
    }
    return out;
}

} // namespace
#endif

NativeFolderPickResult PickFolder(const std::wstring& title, const std::wstring& initialPath) {
#if defined(_WIN32)
    return windows::Win32NativeFolderPicker::PickFolder(title, initialPath);
#elif defined(__APPLE__)
    return PickFolderViaSwiftBridge(title, initialPath);
#else
    NativeFolderPickResult result{};
    result.ok = false;
    result.cancelled = false;
    result.error = "native_folder_picker_not_supported";
    return result;
#endif
}

bool IsNativeFolderPickerSupported() {
#if defined(_WIN32) || defined(__APPLE__)
    return true;
#else
    return false;
#endif
}

} // namespace mousefx::platform

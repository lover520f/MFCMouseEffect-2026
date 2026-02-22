#pragma once

#include <string>

namespace mousefx::platform {

struct NativeFolderPickResult final {
    bool ok = false;
    bool cancelled = false;
    std::wstring folderPath{};
    std::string error{};
};

NativeFolderPickResult PickFolder(
    const std::wstring& title,
    const std::wstring& initialPath = L"");

} // namespace mousefx::platform

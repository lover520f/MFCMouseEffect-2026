#pragma once

#include <string>

namespace mousefx {

struct NativeFolderPickResult final {
    bool ok = false;
    bool cancelled = false;
    std::wstring folderPath{};
    std::string error{};
};

class NativeFolderPicker final {
public:
    static NativeFolderPickResult PickFolder(
        const std::wstring& title,
        const std::wstring& initialPath = L"");
};

} // namespace mousefx


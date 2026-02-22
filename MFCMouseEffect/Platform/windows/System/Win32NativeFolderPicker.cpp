#include "pch.h"

#include "Platform/windows/System/Win32NativeFolderPicker.h"

#include <ShObjIdl.h>
#include <combaseapi.h>
#include <filesystem>
#include <future>
#include <sstream>
#include <string>
#include <thread>

namespace mousefx::platform::windows {
namespace {

std::string HrToHexString(HRESULT hr) {
    std::ostringstream out;
    out << "0x" << std::hex << std::uppercase << static_cast<unsigned long>(hr);
    return out.str();
}

std::wstring TrimWide(std::wstring value) {
    const auto isSpace = [](wchar_t ch) {
        return ch == L' ' || ch == L'\t' || ch == L'\r' || ch == L'\n';
    };
    size_t begin = 0;
    while (begin < value.size() && isSpace(value[begin])) {
        ++begin;
    }
    if (begin >= value.size()) {
        return {};
    }
    size_t end = value.size();
    while (end > begin && isSpace(value[end - 1])) {
        --end;
    }
    return value.substr(begin, end - begin);
}

NativeFolderPickResult PickFolderOnCurrentThread(const std::wstring& title, const std::wstring& initialPath) {
    NativeFolderPickResult out{};
    const HRESULT initHr = ::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    const bool uninit = (initHr == S_OK || initHr == S_FALSE);
    if (FAILED(initHr)) {
        out.error = "failed to initialize COM: " + HrToHexString(initHr);
        return out;
    }

    IFileOpenDialog* dialog = nullptr;
    HRESULT hr = ::CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog));
    if (FAILED(hr) || !dialog) {
        if (uninit) {
            ::CoUninitialize();
        }
        out.error = "failed to create folder dialog: " + HrToHexString(hr);
        return out;
    }

    DWORD options = 0;
    hr = dialog->GetOptions(&options);
    if (SUCCEEDED(hr)) {
        options |= (FOS_PICKFOLDERS | FOS_PATHMUSTEXIST | FOS_FORCEFILESYSTEM | FOS_NOCHANGEDIR);
        hr = dialog->SetOptions(options);
    }
    if (FAILED(hr)) {
        dialog->Release();
        if (uninit) {
            ::CoUninitialize();
        }
        out.error = "failed to set folder dialog options: " + HrToHexString(hr);
        return out;
    }

    const std::wstring titleText = TrimWide(title);
    if (!titleText.empty()) {
        dialog->SetTitle(titleText.c_str());
    }

    const std::wstring initPath = TrimWide(initialPath);
    if (!initPath.empty()) {
        std::error_code ec;
        const std::filesystem::path folderPath(initPath);
        if (std::filesystem::exists(folderPath, ec) && !ec && std::filesystem::is_directory(folderPath, ec) && !ec) {
            IShellItem* folderItem = nullptr;
            hr = SHCreateItemFromParsingName(folderPath.c_str(), nullptr, IID_PPV_ARGS(&folderItem));
            if (SUCCEEDED(hr) && folderItem) {
                dialog->SetDefaultFolder(folderItem);
                dialog->SetFolder(folderItem);
                folderItem->Release();
            }
        }
    }

    hr = dialog->Show(::GetForegroundWindow());
    if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
        dialog->Release();
        if (uninit) {
            ::CoUninitialize();
        }
        out.cancelled = true;
        out.error = "cancelled";
        return out;
    }
    if (FAILED(hr)) {
        dialog->Release();
        if (uninit) {
            ::CoUninitialize();
        }
        out.error = "failed to show folder dialog: " + HrToHexString(hr);
        return out;
    }

    IShellItem* shellItem = nullptr;
    hr = dialog->GetResult(&shellItem);
    if (FAILED(hr) || !shellItem) {
        dialog->Release();
        if (uninit) {
            ::CoUninitialize();
        }
        out.error = "failed to get selected folder: " + HrToHexString(hr);
        return out;
    }

    PWSTR rawPath = nullptr;
    hr = shellItem->GetDisplayName(SIGDN_FILESYSPATH, &rawPath);
    if (FAILED(hr) || !rawPath) {
        shellItem->Release();
        dialog->Release();
        if (uninit) {
            ::CoUninitialize();
        }
        out.error = "failed to read selected folder path: " + HrToHexString(hr);
        return out;
    }

    out.ok = true;
    out.folderPath = rawPath;
    ::CoTaskMemFree(rawPath);
    shellItem->Release();
    dialog->Release();
    if (uninit) {
        ::CoUninitialize();
    }
    return out;
}

} // namespace

NativeFolderPickResult Win32NativeFolderPicker::PickFolder(
    const std::wstring& title,
    const std::wstring& initialPath) {
    std::promise<NativeFolderPickResult> promise;
    std::future<NativeFolderPickResult> future = promise.get_future();
    std::thread worker([title, initialPath, p = std::move(promise)]() mutable {
        p.set_value(PickFolderOnCurrentThread(title, initialPath));
    });
    worker.join();
    return future.get();
}

} // namespace mousefx::platform::windows

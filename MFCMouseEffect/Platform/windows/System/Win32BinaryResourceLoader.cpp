#include "pch.h"

#include "Platform/windows/System/Win32BinaryResourceLoader.h"

#include <windows.h>

namespace mousefx::platform::windows {

bool Win32BinaryResourceLoader::TryLoadEmbeddedBinaryResource(
    int resourceId,
    std::vector<uint8_t>* outBytes) {
    if (outBytes == nullptr) {
        return false;
    }

    HINSTANCE module = ::GetModuleHandleW(nullptr);
    HRSRC resource = ::FindResourceW(module, MAKEINTRESOURCEW(resourceId), RT_RCDATA);
    if (resource == nullptr) {
        return false;
    }

    HGLOBAL handle = ::LoadResource(module, resource);
    if (handle == nullptr) {
        return false;
    }

    const DWORD size = ::SizeofResource(module, resource);
    if (size == 0) {
        return false;
    }

    const void* raw = ::LockResource(handle);
    if (raw == nullptr) {
        return false;
    }

    outBytes->assign(
        static_cast<const uint8_t*>(raw),
        static_cast<const uint8_t*>(raw) + size);
    return true;
}

} // namespace mousefx::platform::windows


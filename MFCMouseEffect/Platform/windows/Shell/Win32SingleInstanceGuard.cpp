#include "pch.h"
#include "framework.h"

#include "Platform/windows/Shell/Win32SingleInstanceGuard.h"

namespace mousefx {

Win32SingleInstanceGuard::~Win32SingleInstanceGuard() {
    Release();
}

bool Win32SingleInstanceGuard::Acquire(const std::wstring& key) {
    if (key.empty()) {
        return false;
    }
    if (mutex_) {
        return true;
    }

    mutex_ = CreateMutexW(nullptr, TRUE, key.c_str());
    if (!mutex_) {
        return false;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(mutex_);
        mutex_ = nullptr;
        return false;
    }
    return true;
}

void Win32SingleInstanceGuard::Release() {
    if (!mutex_) {
        return;
    }
    ReleaseMutex(mutex_);
    CloseHandle(mutex_);
    mutex_ = nullptr;
}

} // namespace mousefx

#pragma once

#include <windows.h>

#include "MouseFx/Core/Shell/ISingleInstanceGuard.h"

namespace mousefx {

class Win32SingleInstanceGuard final : public ISingleInstanceGuard {
public:
    Win32SingleInstanceGuard() = default;
    ~Win32SingleInstanceGuard() override;

    Win32SingleInstanceGuard(const Win32SingleInstanceGuard&) = delete;
    Win32SingleInstanceGuard& operator=(const Win32SingleInstanceGuard&) = delete;

    bool Acquire(const std::wstring& key) override;
    void Release() override;

private:
    HANDLE mutex_ = nullptr;
};

} // namespace mousefx

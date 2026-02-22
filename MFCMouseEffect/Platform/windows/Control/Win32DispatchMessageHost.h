#pragma once

#include <windows.h>

#include "MouseFx/Core/Control/IDispatchMessageHost.h"

namespace mousefx {

class IDispatchMessageHandler;

class Win32DispatchMessageHost final : public IDispatchMessageHost {
public:
    Win32DispatchMessageHost() = default;
    ~Win32DispatchMessageHost() override;

    Win32DispatchMessageHost(const Win32DispatchMessageHost&) = delete;
    Win32DispatchMessageHost& operator=(const Win32DispatchMessageHost&) = delete;

    bool Create(IDispatchMessageHandler* handler) override;
    void Destroy() override;
    bool IsCreated() const override;
    bool IsOwnerThread() const override;
    uintptr_t NativeHandle() const override;
    uint32_t LastError() const override { return lastError_; }
    intptr_t SendSync(uint32_t msg, uintptr_t wParam, intptr_t lParam) override;
    bool SetTimer(uintptr_t timerId, uint32_t intervalMs) override;
    void KillTimer(uintptr_t timerId) override;

private:
    static const wchar_t* ClassName();
    static bool EnsureClassRegistered();
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND hwnd_ = nullptr;
    DWORD ownerThreadId_ = 0;
    uint32_t lastError_ = ERROR_SUCCESS;
    IDispatchMessageHandler* handler_ = nullptr;
};

} // namespace mousefx

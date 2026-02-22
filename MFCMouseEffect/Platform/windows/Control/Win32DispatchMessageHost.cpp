#include "pch.h"

#include "Platform/windows/Control/Win32DispatchMessageHost.h"

#include "MouseFx/Core/Control/IDispatchMessageHandler.h"

namespace mousefx {

Win32DispatchMessageHost::~Win32DispatchMessageHost() {
    Destroy();
}

const wchar_t* Win32DispatchMessageHost::ClassName() {
    return L"MouseFxDispatchWindow";
}

bool Win32DispatchMessageHost::EnsureClassRegistered() {
    static bool registered = false;
    static bool ok = false;
    if (registered) {
        return ok;
    }
    registered = true;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &Win32DispatchMessageHost::WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = ClassName();
    ok = (RegisterClassExW(&wc) != 0) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    return ok;
}

bool Win32DispatchMessageHost::Create(IDispatchMessageHandler* handler) {
    if (hwnd_ != nullptr) {
        return true;
    }
    if (!handler) {
        lastError_ = ERROR_INVALID_PARAMETER;
        return false;
    }
    if (!EnsureClassRegistered()) {
        lastError_ = GetLastError();
        return false;
    }

    handler_ = handler;
    hwnd_ = CreateWindowExW(
        0,
        ClassName(),
        L"",
        0,
        0,
        0,
        0,
        0,
        HWND_MESSAGE,
        nullptr,
        GetModuleHandleW(nullptr),
        this);
    if (!hwnd_) {
        lastError_ = GetLastError();
        handler_ = nullptr;
        return false;
    }

    ownerThreadId_ = GetCurrentThreadId();
    lastError_ = ERROR_SUCCESS;
    return true;
}

void Win32DispatchMessageHost::Destroy() {
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    ownerThreadId_ = 0;
    handler_ = nullptr;
}

bool Win32DispatchMessageHost::IsCreated() const {
    return hwnd_ != nullptr && IsWindow(hwnd_);
}

bool Win32DispatchMessageHost::IsOwnerThread() const {
    return IsCreated() && ownerThreadId_ == GetCurrentThreadId();
}

uintptr_t Win32DispatchMessageHost::NativeHandle() const {
    return reinterpret_cast<uintptr_t>(hwnd_);
}

intptr_t Win32DispatchMessageHost::SendSync(uint32_t msg, uintptr_t wParam, intptr_t lParam) {
    if (!IsCreated()) {
        return 0;
    }
    return static_cast<intptr_t>(SendMessageW(
        hwnd_,
        static_cast<UINT>(msg),
        static_cast<WPARAM>(wParam),
        static_cast<LPARAM>(lParam)));
}

bool Win32DispatchMessageHost::SetTimer(uintptr_t timerId, uint32_t intervalMs) {
    if (!IsCreated()) {
        return false;
    }
    if (::SetTimer(hwnd_, static_cast<UINT_PTR>(timerId), static_cast<UINT>(intervalMs), nullptr) == 0) {
        lastError_ = GetLastError();
        return false;
    }
    return true;
}

void Win32DispatchMessageHost::KillTimer(uintptr_t timerId) {
    if (!IsCreated()) {
        return;
    }
    ::KillTimer(hwnd_, static_cast<UINT_PTR>(timerId));
}

LRESULT CALLBACK Win32DispatchMessageHost::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Win32DispatchMessageHost* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<Win32DispatchMessageHost*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<Win32DispatchMessageHost*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (!self) {
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return self->OnWndProc(hwnd, msg, wParam, lParam);
}

LRESULT Win32DispatchMessageHost::OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCDESTROY) {
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, 0);
    }
    if (!handler_) {
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return static_cast<LRESULT>(handler_->OnDispatchMessage(
        reinterpret_cast<uintptr_t>(hwnd),
        static_cast<uint32_t>(msg),
        static_cast<uintptr_t>(wParam),
        static_cast<intptr_t>(lParam)));
}

} // namespace mousefx

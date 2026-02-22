#pragma once

#include <windows.h>
#include <atomic>
#include <cstdint>

#include "MouseFx/Core/System/IGlobalMouseHook.h"

namespace mousefx {

class Win32GlobalMouseHook final : public IGlobalMouseHook {
public:
    Win32GlobalMouseHook() = default;
    ~Win32GlobalMouseHook() override { Stop(); }

    Win32GlobalMouseHook(const Win32GlobalMouseHook&) = delete;
    Win32GlobalMouseHook& operator=(const Win32GlobalMouseHook&) = delete;

    bool Start(uintptr_t dispatchHandle) override;
    void Stop() override;
    uint32_t LastError() const override { return lastError_; }
    bool ConsumeLatestMove(ScreenPoint& outPt) override;
    void SetKeyboardCaptureExclusive(bool enabled) override;

private:
    static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);

    static Win32GlobalMouseHook* instance_;
    HHOOK hook_ = nullptr;
    HHOOK keyboardHook_ = nullptr;
    HWND dispatchHwnd_ = nullptr;
    uint32_t lastError_ = ERROR_SUCCESS;
    std::atomic<bool> movePending_{false};
    std::atomic<LONG> latestMoveX_{0};
    std::atomic<LONG> latestMoveY_{0};
    std::atomic<bool> keyboardCaptureExclusive_{false};
    std::atomic<uint32_t> keyboardModifierMask_{0};
};

} // namespace mousefx

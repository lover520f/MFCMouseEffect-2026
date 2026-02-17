#pragma once

#include <windows.h>
#include <cstdint>
#include <atomic>
#include <string>

namespace mousefx {

enum class MouseButton : uint8_t {
    Left = 1,
    Right = 2,
    Middle = 3,
};

struct ClickEvent {
    POINT pt{};
    MouseButton button{MouseButton::Left};
};

struct KeyEvent {
    POINT pt{};
    UINT vkCode{0};
    bool systemKey{false};
    bool ctrl{false};
    bool shift{false};
    bool alt{false};
    bool win{false};
    std::wstring text{};
};

// WH_MOUSE_LL hook that posts ClickEvent objects to a dispatch window (message-only HWND).
class GlobalMouseHook final {
public:
    GlobalMouseHook() = default;
    ~GlobalMouseHook() { Stop(); }

    GlobalMouseHook(const GlobalMouseHook&) = delete;
    GlobalMouseHook& operator=(const GlobalMouseHook&) = delete;

    bool Start(HWND dispatchHwnd);
    void Stop();
    DWORD LastError() const { return lastError_; }
    bool ConsumeLatestMove(POINT& outPt);
    void SetKeyboardCaptureExclusive(bool enabled);

private:
    static LRESULT CALLBACK HookProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK KeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam);

    static GlobalMouseHook* instance_;
    HHOOK hook_ = nullptr;
    HHOOK keyboardHook_ = nullptr;
    HWND dispatchHwnd_ = nullptr;
    DWORD lastError_ = ERROR_SUCCESS;
    std::atomic<bool> movePending_{false};
    std::atomic<LONG> latestMoveX_{0};
    std::atomic<LONG> latestMoveY_{0};
    std::atomic<bool> keyboardCaptureExclusive_{false};
    std::atomic<uint32_t> keyboardModifierMask_{0};
};

} // namespace mousefx

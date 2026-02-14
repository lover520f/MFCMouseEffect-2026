#pragma once

#include <windows.h>
#include <cstdint>
#include <string>

#include "EffectConfig.h"
#include "GlobalMouseHook.h"
#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Renderers/Indicator/IndicatorRenderer.h"

namespace mousefx {

// Displays a small animated overlay near the cursor showing
// mouse clicks, scroll wheel events, and keyboard key presses.
// Window lifecycle and timer management live here; all GDI+
// drawing is delegated to IndicatorRenderer.
class InputIndicatorOverlay final {
public:
    InputIndicatorOverlay() = default;
    ~InputIndicatorOverlay() { Shutdown(); }

    InputIndicatorOverlay(const InputIndicatorOverlay&) = delete;
    InputIndicatorOverlay& operator=(const InputIndicatorOverlay&) = delete;

    bool Initialize();
    void Shutdown();
    void Hide();
    void UpdateConfig(const InputIndicatorConfig& cfg);

    void OnClick(const ClickEvent& ev);
    void OnScroll(const ScrollEvent& ev);
    void OnKey(const KeyEvent& ev);

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool EnsureWindow();
    void Trigger(IndicatorEventKind kind, POINT anchorPt, std::wstring label = {});
    void Render();
    void UpdatePlacement(POINT anchorPt);

    static int ClampInt(int v, int lo, int hi);
    static bool IsRelativeMode(const std::string& mode);

private:
    InputIndicatorConfig config_{};
    IndicatorRenderer renderer_{};
    HWND hwnd_ = nullptr;
    bool initialized_ = false;
    bool windowClassRegistered_ = false;
    bool active_ = false;
    IndicatorEventKind eventKind_ = IndicatorEventKind::None;
    uint64_t eventStartMs_ = 0;
    POINT anchorPt_{};

    MouseButton lastClickButton_ = MouseButton::Left;
    uint64_t lastClickTickMs_ = 0;
    int clickStreak_ = 0;

    int scrollStreak_ = 0;
    uint64_t lastScrollTickMs_ = 0;
    int lastScrollDelta_ = 0;

    std::wstring eventLabel_{};
};

} // namespace mousefx

#pragma once

#include <windows.h>
#include <cstdint>
#include <map>
#include <string>

#include "Config/EffectConfig.h"
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
    HWND CreateCloneWindow();
    void Trigger(IndicatorEventKind kind, POINT anchorPt, std::wstring label = {}, bool isKeyboard = false);
    void TriggerOnEnabledMonitors(IndicatorEventKind kind, POINT anchorPt, std::wstring label, bool isKeyboard);
    void Render();
    void RenderToWindow(HWND hwnd);
    void UpdatePlacement(POINT anchorPt, bool isKeyboard = false);
    void UpdateClonePlacement(HWND hwnd, const std::string& monitorId, bool isKeyboard);
    void SyncCloneWindows(bool isKeyboard);
    void DestroyClones();

    static int ClampInt(int v, int lo, int hi);
    static bool IsRelativeMode(const std::string& mode);
    bool IsCustomMode(bool isKeyboard) const;

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

    // Key Streak
    uint32_t lastKeyVkCode_ = 0;
    uint32_t lastKeyModifiers_ = 0; // Packed modifiers
    int keyStreak_ = 0;
    uint64_t lastKeyTickMs_ = 0;

    std::wstring eventLabel_{};

    // Multi-monitor clone windows (monitorId -> HWND)
    std::map<std::string, HWND> cloneWindows_;
    bool customModeActive_ = false;
};

} // namespace mousefx

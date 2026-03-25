#pragma once

#include <windows.h>
#include <cstdint>
#include <map>
#include <string>

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/Overlay/InputIndicatorLabelFormatter.h"
#include "MouseFx/Core/Overlay/IInputIndicatorOverlay.h"
#include "MouseFx/Renderers/Indicator/CursorDecorationRenderer.h"
#include "MouseFx/Renderers/Indicator/IndicatorRenderer.h"

namespace mousefx {

// Displays a small animated overlay near the cursor showing
// mouse clicks, scroll wheel events, and keyboard key presses.
// Window lifecycle and timer management live here; all GDI+
// drawing is delegated to IndicatorRenderer.
class Win32InputIndicatorOverlay final : public IInputIndicatorOverlay {
public:
    Win32InputIndicatorOverlay() = default;
    ~Win32InputIndicatorOverlay() override { Shutdown(); }

    Win32InputIndicatorOverlay(const Win32InputIndicatorOverlay&) = delete;
    Win32InputIndicatorOverlay& operator=(const Win32InputIndicatorOverlay&) = delete;

    bool Initialize() override;
    void Shutdown() override;
    void Hide() override;
    void UpdateConfig(const InputIndicatorConfig& cfg) override;

    void OnClick(const ClickEvent& ev) override;
    void OnScroll(const ScrollEvent& ev) override;
    void OnKey(const KeyEvent& ev) override;
    void OnMove(const ScreenPoint& pt) override;
    void SetCursorDecorationNativeSuppressed(bool suppressed) override;
    void SetCursorDecorationBlockedByAppBlacklist(bool blocked) override;

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT OnWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool EnsureWindow();
    bool EnsureDecorationWindow();
    HWND CreateCloneWindow();
    void Trigger(IndicatorEventKind kind, POINT anchorPt, std::wstring label = {}, bool isKeyboard = false);
    void TriggerOnEnabledMonitors(IndicatorEventKind kind, POINT anchorPt, std::wstring label, bool isKeyboard);
    void Render();
    void RenderToWindow(HWND hwnd);
    void RenderDecoration();
    void RenderDecorationToWindow(HWND hwnd);
    void UpdateRenderSize(IndicatorEventKind kind, const std::wstring& label);
    void UpdatePlacement(POINT anchorPt, bool isKeyboard = false);
    void UpdateDecorationPlacement(POINT anchorPt);
    void UpdateClonePlacement(HWND hwnd, const std::string& monitorId, bool isKeyboard);
    void SyncCloneWindows(bool isKeyboard);
    void DestroyClones();
    void HideDecorationWindow();
    void UpdateFrameTimerForPoint(POINT anchorPt, bool force);

    static int ClampInt(int v, int lo, int hi);
    static bool IsRelativeMode(const std::string& mode);
    bool IsCustomMode(bool isKeyboard) const;

private:
    InputIndicatorConfig config_{};
    IndicatorRenderer renderer_{};
    CursorDecorationRenderer cursorDecorationRenderer_{};
    HWND hwnd_ = nullptr;
    HWND decorationHwnd_ = nullptr;
    bool initialized_ = false;
    bool windowClassRegistered_ = false;
    bool active_ = false;
    bool hasCursorPoint_ = false;
    bool cursorDecorationNativeSuppressed_ = false;
    bool cursorDecorationBlockedByAppBlacklist_ = false;
    IndicatorEventKind eventKind_ = IndicatorEventKind::None;
    uint64_t eventStartMs_ = 0;
    POINT anchorPt_{};
    POINT cursorPt_{};

    InputIndicatorMouseStreakState mouseStreakState_{};

    InputIndicatorKeyStreakState keyStreakState_{};

    std::wstring eventLabel_{};
    int renderWidthPx_ = 72;
    int renderHeightPx_ = 72;
    UINT frameTimerIntervalMs_ = 16;
    bool frameTimerArmed_ = false;

    // Multi-monitor clone windows (monitorId -> HWND)
    std::map<std::string, HWND> cloneWindows_;
    bool customModeActive_ = false;
};

} // namespace mousefx

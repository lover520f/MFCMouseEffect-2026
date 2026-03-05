#include "pch.h"

#include "Platform/windows/Overlay/Win32OverlayTimerSupport.h"

#include <algorithm>
#include <atomic>
#include <cmath>

namespace mousefx::win32_overlay_timer_support {
namespace {

std::atomic<int> g_overlayTargetFps{0};

int SanitizeTargetFps(int targetFps) {
    if (targetFps <= 0) {
        return 0;
    }
    return std::clamp(targetFps, 1, 360);
}

int ResolveMonitorRefreshRateHz(HMONITOR monitor) {
    constexpr int kFallbackRefreshHz = 60;
    if (!monitor) {
        return kFallbackRefreshHz;
    }

    MONITORINFOEXW monitorInfo{};
    monitorInfo.cbSize = sizeof(monitorInfo);
    if (!GetMonitorInfoW(monitor, &monitorInfo)) {
        return kFallbackRefreshHz;
    }

    int refreshHz = 0;

    DEVMODEW displayMode{};
    displayMode.dmSize = sizeof(displayMode);
    if (EnumDisplaySettingsExW(
            monitorInfo.szDevice,
            ENUM_CURRENT_SETTINGS,
            &displayMode,
            0)) {
        const int modeRefresh = static_cast<int>(displayMode.dmDisplayFrequency);
        if (modeRefresh > 1 && modeRefresh <= 1000) {
            refreshHz = modeRefresh;
        }
    }

    if (refreshHz <= 1 || refreshHz > 1000) {
        HDC monitorDc = CreateDCW(L"DISPLAY", monitorInfo.szDevice, nullptr, nullptr);
        if (monitorDc) {
            const int dcRefresh = GetDeviceCaps(monitorDc, VREFRESH);
            DeleteDC(monitorDc);
            if (dcRefresh > 1 && dcRefresh <= 1000) {
                refreshHz = dcRefresh;
            }
        }
    }

    if (refreshHz <= 1 || refreshHz > 1000) {
        refreshHz = kFallbackRefreshHz;
    }
    return refreshHz;
}

int ResolveEffectiveFps(int monitorRefreshHz) {
    const int safeRefresh = std::max(1, monitorRefreshHz);
    const int configuredTarget = g_overlayTargetFps.load(std::memory_order_relaxed);
    if (configuredTarget <= 0) {
        return safeRefresh;
    }
    return std::min(std::max(1, configuredTarget), safeRefresh);
}

int ResolveIntervalMsFromMonitor(HMONITOR monitor) {
    const int effectiveFps = ResolveEffectiveFps(ResolveMonitorRefreshRateHz(monitor));
    const int intervalMs = static_cast<int>(std::lround(1000.0 / static_cast<double>(std::max(1, effectiveFps))));
    return std::clamp(intervalMs, 4, 1000);
}

} // namespace

void SetOverlayTargetFps(int targetFps) {
    g_overlayTargetFps.store(SanitizeTargetFps(targetFps), std::memory_order_relaxed);
}

int ResolveTimerIntervalMsForScreenPoint(int x, int y) {
    POINT pt{};
    pt.x = x;
    pt.y = y;
    const HMONITOR monitor = MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
    return ResolveIntervalMsFromMonitor(monitor);
}

int ResolveTimerIntervalMsForCursor() {
    POINT cursor{};
    if (GetCursorPos(&cursor)) {
        return ResolveTimerIntervalMsForScreenPoint(cursor.x, cursor.y);
    }
    return ResolveTimerIntervalMsForVirtualScreen();
}

int ResolveTimerIntervalMsForWindow(HWND hwnd) {
    if (!hwnd) {
        return ResolveTimerIntervalMsForVirtualScreen();
    }
    const HMONITOR monitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    return ResolveIntervalMsFromMonitor(monitor);
}

int ResolveTimerIntervalMsForVirtualScreen() {
    const int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    const int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    const int w = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    const int h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    const int centerX = x + (w / 2);
    const int centerY = y + (h / 2);
    return ResolveTimerIntervalMsForScreenPoint(centerX, centerY);
}

} // namespace mousefx::win32_overlay_timer_support


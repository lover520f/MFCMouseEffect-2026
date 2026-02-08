#include "pch.h"

#include "OverlayCoordSpace.h"

#include <atomic>

namespace mousefx {
namespace {

std::atomic<bool> g_overlayOriginOverrideEnabled{false};
std::atomic<int> g_overlayOriginX{0};
std::atomic<int> g_overlayOriginY{0};
std::atomic<UINT_PTR> g_overlayWindowHandle{0};

POINT GetVirtualScreenOrigin() {
    POINT pt{};
    pt.x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    pt.y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    return pt;
}

} // namespace

void SetOverlayWindowHandle(HWND hwnd) {
    g_overlayWindowHandle.store((UINT_PTR)hwnd, std::memory_order_release);
}

void ClearOverlayWindowHandle() {
    g_overlayWindowHandle.store(0, std::memory_order_release);
}

void SetOverlayOriginOverride(int x, int y) {
    g_overlayOriginX.store(x, std::memory_order_relaxed);
    g_overlayOriginY.store(y, std::memory_order_relaxed);
    g_overlayOriginOverrideEnabled.store(true, std::memory_order_release);
}

void ClearOverlayOriginOverride() {
    g_overlayOriginOverrideEnabled.store(false, std::memory_order_release);
}

POINT GetOverlayOrigin() {
    if (g_overlayOriginOverrideEnabled.load(std::memory_order_acquire)) {
        POINT pt{};
        pt.x = g_overlayOriginX.load(std::memory_order_relaxed);
        pt.y = g_overlayOriginY.load(std::memory_order_relaxed);
        return pt;
    }

    return GetVirtualScreenOrigin();
}

POINT ScreenToOverlayPoint(const POINT& screenPt) {
    POINT pt = screenPt;
    const POINT origin = GetOverlayOrigin();
    pt.x -= origin.x;
    pt.y -= origin.y;
    return pt;
}

} // namespace mousefx

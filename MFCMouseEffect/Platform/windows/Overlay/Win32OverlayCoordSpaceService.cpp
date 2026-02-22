#include "pch.h"

#include "Platform/windows/Overlay/Win32OverlayCoordSpaceService.h"

#include <windows.h>

namespace mousefx {

void Win32OverlayCoordSpaceService::SetOverlayWindowHandle(uintptr_t hwndValue) {
    overlayWindowHandle_.store(hwndValue, std::memory_order_release);
}

void Win32OverlayCoordSpaceService::ClearOverlayWindowHandle() {
    overlayWindowHandle_.store(0, std::memory_order_release);
}

void Win32OverlayCoordSpaceService::SetOverlayOriginOverride(int x, int y) {
    overlayOriginX_.store(x, std::memory_order_relaxed);
    overlayOriginY_.store(y, std::memory_order_relaxed);
    overlayOriginOverrideEnabled_.store(true, std::memory_order_release);
}

void Win32OverlayCoordSpaceService::ClearOverlayOriginOverride() {
    overlayOriginOverrideEnabled_.store(false, std::memory_order_release);
}

ScreenPoint Win32OverlayCoordSpaceService::GetOverlayOrigin() const {
    if (overlayOriginOverrideEnabled_.load(std::memory_order_acquire)) {
        ScreenPoint pt{};
        pt.x = overlayOriginX_.load(std::memory_order_relaxed);
        pt.y = overlayOriginY_.load(std::memory_order_relaxed);
        return pt;
    }
    return GetVirtualScreenOrigin();
}

ScreenPoint Win32OverlayCoordSpaceService::ScreenToOverlayPoint(const ScreenPoint& screenPt) const {
    ScreenPoint pt = screenPt;
    const ScreenPoint origin = GetOverlayOrigin();
    pt.x -= origin.x;
    pt.y -= origin.y;
    return pt;
}

ScreenPoint Win32OverlayCoordSpaceService::GetVirtualScreenOrigin() {
    ScreenPoint pt{};
    pt.x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    pt.y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    return pt;
}

} // namespace mousefx

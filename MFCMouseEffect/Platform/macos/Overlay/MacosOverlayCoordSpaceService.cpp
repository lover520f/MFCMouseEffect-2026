#include "pch.h"

#include "Platform/macos/Overlay/MacosOverlayCoordSpaceService.h"
#include "Platform/macos/Overlay/MacosOverlayCoordSpaceConversion.h"

namespace mousefx {

void MacosOverlayCoordSpaceService::SetOverlayWindowHandle(uintptr_t hwndValue) {
    overlayWindowHandle_.store(hwndValue, std::memory_order_release);
}

void MacosOverlayCoordSpaceService::ClearOverlayWindowHandle() {
    overlayWindowHandle_.store(0, std::memory_order_release);
}

void MacosOverlayCoordSpaceService::SetOverlayOriginOverride(int x, int y) {
    overlayOriginX_.store(x, std::memory_order_relaxed);
    overlayOriginY_.store(y, std::memory_order_relaxed);
    overlayOriginOverrideEnabled_.store(true, std::memory_order_release);
}

void MacosOverlayCoordSpaceService::ClearOverlayOriginOverride() {
    overlayOriginOverrideEnabled_.store(false, std::memory_order_release);
}

ScreenPoint MacosOverlayCoordSpaceService::GetOverlayOrigin() const {
    if (overlayOriginOverrideEnabled_.load(std::memory_order_acquire)) {
        ScreenPoint pt{};
        pt.x = overlayOriginX_.load(std::memory_order_relaxed);
        pt.y = overlayOriginY_.load(std::memory_order_relaxed);
        return pt;
    }
    return {};
}

ScreenPoint MacosOverlayCoordSpaceService::ScreenToOverlayPoint(const ScreenPoint& screenPt) const {
    ScreenPoint converted = screenPt;
    ScreenPoint cocoaPt{};
    if (macos_overlay_coord_conversion::TryConvertQuartzToCocoa(screenPt, &cocoaPt)) {
        converted = cocoaPt;
    }
    const ScreenPoint origin = GetOverlayOrigin();
    converted.x -= origin.x;
    converted.y -= origin.y;
    return converted;
}

} // namespace mousefx

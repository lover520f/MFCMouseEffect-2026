#include "pch.h"

#include "OverlayCoordSpace.h"
#include "MouseFx/Core/Overlay/NullOverlayCoordSpaceService.h"
#include "Platform/PlatformOverlayCoordSpaceFactory.h"

#include <memory>

namespace mousefx {
namespace {

IOverlayCoordSpaceService& CoordSpaceService() {
    static std::unique_ptr<IOverlayCoordSpaceService> service =
        platform::CreateOverlayCoordSpaceService();
    static NullOverlayCoordSpaceService fallbackService{};
    if (service) {
        return *service;
    }
    return fallbackService;
}

} // namespace

void SetOverlayWindowHandle(uintptr_t windowHandle) {
    CoordSpaceService().SetOverlayWindowHandle(windowHandle);
}

void ClearOverlayWindowHandle() {
    CoordSpaceService().ClearOverlayWindowHandle();
}

void SetOverlayOriginOverride(int x, int y) {
    CoordSpaceService().SetOverlayOriginOverride(x, y);
}

void ClearOverlayOriginOverride() {
    CoordSpaceService().ClearOverlayOriginOverride();
}

ScreenPoint GetOverlayOrigin() {
    return CoordSpaceService().GetOverlayOrigin();
}

ScreenPoint ScreenToOverlayPoint(const ScreenPoint& screenPt) {
    return CoordSpaceService().ScreenToOverlayPoint(screenPt);
}

} // namespace mousefx

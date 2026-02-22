#include "pch.h"

#include "OverlayCoordSpace.h"
#include "MouseFx/Core/Overlay/NullOverlayCoordSpaceService.h"
#include "MouseFx/Core/Protocol/InputTypesWin32.h"
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

void SetOverlayWindowHandle(HWND hwnd) {
    CoordSpaceService().SetOverlayWindowHandle(reinterpret_cast<uintptr_t>(hwnd));
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

POINT GetOverlayOrigin() {
    return ToNativePoint(CoordSpaceService().GetOverlayOrigin());
}

POINT ScreenToOverlayPoint(const POINT& screenPt) {
    return ToNativePoint(CoordSpaceService().ScreenToOverlayPoint(ToScreenPoint(screenPt)));
}

} // namespace mousefx

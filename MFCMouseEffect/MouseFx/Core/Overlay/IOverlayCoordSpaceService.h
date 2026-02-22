#pragma once

#include <cstdint>

#include "MouseFx/Core/Protocol/InputTypes.h"

namespace mousefx {

// Platform-specific overlay coordinate space service.
class IOverlayCoordSpaceService {
public:
    virtual ~IOverlayCoordSpaceService() = default;

    virtual void SetOverlayWindowHandle(uintptr_t hwndValue) = 0;
    virtual void ClearOverlayWindowHandle() = 0;
    virtual void SetOverlayOriginOverride(int x, int y) = 0;
    virtual void ClearOverlayOriginOverride() = 0;
    virtual ScreenPoint GetOverlayOrigin() const = 0;
    virtual ScreenPoint ScreenToOverlayPoint(const ScreenPoint& screenPt) const = 0;
};

} // namespace mousefx

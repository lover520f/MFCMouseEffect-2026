#pragma once

#include "MouseFx/Core/Overlay/IOverlayCoordSpaceService.h"

namespace mousefx {

class NullOverlayCoordSpaceService final : public IOverlayCoordSpaceService {
public:
    void SetOverlayWindowHandle(uintptr_t /*hwndValue*/) override {}
    void ClearOverlayWindowHandle() override {}
    void SetOverlayOriginOverride(int x, int y) override {
        origin_.x = x;
        origin_.y = y;
    }
    void ClearOverlayOriginOverride() override {}

    ScreenPoint GetOverlayOrigin() const override {
        return origin_;
    }

    ScreenPoint ScreenToOverlayPoint(const ScreenPoint& screenPt) const override {
        ScreenPoint local = screenPt;
        local.x -= origin_.x;
        local.y -= origin_.y;
        return local;
    }

private:
    ScreenPoint origin_{};
};

} // namespace mousefx

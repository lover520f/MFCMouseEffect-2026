#pragma once

#include <atomic>

#include "MouseFx/Core/Overlay/IOverlayCoordSpaceService.h"

namespace mousefx {

class Win32OverlayCoordSpaceService final : public IOverlayCoordSpaceService {
public:
    void SetOverlayWindowHandle(uintptr_t hwndValue) override;
    void ClearOverlayWindowHandle() override;
    void SetOverlayOriginOverride(int x, int y) override;
    void ClearOverlayOriginOverride() override;
    ScreenPoint GetOverlayOrigin() const override;
    ScreenPoint ScreenToOverlayPoint(const ScreenPoint& screenPt) const override;

private:
    static ScreenPoint GetVirtualScreenOrigin();

    std::atomic<bool> overlayOriginOverrideEnabled_{false};
    std::atomic<int> overlayOriginX_{0};
    std::atomic<int> overlayOriginY_{0};
    std::atomic<uintptr_t> overlayWindowHandle_{0};
};

} // namespace mousefx

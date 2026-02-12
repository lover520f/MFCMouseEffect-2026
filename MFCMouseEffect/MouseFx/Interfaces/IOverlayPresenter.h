#pragma once

#include "MouseFx/Windows/OverlayPresentFrame.h"

namespace mousefx {

class IOverlayPresenter {
public:
    virtual ~IOverlayPresenter() = default;
    virtual bool Present(const OverlayPresentFrame& frame) = 0;
};

} // namespace mousefx

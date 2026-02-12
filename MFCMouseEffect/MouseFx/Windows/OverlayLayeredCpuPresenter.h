#pragma once

#include <windows.h>
#include <gdiplus.h>

#include <vector>

#include "MouseFx/Interfaces/IOverlayPresenter.h"
#include "MouseFx/Interfaces/IOverlayLayer.h"

namespace mousefx {

class OverlayLayeredCpuPresenter final : public IOverlayPresenter {
public:
    bool Present(const OverlayPresentFrame& frame) override;
};

} // namespace mousefx

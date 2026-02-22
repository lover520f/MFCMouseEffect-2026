#pragma once

#include <memory>

#include "MouseFx/Core/Overlay/IOverlayHostBackend.h"
#include "MouseFx/Core/Overlay/IInputIndicatorOverlay.h"

namespace mousefx::platform {

std::unique_ptr<IOverlayHostBackend> CreateOverlayHostBackend();
std::unique_ptr<IInputIndicatorOverlay> CreateInputIndicatorOverlay();

} // namespace mousefx::platform

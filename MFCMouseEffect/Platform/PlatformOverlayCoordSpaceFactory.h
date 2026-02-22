#pragma once

#include <memory>

#include "MouseFx/Core/Overlay/IOverlayCoordSpaceService.h"

namespace mousefx::platform {

std::unique_ptr<IOverlayCoordSpaceService> CreateOverlayCoordSpaceService();

} // namespace mousefx::platform

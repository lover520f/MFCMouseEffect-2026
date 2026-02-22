#pragma once

#include <memory>

#include "MouseFx/Core/System/IGlobalMouseHook.h"
#include "MouseFx/Core/System/ICursorPositionService.h"

namespace mousefx::platform {

std::unique_ptr<IGlobalMouseHook> CreateGlobalMouseHook();
std::unique_ptr<ICursorPositionService> CreateCursorPositionService();

} // namespace mousefx::platform

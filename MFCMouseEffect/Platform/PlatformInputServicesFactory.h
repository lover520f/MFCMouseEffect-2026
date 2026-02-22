#pragma once

#include <memory>

#include "MouseFx/Core/System/IGlobalMouseHook.h"
#include "MouseFx/Core/System/ICursorPositionService.h"
#include "MouseFx/Core/System/IKeyboardInjector.h"

namespace mousefx::platform {

std::unique_ptr<IGlobalMouseHook> CreateGlobalMouseHook();
std::unique_ptr<ICursorPositionService> CreateCursorPositionService();
std::unique_ptr<IKeyboardInjector> CreateKeyboardInjector();

} // namespace mousefx::platform

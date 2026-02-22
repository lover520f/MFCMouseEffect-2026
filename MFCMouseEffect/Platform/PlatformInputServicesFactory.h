#pragma once

#include <memory>

#include "MouseFx/Core/System/IGlobalMouseHook.h"

namespace mousefx::platform {

std::unique_ptr<IGlobalMouseHook> CreateGlobalMouseHook();

} // namespace mousefx::platform

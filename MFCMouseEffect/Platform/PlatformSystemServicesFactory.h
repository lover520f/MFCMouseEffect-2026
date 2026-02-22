#pragma once

#include <memory>

#include "MouseFx/Core/System/IForegroundProcessService.h"
#include "MouseFx/Core/System/IMonotonicClockService.h"

namespace mousefx::platform {

std::unique_ptr<IMonotonicClockService> CreateMonotonicClockService();
std::unique_ptr<IForegroundProcessService> CreateForegroundProcessService();

} // namespace mousefx::platform

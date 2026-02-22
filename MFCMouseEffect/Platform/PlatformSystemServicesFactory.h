#pragma once

#include <memory>

#include "MouseFx/Core/System/IForegroundProcessService.h"
#include "MouseFx/Core/System/IForegroundSuppressionService.h"
#include "MouseFx/Core/System/IMonotonicClockService.h"

namespace mousefx::platform {

std::unique_ptr<IMonotonicClockService> CreateMonotonicClockService();
std::unique_ptr<IForegroundProcessService> CreateForegroundProcessService();
std::unique_ptr<IForegroundSuppressionService> CreateForegroundSuppressionService();

} // namespace mousefx::platform

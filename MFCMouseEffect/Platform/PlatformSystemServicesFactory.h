#pragma once

#include <memory>

#include "MouseFx/Core/System/IMonotonicClockService.h"

namespace mousefx::platform {

std::unique_ptr<IMonotonicClockService> CreateMonotonicClockService();

} // namespace mousefx::platform

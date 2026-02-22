#pragma once

#include <memory>
#include <string>

#include "MouseFx/Interfaces/IHoldRuntime.h"

namespace mousefx::platform {

std::unique_ptr<IHoldRuntime> CreatePlatformHoldRuntime(const std::string& type);

} // namespace mousefx::platform

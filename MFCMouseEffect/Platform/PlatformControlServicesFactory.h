#pragma once

#include <memory>

#include "MouseFx/Core/Control/IDispatchMessageHost.h"

namespace mousefx::platform {

std::unique_ptr<IDispatchMessageHost> CreateDispatchMessageHost();

} // namespace mousefx::platform

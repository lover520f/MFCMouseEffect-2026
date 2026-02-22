#pragma once

#include <memory>

#include "MouseFx/Core/Control/IDispatchMessageCodec.h"

namespace mousefx::platform {

std::unique_ptr<IDispatchMessageCodec> CreateDispatchMessageCodec();

} // namespace mousefx::platform

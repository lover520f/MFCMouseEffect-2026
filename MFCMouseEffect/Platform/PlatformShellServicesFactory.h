#pragma once

#include "MouseFx/Core/Shell/ShellPlatformServices.h"

namespace mousefx::platform {

// Creates platform-specific shell services (windows/macos/linux packages).
ShellPlatformServices CreateShellPlatformServices();

} // namespace mousefx::platform

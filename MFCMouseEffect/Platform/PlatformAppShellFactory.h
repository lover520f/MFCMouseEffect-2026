#pragma once

#include <memory>

#include "MouseFx/Core/Shell/ShellPlatformServices.h"
#include "Platform/IPlatformAppShell.h"

namespace mousefx::platform {

// Creates the process-level platform app shell.
std::unique_ptr<IPlatformAppShell> CreatePlatformAppShell(ShellPlatformServices services = {});

} // namespace mousefx::platform

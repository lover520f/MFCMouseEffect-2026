#pragma once

#include <string>

namespace mousefx::platform {

// Returns whether native launch-at-startup control is implemented on current platform.
bool IsLaunchAtStartupSupported();

// Applies launch-at-startup policy to the native platform.
// Best-effort API: returns true on successful native apply.
bool ConfigureLaunchAtStartup(bool enabled, std::string* outError);

} // namespace mousefx::platform


#pragma once

#include "Platform/posix/Shell/ScaffoldSettingsRouteConfig.h"

#include <string_view>

namespace mousefx::platform::scaffold {

void ParseLoopbackUrlRoute(std::string_view url, SettingsRoute* route);

} // namespace mousefx::platform::scaffold

#pragma once

#include "Platform/PlatformDisplayTopology.h"

namespace mousefx::platform::windows {

class Win32DisplayTopology final {
public:
    static std::vector<DisplayMonitorEntry> EnumerateDisplayMonitors();

    static std::pair<std::string, DisplayRect> ResolveTargetDisplayMonitor(
        const std::string& targetMonitor,
        DisplayPoint cursorPt);
};

} // namespace mousefx::platform::windows


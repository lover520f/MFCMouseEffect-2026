#include "pch.h"

#include "Platform/PlatformDisplayTopology.h"

#if defined(_WIN32)
#include "Platform/windows/System/Win32DisplayTopology.h"
#endif

namespace mousefx::platform {

std::vector<DisplayMonitorEntry> EnumerateDisplayMonitors() {
#if defined(_WIN32)
    return windows::Win32DisplayTopology::EnumerateDisplayMonitors();
#else
    return {};
#endif
}

std::pair<std::string, DisplayRect> ResolveTargetDisplayMonitor(
    const std::string& targetMonitor,
    DisplayPoint cursorPt) {
#if defined(_WIN32)
    return windows::Win32DisplayTopology::ResolveTargetDisplayMonitor(targetMonitor, cursorPt);
#else
    (void)targetMonitor;
    (void)cursorPt;
    return { "", {} };
#endif
}

DisplayRect ResolveTargetDisplayMonitorBounds(const std::string& targetMonitor, DisplayPoint cursorPt) {
    return ResolveTargetDisplayMonitor(targetMonitor, cursorPt).second;
}

} // namespace mousefx::platform


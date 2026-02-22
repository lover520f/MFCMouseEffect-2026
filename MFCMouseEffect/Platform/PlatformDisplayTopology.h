#pragma once

#include <string>
#include <utility>
#include <vector>

namespace mousefx::platform {

struct DisplayRect final {
    int left = 0;
    int top = 0;
    int right = 0;
    int bottom = 0;
};

struct DisplayPoint final {
    int x = 0;
    int y = 0;
};

struct DisplayMonitorEntry final {
    std::string id;
    std::wstring deviceName;
    DisplayRect bounds{};
    bool isPrimary = false;
};

std::vector<DisplayMonitorEntry> EnumerateDisplayMonitors();

std::pair<std::string, DisplayRect> ResolveTargetDisplayMonitor(
    const std::string& targetMonitor,
    DisplayPoint cursorPt);

DisplayRect ResolveTargetDisplayMonitorBounds(
    const std::string& targetMonitor,
    DisplayPoint cursorPt);

} // namespace mousefx::platform


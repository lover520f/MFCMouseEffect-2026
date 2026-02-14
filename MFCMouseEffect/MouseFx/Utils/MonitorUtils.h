#pragma once

#include <windows.h>
#include <string>
#include <vector>

namespace mousefx {

/// Information about a single connected display monitor.
struct MonitorEntry {
    std::string id;         // Stable identifier: "monitor_1", "monitor_2", ...
    std::wstring deviceName; // Windows device name, e.g. \\.\DISPLAY1
    RECT bounds{};          // Virtual-screen coordinates
    bool isPrimary = false;
};

/// Enumerate all currently connected monitors.
/// Results are ordered by left-to-right, top-to-bottom virtual coordinates.
std::vector<MonitorEntry> EnumMonitors();

/// Resolve a target-monitor specifier to a bounding rectangle.
/// @param targetMonitor  "cursor" | "primary" | "monitor_1" | "monitor_2" | ...
/// @param cursorPt       Current cursor position (used when targetMonitor == "cursor").
/// @return The bounding rectangle of the resolved monitor; falls back to the
///         primary monitor (or the full virtual screen if enumeration fails).
RECT ResolveTargetMonitorBounds(const std::string& targetMonitor, POINT cursorPt);

/// Resolve target monitor to ID and Bounds.
/// Returns {"monitor_N", bounds} or primary/virtual screen fallback.
std::pair<std::string, RECT> ResolveTargetMonitor(const std::string& targetMonitor, POINT cursorPt);

} // namespace mousefx

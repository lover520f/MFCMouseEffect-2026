#include "pch.h"

#include "Platform/windows/System/Win32DisplayTopology.h"

#include <windows.h>

#include <algorithm>

namespace mousefx::platform::windows {
namespace {

struct EnumState final {
    std::vector<DisplayMonitorEntry> monitors;
    int index = 0;
};

DisplayRect ToDisplayRect(const RECT& rect) {
    DisplayRect out{};
    out.left = rect.left;
    out.top = rect.top;
    out.right = rect.right;
    out.bottom = rect.bottom;
    return out;
}

RECT ToNativeRect(const DisplayRect& rect) {
    RECT out{};
    out.left = rect.left;
    out.top = rect.top;
    out.right = rect.right;
    out.bottom = rect.bottom;
    return out;
}

POINT ToNativePoint(DisplayPoint pt) {
    POINT out{};
    out.x = pt.x;
    out.y = pt.y;
    return out;
}

BOOL CALLBACK EnumMonitorsCallback(HMONITOR monitor, HDC, LPRECT, LPARAM lParam) {
    auto* state = reinterpret_cast<EnumState*>(lParam);
    MONITORINFOEXW info{};
    info.cbSize = sizeof(info);
    if (!::GetMonitorInfoW(monitor, &info)) {
        return TRUE;
    }

    DisplayMonitorEntry entry{};
    entry.id = "monitor_" + std::to_string(++state->index);
    entry.deviceName = info.szDevice;
    entry.bounds = ToDisplayRect(info.rcMonitor);
    entry.isPrimary = (info.dwFlags & MONITORINFOF_PRIMARY) != 0;
    state->monitors.push_back(std::move(entry));
    return TRUE;
}

DisplayRect GetVirtualScreenRect() {
    DisplayRect rect{};
    rect.left = ::GetSystemMetrics(SM_XVIRTUALSCREEN);
    rect.top = ::GetSystemMetrics(SM_YVIRTUALSCREEN);
    rect.right = rect.left + ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
    rect.bottom = rect.top + ::GetSystemMetrics(SM_CYVIRTUALSCREEN);
    return rect;
}

} // namespace

std::vector<DisplayMonitorEntry> Win32DisplayTopology::EnumerateDisplayMonitors() {
    EnumState state{};
    ::EnumDisplayMonitors(nullptr, nullptr, &EnumMonitorsCallback, reinterpret_cast<LPARAM>(&state));

    // Sort left-to-right, then top-to-bottom for deterministic ordering.
    std::sort(
        state.monitors.begin(),
        state.monitors.end(),
        [](const DisplayMonitorEntry& lhs, const DisplayMonitorEntry& rhs) {
            if (lhs.bounds.left != rhs.bounds.left) {
                return lhs.bounds.left < rhs.bounds.left;
            }
            return lhs.bounds.top < rhs.bounds.top;
        });

    for (size_t i = 0; i < state.monitors.size(); ++i) {
        state.monitors[i].id = "monitor_" + std::to_string(i + 1);
    }
    return state.monitors;
}

std::pair<std::string, DisplayRect> Win32DisplayTopology::ResolveTargetDisplayMonitor(
    const std::string& targetMonitor,
    DisplayPoint cursorPt) {
    const auto monitors = EnumerateDisplayMonitors();
    if (monitors.empty()) {
        return { "", GetVirtualScreenRect() };
    }

    if (!targetMonitor.empty() && targetMonitor != "cursor" && targetMonitor != "primary") {
        for (const auto& monitor : monitors) {
            if (monitor.id == targetMonitor) {
                return { monitor.id, monitor.bounds };
            }
        }
    }

    if (targetMonitor == "primary") {
        for (const auto& monitor : monitors) {
            if (monitor.isPrimary) {
                return { monitor.id, monitor.bounds };
            }
        }
        return { monitors.front().id, monitors.front().bounds };
    }

    const HMONITOR monitor = ::MonitorFromPoint(ToNativePoint(cursorPt), MONITOR_DEFAULTTONEAREST);
    MONITORINFOEXW info{};
    info.cbSize = sizeof(info);
    if (::GetMonitorInfoW(monitor, &info)) {
        for (const auto& entry : monitors) {
            if (entry.deviceName == info.szDevice) {
                return { entry.id, entry.bounds };
            }
        }
        return { "", ToDisplayRect(info.rcMonitor) };
    }

    return { "", GetVirtualScreenRect() };
}

} // namespace mousefx::platform::windows


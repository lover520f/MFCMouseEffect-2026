#include "pch.h"

#include "MonitorUtils.h"

#include <algorithm>

namespace mousefx {

namespace {

struct EnumState {
    std::vector<MonitorEntry> monitors;
    int index = 0;
};

BOOL CALLBACK EnumMonitorsCallback(HMONITOR hMon, HDC /*hdc*/, LPRECT /*lpRect*/, LPARAM lParam) {
    auto* state = reinterpret_cast<EnumState*>(lParam);
    MONITORINFOEXW mi{};
    mi.cbSize = sizeof(mi);
    if (!GetMonitorInfoW(hMon, &mi)) return TRUE;

    MonitorEntry entry;
    entry.id = "monitor_" + std::to_string(++state->index);
    entry.deviceName = mi.szDevice;
    entry.bounds = mi.rcMonitor;
    entry.isPrimary = (mi.dwFlags & MONITORINFOF_PRIMARY) != 0;
    state->monitors.push_back(std::move(entry));
    return TRUE;
}

RECT GetVirtualScreenRect() {
    RECT r{};
    r.left   = GetSystemMetrics(SM_XVIRTUALSCREEN);
    r.top    = GetSystemMetrics(SM_YVIRTUALSCREEN);
    r.right  = r.left + GetSystemMetrics(SM_CXVIRTUALSCREEN);
    r.bottom = r.top  + GetSystemMetrics(SM_CYVIRTUALSCREEN);
    return r;
}

} // namespace

std::vector<MonitorEntry> EnumMonitors() {
    EnumState state;
    EnumDisplayMonitors(nullptr, nullptr, &EnumMonitorsCallback, reinterpret_cast<LPARAM>(&state));

    // Sort left-to-right, then top-to-bottom for stable ordering.
    std::sort(state.monitors.begin(), state.monitors.end(),
        [](const MonitorEntry& a, const MonitorEntry& b) {
            if (a.bounds.left != b.bounds.left)
                return a.bounds.left < b.bounds.left;
            return a.bounds.top < b.bounds.top;
        });

    // Re-assign ids after sort so ordering is deterministic.
    for (size_t i = 0; i < state.monitors.size(); ++i) {
        state.monitors[i].id = "monitor_" + std::to_string(i + 1);
    }

    return state.monitors;
}

std::pair<std::string, RECT> ResolveTargetMonitor(const std::string& targetMonitor, POINT cursorPt) {
    auto monitors = EnumMonitors();
    if (monitors.empty()) {
        return { "", GetVirtualScreenRect() };
    }

    // specific ID
    if (!targetMonitor.empty() && targetMonitor != "cursor" && targetMonitor != "primary") {
        for (const auto& m : monitors) {
            if (m.id == targetMonitor) return { m.id, m.bounds };
        }
        // Fallback to primary if ID not found
    }

    // primary
    if (targetMonitor == "primary") {
        for (const auto& m : monitors) {
            if (m.isPrimary) return { m.id, m.bounds };
        }
        return { monitors.front().id, monitors.front().bounds };
    }

    // cursor (default)
    HMONITOR hMon = MonitorFromPoint(cursorPt, MONITOR_DEFAULTTONEAREST);
    MONITORINFOEXW mi{};
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(hMon, &mi)) {
        // Find matching entry by device name or bounds
        for (const auto& m : monitors) {
            if (m.deviceName == mi.szDevice) return { m.id, m.bounds };
        }
        // If not found by name (unlikely), match by bounds center?
        // Or just return the info from GetMonitorInfo with unknown ID?
        // But we need text ID for overrides. 
        // Let's return the primary as fallback or just the first one if we can't match?
        // Better: return exact bounds from GetMonitorInfo, but empty ID if not matched in list.
        return { "", mi.rcMonitor };
    }

    return { "", GetVirtualScreenRect() };
}

RECT ResolveTargetMonitorBounds(const std::string& targetMonitor, POINT cursorPt) {
    return ResolveTargetMonitor(targetMonitor, cursorPt).second;
}

} // namespace mousefx

#pragma once

#include <windows.h>

namespace mousefx::gpu {

inline bool HasDesktopDisplayAdapter() {
    DISPLAY_DEVICEW dd{};
    dd.cb = sizeof(dd);
    for (DWORD i = 0; EnumDisplayDevicesW(nullptr, i, &dd, 0); ++i) {
        const DWORD flags = dd.StateFlags;
        const bool attached = (flags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) != 0;
        const bool mirror = (flags & DISPLAY_DEVICE_MIRRORING_DRIVER) != 0;
        if (attached && !mirror) {
            return true;
        }
        dd.cb = sizeof(dd);
    }
    return false;
}

} // namespace mousefx::gpu

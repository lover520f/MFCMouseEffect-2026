#pragma once

#include "MouseFx/Core/Shell/IAppShellHost.h"
#include "Platform/macos/Shell/MacosTrayMenuLocalization.h"

namespace mousefx::macos_tray {

#if defined(__APPLE__)
struct MacosTrayMenuObjects final {
    void* nativeHandle = nullptr;
};

bool BuildMacosTrayMenu(
    IAppShellHost* host,
    const MacosTrayMenuText& menuText,
    MacosTrayMenuObjects* outObjects);
void ReleaseMacosTrayMenu(MacosTrayMenuObjects* objects);
void TerminateMacosTrayApp();
#endif

} // namespace mousefx::macos_tray

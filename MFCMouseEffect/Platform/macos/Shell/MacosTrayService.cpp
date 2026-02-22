#include "Platform/macos/Shell/MacosTrayService.h"

namespace mousefx {

bool MacosTrayService::Start(IAppShellHost* host, bool showTrayIcon) {
    (void)showTrayIcon;
    return host != nullptr;
}

void MacosTrayService::Stop() {
}

void MacosTrayService::RequestExit() {
    // Native menu-bar integration is added in a later stage.
    // Exit flow is guaranteed by AppShellCore::RequestExitFromShell().
}

} // namespace mousefx

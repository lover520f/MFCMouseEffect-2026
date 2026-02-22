#include "Platform/linux/Shell/LinuxTrayService.h"

namespace mousefx {

bool LinuxTrayService::Start(IAppShellHost* host, bool showTrayIcon) {
    (void)showTrayIcon;
    return host != nullptr;
}

void LinuxTrayService::Stop() {
}

void LinuxTrayService::RequestExit() {
    // Native appindicator integration is added in a later stage.
    // Exit flow is guaranteed by AppShellCore::RequestExitFromShell().
}

} // namespace mousefx

#include "pch.h"

#include "Platform/posix/Shell/PosixScaffoldAppShell.h"

#if MFX_PLATFORM_MACOS || MFX_PLATFORM_LINUX

#include "Platform/posix/Shell/PosixShellExitCommand.h"

#include <iostream>
#include <string>
#include <thread>

namespace mousefx::platform {

void PosixScaffoldAppShell::StartStdinExitMonitor() {
    if (stdinMonitorStarted_) {
        return;
    }
    stdinMonitorStarted_ = true;

    auto* eventLoop = services_.eventLoopService.get();
    std::thread([eventLoop]() {
        std::string line;
        while (std::getline(std::cin, line)) {
            if (IsPosixShellExitCommandLine(line) && eventLoop) {
                eventLoop->RequestExit();
                return;
            }
        }
        if (eventLoop) {
            eventLoop->RequestExit();
        }
    }).detach();
}

} // namespace mousefx::platform

#endif

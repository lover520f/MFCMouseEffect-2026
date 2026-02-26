#include "pch.h"

#include "Platform/posix/Shell/PosixCoreAppShell.h"

#if MFX_PLATFORM_MACOS || MFX_PLATFORM_LINUX

#include <iostream>
#include <string>
#include <string_view>
#include <thread>

namespace mousefx::platform {
namespace {

char ToLowerAscii(char c) {
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>(c - 'A' + 'a');
    }
    return c;
}

bool EqualsIgnoreCaseAscii(std::string_view lhs, std::string_view rhs) {
    if (lhs.size() != rhs.size()) {
        return false;
    }
    for (size_t i = 0; i < lhs.size(); ++i) {
        if (ToLowerAscii(lhs[i]) != ToLowerAscii(rhs[i])) {
            return false;
        }
    }
    return true;
}

bool IsExitCommandLine(const std::string& line) {
    if (EqualsIgnoreCaseAscii(line, "exit")) {
        return true;
    }
    const std::string_view text = line;
    return text.find("\"cmd\"") != std::string_view::npos &&
           text.find("\"exit\"") != std::string_view::npos;
}

} // namespace

void PosixCoreAppShell::StartStdinExitMonitor() {
    if (stdinMonitorStarted_) {
        return;
    }
    stdinMonitorStarted_ = true;

    auto* eventLoop = services_.eventLoopService.get();
    std::thread([eventLoop]() {
        std::string line;
        while (std::getline(std::cin, line)) {
            if (IsExitCommandLine(line) && eventLoop) {
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

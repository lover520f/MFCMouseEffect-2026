#include "pch.h"
#include "framework.h"

#include "Platform/windows/Shell/Win32EventLoopService.h"

namespace mousefx {

int Win32EventLoopService::Run() {
    MSG msg{};
    for (;;) {
        const BOOL r = GetMessageW(&msg, nullptr, 0, 0);
        if (r == 0) {
            return static_cast<int>(msg.wParam);
        }
        if (r == -1) {
            return -1;
        }
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
}

void Win32EventLoopService::RequestExit() {
    PostQuitMessage(0);
}

} // namespace mousefx

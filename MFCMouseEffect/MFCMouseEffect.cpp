#include "pch.h"

#include "Platform/windows/Shell/Win32AppShell.h"

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    mousefx::Win32AppShell app;
    if (!app.Initialize()) {
        return 0;
    }
    const int code = app.RunMessageLoop();
    app.Shutdown();
    return code;
}

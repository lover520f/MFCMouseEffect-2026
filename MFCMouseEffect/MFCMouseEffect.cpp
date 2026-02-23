#include "pch.h"

#include "Platform/PlatformAppShellFactory.h"

#include <memory>

int APIENTRY wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int) {
    std::unique_ptr<mousefx::platform::IPlatformAppShell> app = mousefx::platform::CreatePlatformAppShell();
    if (!app || !app->Initialize()) {
        return 0;
    }
    const int code = app->RunMessageLoop();
    app->Shutdown();
    return code;
}

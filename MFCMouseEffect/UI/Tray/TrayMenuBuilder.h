#pragma once

#include <windows.h>

#include <string>

namespace mousefx {
class AppController;
}

class TrayMenuBuilder final {
public:
    static void BuildTrayMenu(HMENU menu, mousefx::AppController* mouseFx);

    // Returns true if cmd maps to an IPC JSON command for AppController::HandleCommand.
    static bool TryBuildIpcJson(UINT cmd, std::string* outJson);

    // Returns true if cmd maps to a theme string for AppController::SetTheme.
    static bool TryBuildTheme(UINT cmd, std::string* outTheme);
};

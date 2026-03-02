#pragma once

#include <windows.h>

#include <string>

namespace mousefx {
class AppController;
class IAppShellHost;

class Win32TrayMenuBuilder final {
public:
    static void BuildTrayMenu(HMENU menu, AppController* mouseFx, IAppShellHost* shellHost);

    // Returns true if cmd maps to an IPC JSON command for AppController::HandleCommand.
    static bool TryBuildIpcJson(UINT cmd, std::string* outJson);

    // Returns true if cmd maps to an effect category/type pair.
    static bool TryBuildEffectSelection(UINT cmd, std::string* outCategory, std::string* outEffectType);

    // Returns true if cmd maps to a theme string for AppController::SetTheme.
    static bool TryBuildTheme(UINT cmd, std::string* outTheme);
};

} // namespace mousefx

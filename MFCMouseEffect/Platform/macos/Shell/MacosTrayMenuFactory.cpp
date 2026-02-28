#include "pch.h"

#include "Platform/macos/Shell/MacosTrayMenuFactory.h"

#include "Platform/macos/Shell/MacosTrayMenuSwiftBridge.h"

namespace mousefx::macos_tray {

#if defined(__APPLE__)
namespace {

void OnOpenSettings(void* context) {
    auto* host = static_cast<IAppShellHost*>(context);
    if (host != nullptr) {
        host->OpenSettingsFromShell();
    }
}

void OnExit(void* context) {
    auto* host = static_cast<IAppShellHost*>(context);
    if (host != nullptr) {
        host->RequestExitFromShell();
    }
}

const char* NormalizeOrFallback(const std::string& value, const char* fallback) {
    if (value.empty()) {
        return fallback;
    }
    return value.c_str();
}

} // namespace

bool BuildMacosTrayMenu(
    IAppShellHost* host,
    const MacosTrayMenuText& menuText,
    MacosTrayMenuObjects* outObjects) {
    if (host == nullptr || outObjects == nullptr) {
        return false;
    }

    void* menuHandle = mfx_macos_tray_menu_create_v1(
        NormalizeOrFallback(menuText.settingsTitle, "Settings"),
        NormalizeOrFallback(menuText.exitTitle, "Exit"),
        NormalizeOrFallback(menuText.tooltip, "MFCMouseEffect"),
        host,
        &OnOpenSettings,
        &OnExit);
    if (menuHandle == nullptr) {
        return false;
    }

    outObjects->nativeHandle = menuHandle;
    mfx_macos_tray_menu_schedule_auto_open_settings_v1(menuHandle);
    return true;
}

void ReleaseMacosTrayMenu(MacosTrayMenuObjects* objects) {
    if (objects == nullptr || objects->nativeHandle == nullptr) {
        return;
    }
    mfx_macos_tray_menu_release_v1(objects->nativeHandle);
    objects->nativeHandle = nullptr;
}

void TerminateMacosTrayApp() {
    mfx_macos_tray_terminate_app_v1();
}
#endif

} // namespace mousefx::macos_tray

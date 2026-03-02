#include "pch.h"

#include "Platform/macos/Shell/MacosTrayMenuFactory.h"

#include "Platform/macos/Shell/MacosTrayMenuSwiftBridge.h"

#include <cctype>
#include <string>
#include <vector>

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

void OnSelectTheme(void* context, const char* themeValueUtf8) {
    auto* host = static_cast<IAppShellHost*>(context);
    if (host == nullptr || themeValueUtf8 == nullptr) {
        return;
    }
    std::string themeValue(themeValueUtf8);
    size_t begin = 0;
    while (begin < themeValue.size() && std::isspace(static_cast<unsigned char>(themeValue[begin]))) {
        ++begin;
    }
    size_t end = themeValue.size();
    while (end > begin && std::isspace(static_cast<unsigned char>(themeValue[end - 1]))) {
        --end;
    }
    themeValue = themeValue.substr(begin, end - begin);
    if (themeValue.empty()) {
        return;
    }
    host->SetThemeFromShell(themeValue);
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

    std::vector<ShellThemeMenuItem> themeOptions;
    std::string selectedTheme;
    host->GetThemeMenuSnapshotFromShell(menuText.preferZhLabels, &themeOptions, &selectedTheme);

    std::vector<std::string> themeValues;
    std::vector<std::string> themeLabels;
    std::vector<const char*> themeValuePointers;
    std::vector<const char*> themeLabelPointers;
    themeValues.reserve(themeOptions.size());
    themeLabels.reserve(themeOptions.size());
    themeValuePointers.reserve(themeOptions.size());
    themeLabelPointers.reserve(themeOptions.size());
    for (const auto& option : themeOptions) {
        themeValues.push_back(option.value);
        themeLabels.push_back(option.label.empty() ? option.value : option.label);
    }
    for (size_t index = 0; index < themeValues.size(); ++index) {
        themeValuePointers.push_back(themeValues[index].c_str());
        themeLabelPointers.push_back(themeLabels[index].c_str());
    }

    void* menuHandle = mfx_macos_tray_menu_create_v2(
        NormalizeOrFallback(menuText.themeTitle, "Theme"),
        NormalizeOrFallback(menuText.settingsTitle, "Settings"),
        NormalizeOrFallback(menuText.exitTitle, "Exit"),
        NormalizeOrFallback(menuText.tooltip, "MFCMouseEffect"),
        themeValuePointers.empty() ? nullptr : themeValuePointers.data(),
        themeLabelPointers.empty() ? nullptr : themeLabelPointers.data(),
        static_cast<uint32_t>(themeValuePointers.size()),
        selectedTheme.empty() ? nullptr : selectedTheme.c_str(),
        host,
        &OnOpenSettings,
        &OnExit,
        &OnSelectTheme);
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

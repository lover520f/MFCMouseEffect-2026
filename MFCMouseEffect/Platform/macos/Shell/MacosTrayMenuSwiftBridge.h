#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*MfxMacosTrayActionCallback)(void* context);
typedef void (*MfxMacosTrayThemeSelectCallback)(void* context, const char* themeValueUtf8);

void* mfx_macos_tray_menu_create_v1(
    const char* settingsTitleUtf8,
    const char* exitTitleUtf8,
    const char* tooltipUtf8,
    void* callbackContext,
    MfxMacosTrayActionCallback onOpenSettings,
    MfxMacosTrayActionCallback onExit);

void* mfx_macos_tray_menu_create_v2(
    const char* themeTitleUtf8,
    const char* settingsTitleUtf8,
    const char* exitTitleUtf8,
    const char* tooltipUtf8,
    const char* const* themeValuesUtf8,
    const char* const* themeLabelsUtf8,
    uint32_t themeCount,
    const char* selectedThemeValueUtf8,
    void* callbackContext,
    MfxMacosTrayActionCallback onOpenSettings,
    MfxMacosTrayActionCallback onExit,
    MfxMacosTrayThemeSelectCallback onThemeSelect);

void mfx_macos_tray_menu_release_v1(void* menuHandle);
void mfx_macos_tray_menu_schedule_auto_open_settings_v1(void* menuHandle);
void mfx_macos_tray_terminate_app_v1(void);

#ifdef __cplusplus
}
#endif

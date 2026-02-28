#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*MfxMacosTrayActionCallback)(void* context);

void* mfx_macos_tray_menu_create_v1(
    const char* settingsTitleUtf8,
    const char* exitTitleUtf8,
    const char* tooltipUtf8,
    void* callbackContext,
    MfxMacosTrayActionCallback onOpenSettings,
    MfxMacosTrayActionCallback onExit);

void mfx_macos_tray_menu_release_v1(void* menuHandle);
void mfx_macos_tray_menu_schedule_auto_open_settings_v1(void* menuHandle);
void mfx_macos_tray_terminate_app_v1(void);

#ifdef __cplusplus
}
#endif

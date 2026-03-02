#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*MfxMacosTrayActionCallback)(void* context);
typedef void (*MfxMacosTrayThemeSelectCallback)(void* context, const char* themeValueUtf8);
typedef void (*MfxMacosTrayEffectSelectCallback)(
    void* context,
    const char* categoryUtf8,
    const char* effectValueUtf8);

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

void* mfx_macos_tray_menu_create_v3(
    const char* themeTitleUtf8,
    const char* settingsTitleUtf8,
    const char* exitTitleUtf8,
    const char* tooltipUtf8,
    const char* const* themeValuesUtf8,
    const char* const* themeLabelsUtf8,
    uint32_t themeCount,
    const char* selectedThemeValueUtf8,
    const char* clickTitleUtf8,
    const char* const* clickValuesUtf8,
    const char* const* clickLabelsUtf8,
    uint32_t clickCount,
    const char* selectedClickValueUtf8,
    const char* trailTitleUtf8,
    const char* const* trailValuesUtf8,
    const char* const* trailLabelsUtf8,
    uint32_t trailCount,
    const char* selectedTrailValueUtf8,
    const char* scrollTitleUtf8,
    const char* const* scrollValuesUtf8,
    const char* const* scrollLabelsUtf8,
    uint32_t scrollCount,
    const char* selectedScrollValueUtf8,
    const char* holdTitleUtf8,
    const char* const* holdValuesUtf8,
    const char* const* holdLabelsUtf8,
    uint32_t holdCount,
    const char* selectedHoldValueUtf8,
    const char* hoverTitleUtf8,
    const char* const* hoverValuesUtf8,
    const char* const* hoverLabelsUtf8,
    uint32_t hoverCount,
    const char* selectedHoverValueUtf8,
    void* callbackContext,
    MfxMacosTrayActionCallback onOpenSettings,
    MfxMacosTrayActionCallback onExit,
    MfxMacosTrayThemeSelectCallback onThemeSelect,
    MfxMacosTrayEffectSelectCallback onEffectSelect);

void* mfx_macos_tray_menu_create_v4(
    const char* themeTitleUtf8,
    const char* starProjectTitleUtf8,
    const char* settingsTitleUtf8,
    const char* reloadConfigTitleUtf8,
    const char* exitTitleUtf8,
    const char* tooltipUtf8,
    const char* const* themeValuesUtf8,
    const char* const* themeLabelsUtf8,
    uint32_t themeCount,
    const char* selectedThemeValueUtf8,
    const char* clickTitleUtf8,
    const char* const* clickValuesUtf8,
    const char* const* clickLabelsUtf8,
    uint32_t clickCount,
    const char* selectedClickValueUtf8,
    const char* trailTitleUtf8,
    const char* const* trailValuesUtf8,
    const char* const* trailLabelsUtf8,
    uint32_t trailCount,
    const char* selectedTrailValueUtf8,
    const char* scrollTitleUtf8,
    const char* const* scrollValuesUtf8,
    const char* const* scrollLabelsUtf8,
    uint32_t scrollCount,
    const char* selectedScrollValueUtf8,
    const char* holdTitleUtf8,
    const char* const* holdValuesUtf8,
    const char* const* holdLabelsUtf8,
    uint32_t holdCount,
    const char* selectedHoldValueUtf8,
    const char* hoverTitleUtf8,
    const char* const* hoverValuesUtf8,
    const char* const* hoverLabelsUtf8,
    uint32_t hoverCount,
    const char* selectedHoverValueUtf8,
    void* callbackContext,
    MfxMacosTrayActionCallback onOpenSettings,
    MfxMacosTrayActionCallback onReloadConfig,
    MfxMacosTrayActionCallback onExit,
    MfxMacosTrayActionCallback onStarProject,
    MfxMacosTrayThemeSelectCallback onThemeSelect,
    MfxMacosTrayEffectSelectCallback onEffectSelect);

void mfx_macos_tray_menu_release_v1(void* menuHandle);
void mfx_macos_tray_menu_schedule_auto_open_settings_v1(void* menuHandle);
void mfx_macos_tray_terminate_app_v1(void);

#ifdef __cplusplus
}
#endif

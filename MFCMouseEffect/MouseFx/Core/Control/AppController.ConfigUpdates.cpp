// AppController.ConfigUpdates.cpp - configuration update entry points.

#include "pch.h"
#include "AppController.h"

#include "MouseFx/Core/Config/EffectConfigInternal.h"
#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterSelection.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Utils/MathUtils.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/PlatformTarget.h"
#if MFX_PLATFORM_MACOS
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#elif MFX_PLATFORM_WINDOWS
#include "Platform/windows/Overlay/Win32OverlayTimerSupport.h"
#endif

#include <cmath>

namespace mousefx {

void AppController::ApplyOverlayTargetFpsToPlatform() {
#if MFX_PLATFORM_MACOS
    macos_overlay_support::SetOverlayTargetFps(config_.overlayTargetFps);
#elif MFX_PLATFORM_WINDOWS
    win32_overlay_timer_support::SetOverlayTargetFps(config_.overlayTargetFps);
#endif
}

void AppController::SetUiLanguage(const std::string& lang) {
    if (lang.empty()) return;
    config_.uiLanguage = lang;
    PersistConfig();
}

void AppController::SetTextEffectContent(const std::vector<std::wstring>& texts) {
    config_.textClick.texts = texts;
    PersistConfig();
    // Note: TextEffect pulls from config each click, so no need to "reload" the effect object
    // unless we want to refresh its internal pool immediately.
    // TextEffect::Initialize() builds the pool.
    // We should probably re-initialize the text effect if it's active.
    // Simple way: re-set it to trigger re-init.
    if (NormalizeClickEffectType(config_.active.click) == "text") {
        SetEffect(EffectCategory::Click, "text");
    }
}

void AppController::SetTextEffectFontSize(float sizePt) {
    const float clamped = ClampFloat(sizePt, 6.0f, 96.0f);
    if (std::fabs(config_.textClick.fontSize - clamped) < 0.01f) return;
    config_.textClick.fontSize = clamped;
    PersistConfig();
    if (NormalizeClickEffectType(config_.active.click) == "text") {
        SetEffect(EffectCategory::Click, "text");
    }
}

void AppController::SetInputIndicatorConfig(const InputIndicatorConfig& cfg) {
    config_.inputIndicator = cfg;
    inputIndicatorOverlay_->UpdateConfig(config_.inputIndicator);
    PersistConfig();
}

void AppController::SetInputAutomationConfig(const InputAutomationConfig& cfg) {
    config_.automation = config_internal::SanitizeInputAutomationConfig(cfg);
    inputAutomationEngine_.UpdateConfig(config_.automation);
    PersistConfig();
}

void AppController::SetOverlayTargetFps(int targetFps) {
    const int normalized = config_internal::SanitizeOverlayTargetFps(targetFps);
    if (config_.overlayTargetFps != normalized) {
        config_.overlayTargetFps = normalized;
        PersistConfig();
    }
    ApplyOverlayTargetFpsToPlatform();
}

void AppController::SetTrailTuning(const std::string& style, const TrailProfilesConfig& profiles, const TrailRendererParamsConfig& params) {
    config_.trailStyle = style.empty() ? "custom" : style;
    config_.trailProfiles = profiles;
    config_.trailParams = params;
    PersistConfig();

    // Recreate current trail effect to apply immediately (if any).
    if (IsActiveEffectEnabled(EffectCategory::Trail)) {
        ReapplyActiveEffect(EffectCategory::Trail);
    }
}

void AppController::SetTrailLineWidth(float lineWidth) {
    const float clamped = ClampFloat(lineWidth, 1.0f, 18.0f);
    if (std::fabs(config_.trail.lineWidth - clamped) < 0.01f) {
        return;
    }
    config_.trail.lineWidth = clamped;
    PersistConfig();
    if (IsActiveEffectEnabled(EffectCategory::Trail)) {
        ReapplyActiveEffect(EffectCategory::Trail);
    }
}

void AppController::SetEffectSizeScales(const EffectSizeScaleConfig& scales) {
    const EffectSizeScaleConfig normalized = config_internal::SanitizeEffectSizeScaleConfig(scales);
    if (config_.effectSizeScales.click == normalized.click &&
        config_.effectSizeScales.trail == normalized.trail &&
        config_.effectSizeScales.scroll == normalized.scroll &&
        config_.effectSizeScales.hold == normalized.hold &&
        config_.effectSizeScales.hover == normalized.hover) {
        return;
    }
    config_.effectSizeScales = normalized;
    PersistConfig();

    if (IsActiveEffectEnabled(EffectCategory::Click)) {
        ReapplyActiveEffect(EffectCategory::Click);
    }
    if (IsActiveEffectEnabled(EffectCategory::Trail)) {
        ReapplyActiveEffect(EffectCategory::Trail);
    }
    if (IsActiveEffectEnabled(EffectCategory::Scroll)) {
        ReapplyActiveEffect(EffectCategory::Scroll);
    }
    if (IsActiveEffectEnabled(EffectCategory::Hold)) {
        ReapplyActiveEffect(EffectCategory::Hold);
    }
    if (IsActiveEffectEnabled(EffectCategory::Hover)) {
        ReapplyActiveEffect(EffectCategory::Hover);
    }
}

void AppController::SetEffectConflictPolicy(const EffectConflictPolicyConfig& policy) {
    const EffectConflictPolicyConfig normalized =
        config_internal::SanitizeEffectConflictPolicyConfig(policy);
    const EffectConflictPolicyConfig current =
        config_internal::SanitizeEffectConflictPolicyConfig(config_.effectConflictPolicy);
    if (current.holdMovePolicy == normalized.holdMovePolicy) {
        return;
    }
    config_.effectConflictPolicy = normalized;
    PersistConfig();
}

void AppController::SetThemeCatalogRootPath(const std::string& rootPath) {
    const std::string normalizedRootPath = TrimAscii(rootPath);
    if (config_.themeCatalogRootPath == normalizedRootPath) {
        return;
    }

    config_.themeCatalogRootPath = normalizedRootPath;
    ReloadThemeCatalogFromRootPath(config_.themeCatalogRootPath);
    const bool themeNormalized = NormalizeConfiguredThemeName();
    ApplyConfiguredEffects();
    if (NormalizeActiveEffectTypes() || themeNormalized) {
        PersistConfig();
        return;
    }
    PersistConfig();
}

void AppController::ResetConfig() {
    // 1. Get default config
    config_ = EffectConfig::GetDefault();
    ReloadThemeCatalogFromRootPath(config_.themeCatalogRootPath);
    NormalizeConfiguredThemeName();
    QuantumHaloPresenterSelection::SetConfiguredBackendPreference(config_.holdPresenterBackend);
    ApplyOverlayTargetFpsToPlatform();

    // 2. Save it to disk
    PersistConfig();

    // 3. Re-apply everything
    ApplyConfiguredEffects();
    inputIndicatorOverlay_->UpdateConfig(config_.inputIndicator);
    inputAutomationEngine_.UpdateConfig(config_.automation);
    ApplyWasmConfigToHost(false);

    // Theme/Language rely on being pulled by UI or re-applied if needed?
    // SettingsWnd calls sync, so it will pull new values.
    // But existing effects might need theme re-apply.
    SetTheme(config_.theme);
}

void AppController::ReloadConfigFromDisk() {
    if (configDir_.empty()) return;

    EffectConfig loaded = EffectConfig::Load(configDir_);
    config_ = loaded;
    ReloadThemeCatalogFromRootPath(config_.themeCatalogRootPath);
    const bool themeNormalized = NormalizeConfiguredThemeName();
    QuantumHaloPresenterSelection::SetConfiguredBackendPreference(config_.holdPresenterBackend);
    ApplyOverlayTargetFpsToPlatform();
    inputAutomationEngine_.UpdateConfig(config_.automation);
    ApplyWasmConfigToHost(true);

    ApplyConfiguredEffects();
    if (NormalizeActiveEffectTypes() || themeNormalized) {
        PersistConfig();
    }

#ifdef _DEBUG
    OutputDebugStringW(L"MouseFx: reload_config applied.\n");
#endif
}

} // namespace mousefx

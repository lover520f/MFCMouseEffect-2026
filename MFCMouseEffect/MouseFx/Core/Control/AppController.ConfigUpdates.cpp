// AppController.ConfigUpdates.cpp - configuration update entry points.

#include "pch.h"
#include "AppController.h"

#include "MouseFx/Core/Config/EffectConfigInternal.h"
#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterSelection.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Utils/MathUtils.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/PlatformLaunchAtStartup.h"
#include "Platform/PlatformTarget.h"
#if MFX_PLATFORM_MACOS
#include "Platform/macos/Effects/MacosOverlayRenderSupport.h"
#elif MFX_PLATFORM_WINDOWS
#include "Platform/windows/Overlay/Win32OverlayTimerSupport.h"
#endif

#include <cmath>
#include <filesystem>

namespace mousefx {
namespace {

bool IsRegularFileUtf8(const std::string& pathUtf8) {
    const std::string path = TrimAscii(pathUtf8);
    if (path.empty()) {
        return false;
    }
    const std::filesystem::path filePath(Utf8ToWString(path));
    std::error_code ec;
    return std::filesystem::exists(filePath, ec) &&
           !ec &&
           std::filesystem::is_regular_file(filePath, ec) &&
           !ec;
}

InputIndicatorConfig ResolveInputIndicatorManifestFallback(InputIndicatorConfig cfg) {
    cfg = config_internal::SanitizeInputIndicatorConfig(std::move(cfg));
    if (cfg.renderMode != "wasm") {
        return cfg;
    }
    if (cfg.wasmManifestPath.empty() || IsRegularFileUtf8(cfg.wasmManifestPath)) {
        return cfg;
    }

    // Configured wasm manifest is stale/missing; degrade to native indicator immediately.
    cfg.wasmManifestPath.clear();
    cfg.renderMode = "native";
    return config_internal::SanitizeInputIndicatorConfig(std::move(cfg));
}

} // namespace

void AppController::SyncLaunchAtStartupRegistration() {
    if (!platform::IsLaunchAtStartupSupported()) {
        return;
    }
    std::string error;
    if (platform::ConfigureLaunchAtStartup(config_.launchAtStartup, &error)) {
        return;
    }
#ifdef _DEBUG
    std::wstring errorWide = Utf8ToWString(error.empty() ? "launch_at_startup_apply_failed" : error);
    OutputDebugStringW((L"MouseFx: launch-at-startup apply failed: " + errorWide + L"\n").c_str());
#endif
}

void AppController::ApplyOverlayTargetFpsToPlatform() {
#if MFX_PLATFORM_MACOS
    macos_overlay_support::SetOverlayTargetFps(config_.overlayTargetFps);
#elif MFX_PLATFORM_WINDOWS
    win32_overlay_timer_support::SetOverlayTargetFps(config_.overlayTargetFps);
#endif
    ArmWasmFrameTimer();
}

void AppController::SetUiLanguage(const std::string& lang) {
    if (lang.empty()) return;
    config_.uiLanguage = lang;
    PersistConfig();
}

void AppController::SetLaunchAtStartup(bool enabled) {
    if (config_.launchAtStartup == enabled) {
        return;
    }
    config_.launchAtStartup = enabled;
    PersistConfig();
    SyncLaunchAtStartupRegistration();
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

void AppController::SetMouseCompanionConfig(const MouseCompanionConfig& cfg) {
    const MouseCompanionConfig normalized = config_internal::SanitizeMouseCompanionConfig(cfg);
    if (config_.mouseCompanion.enabled == normalized.enabled &&
        config_.mouseCompanion.modelPath == normalized.modelPath &&
        config_.mouseCompanion.actionLibraryPath == normalized.actionLibraryPath &&
        config_.mouseCompanion.appearanceProfilePath == normalized.appearanceProfilePath &&
        config_.mouseCompanion.positionMode == normalized.positionMode &&
        config_.mouseCompanion.edgeClampMode == normalized.edgeClampMode &&
        config_.mouseCompanion.sizePx == normalized.sizePx &&
        config_.mouseCompanion.offsetX == normalized.offsetX &&
        config_.mouseCompanion.offsetY == normalized.offsetY &&
        config_.mouseCompanion.absoluteX == normalized.absoluteX &&
        config_.mouseCompanion.absoluteY == normalized.absoluteY &&
        config_.mouseCompanion.targetMonitor == normalized.targetMonitor &&
        config_.mouseCompanion.pressLiftPx == normalized.pressLiftPx &&
        config_.mouseCompanion.smoothingPercent == normalized.smoothingPercent &&
        config_.mouseCompanion.followThresholdPx == normalized.followThresholdPx &&
        config_.mouseCompanion.releaseHoldMs == normalized.releaseHoldMs &&
        config_.mouseCompanion.clickStreakBreakMs == normalized.clickStreakBreakMs &&
        std::fabs(config_.mouseCompanion.headTintPerClick - normalized.headTintPerClick) < 1e-6 &&
        std::fabs(config_.mouseCompanion.headTintMax - normalized.headTintMax) < 1e-6 &&
        std::fabs(config_.mouseCompanion.headTintDecayPerSecond - normalized.headTintDecayPerSecond) < 1e-6 &&
        config_.mouseCompanion.useTestProfile == normalized.useTestProfile &&
        config_.mouseCompanion.testPressLiftPx == normalized.testPressLiftPx &&
        config_.mouseCompanion.testSmoothingPercent == normalized.testSmoothingPercent &&
        config_.mouseCompanion.testClickStreakBreakMs == normalized.testClickStreakBreakMs &&
        std::fabs(config_.mouseCompanion.testHeadTintPerClick - normalized.testHeadTintPerClick) < 1e-6 &&
        std::fabs(config_.mouseCompanion.testHeadTintMax - normalized.testHeadTintMax) < 1e-6 &&
        std::fabs(config_.mouseCompanion.testHeadTintDecayPerSecond - normalized.testHeadTintDecayPerSecond) < 1e-6) {
        return;
    }

    config_.mouseCompanion = normalized;
    MouseCompanionConfig activeCompanionCfg = normalized;
    if (activeCompanionCfg.useTestProfile) {
        activeCompanionCfg.pressLiftPx = activeCompanionCfg.testPressLiftPx;
        activeCompanionCfg.smoothingPercent = activeCompanionCfg.testSmoothingPercent;
        activeCompanionCfg.clickStreakBreakMs = activeCompanionCfg.testClickStreakBreakMs;
        activeCompanionCfg.headTintPerClick = activeCompanionCfg.testHeadTintPerClick;
        activeCompanionCfg.headTintMax = activeCompanionCfg.testHeadTintMax;
        activeCompanionCfg.headTintDecayPerSecond = activeCompanionCfg.testHeadTintDecayPerSecond;
    }
    OnMouseCompanionPluginConfigChanged(normalized, CurrentTickMs());
    {
        std::lock_guard<std::mutex> guard(mouseCompanionRuntimeStatusMutex_);
        mouseCompanionRuntimeStatus_.configEnabled = normalized.enabled;
        mouseCompanionRuntimeStatus_.runtimePresent = false;
        mouseCompanionRuntimeStatus_.visualHostActive = false;
        mouseCompanionRuntimeStatus_.visualModelLoaded = false;
        mouseCompanionRuntimeStatus_.modelLoaded = false;
        mouseCompanionRuntimeStatus_.actionLibraryLoaded = false;
        mouseCompanionRuntimeStatus_.effectProfileLoaded = false;
        mouseCompanionRuntimeStatus_.appearanceProfileLoaded = false;
        mouseCompanionRuntimeStatus_.poseBindingConfigured = false;
        mouseCompanionRuntimeStatus_.skeletonBoneCount = 0;
        mouseCompanionRuntimeStatus_.configuredModelPath = normalized.modelPath;
        mouseCompanionRuntimeStatus_.configuredActionLibraryPath = normalized.actionLibraryPath;
        mouseCompanionRuntimeStatus_.configuredEffectProfilePath = "MFCMouseEffect/Assets/Pet3D/source/pet-effects.json";
        mouseCompanionRuntimeStatus_.configuredAppearanceProfilePath = normalized.appearanceProfilePath;
        mouseCompanionRuntimeStatus_.visualModelPath.clear();
        mouseCompanionRuntimeStatus_.loadedModelPath.clear();
        mouseCompanionRuntimeStatus_.loadedModelSourceFormat = "unknown";
        mouseCompanionRuntimeStatus_.loadedActionLibraryPath.clear();
        mouseCompanionRuntimeStatus_.loadedEffectProfilePath.clear();
        mouseCompanionRuntimeStatus_.loadedAppearanceProfilePath.clear();
        mouseCompanionRuntimeStatus_.modelConvertedToCanonical = false;
        mouseCompanionRuntimeStatus_.modelImportDiagnostics.clear();
        mouseCompanionRuntimeStatus_.visualModelLoadError = "backend_removed_pending_rewrite";
        mouseCompanionRuntimeStatus_.modelLoadError = "backend_removed_pending_rewrite";
        mouseCompanionRuntimeStatus_.actionLibraryLoadError = "backend_removed_pending_rewrite";
        mouseCompanionRuntimeStatus_.effectProfileLoadError = "backend_removed_pending_rewrite";
        mouseCompanionRuntimeStatus_.appearanceProfileLoadError = "backend_removed_pending_rewrite";
        mouseCompanionRuntimeStatus_.lastActionCode = 0;
        mouseCompanionRuntimeStatus_.lastActionName = "idle";
        mouseCompanionRuntimeStatus_.lastActionIntensity = 0.0f;
        mouseCompanionRuntimeStatus_.lastActionTickMs = CurrentTickMs();
        mouseCompanionRuntimeStatus_.clickStreak = 0;
        mouseCompanionRuntimeStatus_.clickStreakTintAmount = 0.0f;
        mouseCompanionRuntimeStatus_.clickStreakBreakMs = activeCompanionCfg.clickStreakBreakMs;
        mouseCompanionRuntimeStatus_.clickStreakDecayPerSecond =
            static_cast<float>(activeCompanionCfg.headTintDecayPerSecond);
        mouseCompanionRuntimeStatus_.actionCoverageReady = false;
        mouseCompanionRuntimeStatus_.actionCoverageExpectedActionCount = 0;
        mouseCompanionRuntimeStatus_.actionCoverageCoveredActionCount = 0;
        mouseCompanionRuntimeStatus_.actionCoverageMissingActionCount = 0;
        mouseCompanionRuntimeStatus_.actionCoverageSkeletonBoneCount = 0;
        mouseCompanionRuntimeStatus_.actionCoverageTotalTrackCount = 0;
        mouseCompanionRuntimeStatus_.actionCoverageMappedTrackCount = 0;
        mouseCompanionRuntimeStatus_.actionCoverageOverallRatio = 0.0f;
        mouseCompanionRuntimeStatus_.actionCoverageError = "backend_removed_pending_rewrite";
        mouseCompanionRuntimeStatus_.actionCoverageMissingActions.clear();
        mouseCompanionRuntimeStatus_.actionCoverageMissingBoneNames.clear();
        mouseCompanionRuntimeStatus_.actionCoverageActions.clear();
    }
    PersistConfig();
    if (normalized.enabled) {
        TryLoadDefaultPetModel();
    } else {
        ShutdownPetVisualHost();
    }
}

void AppController::SetInputIndicatorConfig(const InputIndicatorConfig& cfg) {
    config_.inputIndicator = ResolveInputIndicatorManifestFallback(cfg);
    if (config_.inputIndicator.renderMode == "wasm") {
        EnsureInputIndicatorWasmBudgetFloor();
    }
    inputIndicatorOverlay_->UpdateConfig(config_.inputIndicator);
    PersistConfig();
    SyncInputIndicatorWasmHostToConfig();
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
    SyncLaunchAtStartupRegistration();

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
    SyncLaunchAtStartupRegistration();

    ApplyConfiguredEffects();
    if (NormalizeActiveEffectTypes() || themeNormalized) {
        PersistConfig();
    }

#ifdef _DEBUG
    OutputDebugStringW(L"MouseFx: reload_config applied.\n");
#endif
}

} // namespace mousefx

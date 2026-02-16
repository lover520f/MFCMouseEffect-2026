// AppController.ConfigUpdates.cpp - configuration update entry points.

#include "pch.h"
#include "AppController.h"

#include "MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterSelection.h"
#include "MouseFx/Utils/MathUtils.h"

#include <cmath>

namespace mousefx {

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
    if (config_.active.click == "text") {
        SetEffect(EffectCategory::Click, "text");
    }
}

void AppController::SetTextEffectFontSize(float sizePt) {
    const float clamped = ClampFloat(sizePt, 6.0f, 96.0f);
    if (std::fabs(config_.textClick.fontSize - clamped) < 0.01f) return;
    config_.textClick.fontSize = clamped;
    PersistConfig();
    if (config_.active.click == "text") {
        SetEffect(EffectCategory::Click, "text");
    }
}

void AppController::SetInputIndicatorConfig(const InputIndicatorConfig& cfg) {
    config_.inputIndicator = cfg;
    inputIndicatorOverlay_.UpdateConfig(config_.inputIndicator);
    PersistConfig();
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

void AppController::ResetConfig() {
    // 1. Get default config
    config_ = EffectConfig::GetDefault();
    QuantumHaloPresenterSelection::SetConfiguredBackendPreference(config_.holdPresenterBackend);

    // 2. Save it to disk
    PersistConfig();

    // 3. Re-apply everything
    ApplyConfiguredEffects();
    inputIndicatorOverlay_.UpdateConfig(config_.inputIndicator);

    // Theme/Language rely on being pulled by UI or re-applied if needed?
    // SettingsWnd calls sync, so it will pull new values.
    // But existing effects might need theme re-apply.
    SetTheme(config_.theme);
}

void AppController::ReloadConfigFromDisk() {
    if (configDir_.empty()) return;

    EffectConfig loaded = EffectConfig::Load(configDir_);
    config_ = loaded;
    QuantumHaloPresenterSelection::SetConfiguredBackendPreference(config_.holdPresenterBackend);

    ApplyConfiguredEffects();
    if (NormalizeActiveEffectTypes()) {
        PersistConfig();
    }

#ifdef _DEBUG
    OutputDebugStringW(L"MouseFx: reload_config applied.\n");
#endif
}

} // namespace mousefx

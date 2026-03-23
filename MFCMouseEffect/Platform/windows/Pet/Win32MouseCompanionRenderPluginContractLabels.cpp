#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRenderPluginContractLabels.h"

#include "MouseFx/Utils/StringUtils.h"

namespace mousefx::windows {

bool IsSupportedWin32MouseCompanionRenderPluginStyleIntent(
    const std::string& value) {
    return value == "style_candidate:none" ||
        value == "style_candidate:builtin_passthrough_baseline" ||
        value == "style_candidate:balanced_default_candidate" ||
        value == "style_candidate:agile_follow_drag" ||
        value == "style_candidate:dreamy_follow_scroll" ||
        value == "style_candidate:charming_click_hold" ||
        value == "style_candidate:single_selected_wasm_v1";
}

bool IsSupportedWin32MouseCompanionRenderPluginSampleTier(
    const std::string& value) {
    return value == "baseline_reference" ||
        value == "ship_default_candidate" ||
        value == "experimental_style_candidate";
}

std::string ResolveWin32MouseCompanionRenderPluginDefaultLaneStyleIntent(
    const std::string& defaultLaneCandidate,
    const std::string& declaredStyleIntent,
    Win32MouseCompanionRealRendererAppearanceComboPreset comboPresetOverride) {
    const std::string trimmedDeclaredStyleIntent = TrimAscii(declaredStyleIntent);
    if (!trimmedDeclaredStyleIntent.empty() && TrimAscii(defaultLaneCandidate) != "builtin") {
        return trimmedDeclaredStyleIntent;
    }

    if (defaultLaneCandidate == "builtin_passthrough") {
        return "style_candidate:builtin_passthrough_baseline";
    }
    if (defaultLaneCandidate != "wasm_v1") {
        return "style_candidate:none";
    }

    switch (comboPresetOverride) {
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Agile:
        return "style_candidate:agile_follow_drag";
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Dreamy:
        return "style_candidate:dreamy_follow_scroll";
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Charming:
        return "style_candidate:charming_click_hold";
    case Win32MouseCompanionRealRendererAppearanceComboPreset::None:
        return "style_candidate:balanced_default_candidate";
    }

    return "style_candidate:none";
}

std::string BuildWin32MouseCompanionRenderPluginContractBrief(
    const std::string& appearanceSemanticsMode,
    const std::string& styleIntent,
    const std::string& sampleTier) {
    const std::string mode = TrimAscii(appearanceSemanticsMode).empty()
        ? "-"
        : TrimAscii(appearanceSemanticsMode);
    const std::string intent = TrimAscii(styleIntent).empty()
        ? "-"
        : TrimAscii(styleIntent);
    const std::string tier = TrimAscii(sampleTier).empty()
        ? "-"
        : TrimAscii(sampleTier);
    return mode + "/" + intent + "/" + tier;
}

} // namespace mousefx::windows

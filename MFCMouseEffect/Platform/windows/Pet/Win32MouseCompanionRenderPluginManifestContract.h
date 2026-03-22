#pragma once

#include <string>

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"

namespace mousefx::wasm {
struct HostDiagnostics;
}

namespace mousefx::windows {

struct Win32MouseCompanionRendererPluginAppearanceSemanticsTuning final {
    float followLiftScale{1.0f};
    float clickSquashScale{1.0f};
    float dragLeanScale{1.0f};
    float highlightAlphaScale{1.0f};
    float followTailSwingScale{1.0f};
    float holdHeadNodScale{1.0f};
    float scrollTailLiftScale{1.0f};
    float followHeadNodScale{1.0f};
};

struct Win32MouseCompanionRendererPluginAppearanceSemanticsPatch final {
    bool hasThemeGlowColor{false};
    Gdiplus::Color themeGlowColor{};
    bool hasThemeBodyStroke{false};
    Gdiplus::Color themeBodyStroke{};
    bool hasThemeHeadFill{false};
    Gdiplus::Color themeHeadFill{};
    bool hasThemeAccentFill{false};
    Gdiplus::Color themeAccentFill{};
    bool hasThemeAccessoryFill{false};
    Gdiplus::Color themeAccessoryFill{};
    bool hasThemePedestalFill{false};
    Gdiplus::Color themePedestalFill{};

    bool hasFrameBodyWidthScale{false};
    float frameBodyWidthScale{1.0f};
    bool hasFrameHeadWidthScale{false};
    float frameHeadWidthScale{1.0f};
    bool hasFrameHeadHeightScale{false};
    float frameHeadHeightScale{1.0f};

    bool hasFaceBlushWidthScale{false};
    float faceBlushWidthScale{1.0f};
    bool hasFacePupilFocusScale{false};
    float facePupilFocusScale{1.0f};
    bool hasFaceHighlightAlphaScale{false};
    float faceHighlightAlphaScale{1.0f};
    bool hasFaceWhiskerSpreadScale{false};
    float faceWhiskerSpreadScale{1.0f};

    bool hasAppendageEarScale{false};
    float appendageEarScale{1.0f};
    bool hasAppendageTailWidthScale{false};
    float appendageTailWidthScale{1.0f};
    bool hasAppendageFollowTailWidthScale{false};
    float appendageFollowTailWidthScale{1.0f};
    bool hasAppendageFollowEarSpreadScale{false};
    float appendageFollowEarSpreadScale{1.0f};

    bool hasMotionFollowStateLiftScale{false};
    float motionFollowStateLiftScale{1.0f};
    bool hasMotionClickSquashScale{false};
    float motionClickSquashScale{1.0f};
    bool hasMotionDragLeanScale{false};
    float motionDragLeanScale{1.0f};
    bool hasMotionHoldHeadNodScale{false};
    float motionHoldHeadNodScale{1.0f};
    bool hasMotionScrollTailLiftScale{false};
    float motionScrollTailLiftScale{1.0f};
    bool hasMotionFollowHeadNodScale{false};
    float motionFollowHeadNodScale{1.0f};

    bool hasMoodGlowTintMixScale{false};
    float moodGlowTintMixScale{1.0f};
    bool hasMoodAccentTintMixScale{false};
    float moodAccentTintMixScale{1.0f};
    bool hasMoodShadowAlphaBias{false};
    float moodShadowAlphaBias{0.0f};
    bool hasMoodPedestalAlphaBias{false};
    float moodPedestalAlphaBias{0.0f};
    bool hasMoodScrollArcAlphaScale{false};
    float moodScrollArcAlphaScale{1.0f};
    bool hasMoodFollowTrailAlphaScale{false};
    float moodFollowTrailAlphaScale{1.0f};
};

struct Win32MouseCompanionWasmRenderPluginManifestPreflight final {
    bool ok{false};
    std::string pluginId;
    std::string failureReason;
    std::string metadataPath;
    uint32_t metadataSchemaVersion{0};
    std::string appearanceSemanticsMode{"legacy_manifest_compat"};
    Win32MouseCompanionRealRendererAppearanceComboPreset comboPresetOverride{
        Win32MouseCompanionRealRendererAppearanceComboPreset::None};
    Win32MouseCompanionRendererPluginAppearanceSemanticsTuning tuning{};
    Win32MouseCompanionRendererPluginAppearanceSemanticsPatch appearanceSemanticsPatch{};
};

Win32MouseCompanionWasmRenderPluginManifestPreflight
PreflightWin32MouseCompanionWasmRenderPluginManifest(
    const std::string& manifestPathUtf8);

std::string BuildWin32MouseCompanionWasmRenderPluginLoadFailureReason(
    const wasm::HostDiagnostics& diagnostics);

} // namespace mousefx::windows

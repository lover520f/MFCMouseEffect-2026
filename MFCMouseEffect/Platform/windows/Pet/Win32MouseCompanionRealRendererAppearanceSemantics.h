#pragma once

#include <gdiplus.h>
#include <vector>

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererSceneRuntime.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererStyleProfile.h"

namespace mousefx::windows {

enum class Win32MouseCompanionRealRendererAppearanceAccessoryFamily {
    None = 0,
    Star,
    Moon,
    Leaf,
    RibbonBow,
};

enum class Win32MouseCompanionRealRendererAppearanceComboPreset {
    None = 0,
    Dreamy,
    Agile,
    Charming,
};

struct Win32MouseCompanionRealRendererAppearanceTheme final {
    Gdiplus::Color glowColor{};
    Gdiplus::Color baseBodyFill{};
    Gdiplus::Color bodyStroke{};
    Gdiplus::Color headFill{};
    Gdiplus::Color headFillRear{};
    Gdiplus::Color earFill{};
    Gdiplus::Color earFillRear{};
    Gdiplus::Color earInner{};
    Gdiplus::Color earInnerRear{};
    Gdiplus::Color blushRgb{};
    Gdiplus::Color accentFill{};
    Gdiplus::Color pedestalFill{};
    Gdiplus::Color accessoryFill{};
    Gdiplus::Color accessoryStroke{};
};

struct Win32MouseCompanionRealRendererAppearanceFrameSemantics final {
    float bodyWidthScale{1.0f};
    float bodyHeightScale{1.0f};
    float headWidthScale{1.0f};
    float headHeightScale{1.0f};
    float shoulderPatchScale{1.0f};
    float hipPatchScale{1.0f};
};

struct Win32MouseCompanionRealRendererAppearanceFaceSemantics final {
    float blushWidthScale{1.0f};
    float cheekWidthScale{1.0f};
    float cheekHeightScale{1.0f};
    float jawHeightScale{1.0f};
    float muzzleWidthScale{1.0f};
    float muzzleHeightScale{1.0f};
    float foreheadWidthScale{1.0f};
    float foreheadHeightScale{1.0f};
    float templeHeightScale{1.0f};
    float browTiltScale{1.0f};
    float mouthReactiveScale{1.0f};
    float pupilFocusScale{1.0f};
    float highlightAlphaScale{1.0f};
    float whiskerSpreadScale{1.0f};
};

struct Win32MouseCompanionRealRendererAppearanceAppendageSemantics final {
    float tailWidthScale{1.0f};
    float tailHeightScale{1.0f};
    float earScale{1.0f};
    float followHandReachScale{1.0f};
    float dragHandReachScale{1.0f};
    float holdLegStanceScale{1.0f};
    float followLegStanceScale{1.0f};
    float followTailWidthScale{1.0f};
    float scrollTailHeightScale{1.0f};
    float followEarSpreadScale{1.0f};
    float clickEarLiftScale{1.0f};
};

struct Win32MouseCompanionRealRendererAppearanceMotionSemantics final {
    float followStateLiftScale{1.0f};
    float clickSquashScale{1.0f};
    float dragLeanScale{1.0f};
    float bodyForwardScale{1.0f};
    float holdHeadNodScale{1.0f};
    float followHeadNodScale{1.0f};
    float followTailSwingScale{1.0f};
    float scrollTailLiftScale{1.0f};
};

struct Win32MouseCompanionRealRendererAppearanceAdornmentSemantics final {
    Win32MouseCompanionRealRendererAppearanceAccessoryFamily family{
        Win32MouseCompanionRealRendererAppearanceAccessoryFamily::None};
    float xOffsetRatio{0.0f};
    float yOffsetRatio{0.0f};
    float widthScale{1.0f};
    float heightScale{1.0f};
    float followLiftScale{1.0f};
    float scrollLiftScale{1.0f};
    float dragShiftScale{1.0f};
    float clickBounceScale{1.0f};
    float holdSettleScale{1.0f};
};

struct Win32MouseCompanionRealRendererAppearanceMoodSemantics final {
    float glowTintMixScale{1.0f};
    float accentTintMixScale{1.0f};
    float shadowTintMixScale{1.0f};
    float pedestalTintMixScale{1.0f};
    float shadowAlphaBias{0.0f};
    float pedestalAlphaBias{0.0f};
    float clickRingAlphaScale{1.0f};
    float holdBandAlphaScale{1.0f};
    float scrollArcAlphaScale{1.0f};
    float dragLineAlphaScale{1.0f};
    float followTrailAlphaScale{1.0f};
};

struct Win32MouseCompanionRealRendererAppearanceSemantics final {
    Win32MouseCompanionRealRendererAppearanceComboPreset comboPreset{
        Win32MouseCompanionRealRendererAppearanceComboPreset::None};
    Win32MouseCompanionRealRendererAppearanceTheme theme{};
    Win32MouseCompanionRealRendererAppearanceFrameSemantics frame{};
    Win32MouseCompanionRealRendererAppearanceFaceSemantics face{};
    Win32MouseCompanionRealRendererAppearanceAppendageSemantics appendage{};
    Win32MouseCompanionRealRendererAppearanceMotionSemantics motion{};
    Win32MouseCompanionRealRendererAppearanceAdornmentSemantics adornment{};
    Win32MouseCompanionRealRendererAppearanceMoodSemantics mood{};
};

Win32MouseCompanionRealRendererAppearanceSemantics BuildWin32MouseCompanionRealRendererAppearanceSemantics(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererStyleProfile& style);

Win32MouseCompanionRealRendererAppearanceAccessoryFamily
ResolveWin32MouseCompanionRealRendererAppearanceAccessoryFamily(
    const std::vector<std::string>& accessoryIds);

Win32MouseCompanionRealRendererAppearanceComboPreset
ResolveWin32MouseCompanionRealRendererAppearanceComboPreset(
    const std::string& skinVariantId,
    Win32MouseCompanionRealRendererAppearanceAccessoryFamily family);

const char* ToStringWin32MouseCompanionRealRendererAppearanceAccessoryFamily(
    Win32MouseCompanionRealRendererAppearanceAccessoryFamily family);

const char* ToStringWin32MouseCompanionRealRendererAppearanceComboPreset(
    Win32MouseCompanionRealRendererAppearanceComboPreset preset);

bool TryParseWin32MouseCompanionRealRendererAppearanceComboPreset(
    const std::string& raw,
    Win32MouseCompanionRealRendererAppearanceComboPreset* outPreset);

} // namespace mousefx::windows

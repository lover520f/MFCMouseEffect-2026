#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPaletteBuilder.h"

#include <algorithm>
#include <cmath>

namespace mousefx::windows {
namespace {

Gdiplus::Color MakeColor(BYTE a, BYTE r, BYTE g, BYTE b) {
    return Gdiplus::Color(a, r, g, b);
}

Gdiplus::Color Darken(const Gdiplus::Color& color, float factor) {
    const float clamped = std::clamp(factor, 0.0f, 1.0f);
    const auto scale = [clamped](BYTE c) -> BYTE {
        return static_cast<BYTE>(std::lround(static_cast<float>(c) * (1.0f - clamped)));
    };
    return MakeColor(color.GetA(), scale(color.GetR()), scale(color.GetG()), scale(color.GetB()));
}

BYTE BlendChannel(BYTE from, BYTE to, float mix) {
    const float clampedMix = std::clamp(mix, 0.0f, 1.0f);
    const float blended =
        static_cast<float>(from) + (static_cast<float>(to) - static_cast<float>(from)) * clampedMix;
    return static_cast<BYTE>(std::clamp(std::lround(blended), 0L, 255L));
}

Gdiplus::Color BlendToward(const Gdiplus::Color& base, const Gdiplus::Color& tint, float mix) {
    return MakeColor(
        BlendChannel(base.GetA(), tint.GetA(), mix),
        BlendChannel(base.GetR(), tint.GetR(), mix),
        BlendChannel(base.GetG(), tint.GetG(), mix),
        BlendChannel(base.GetB(), tint.GetB(), mix));
}

} // namespace

void BuildWin32MouseCompanionRealRendererPalette(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererMotionProfile& profile,
    const Win32MouseCompanionRealRendererStyleProfile& style,
    const Win32MouseCompanionRealRendererPaletteProfile& palette,
    Win32MouseCompanionRealRendererScene& scene) {
    const auto baseBody = palette.baseBodyFill;
    const float emphasis = std::max({profile.actionIntensity, profile.reactiveIntensity, profile.scrollIntensity});
    const float shadowEmphasis = std::clamp(
        emphasis + (runtime.hold ? 0.18f : 0.0f) + (runtime.follow ? 0.08f : 0.0f),
        0.0f,
        1.0f);
    const float shadowAlphaBias = runtime.follow ? style.followShadowAlphaBias
        : runtime.hold                           ? style.holdShadowAlphaBias
        : runtime.scroll                         ? style.scrollShadowAlphaBias
        : runtime.drag                           ? style.dragShadowAlphaBias
                                                 : 0.0f;
    const float pedestalAlphaBias = runtime.follow ? style.followPedestalAlphaBias
        : runtime.hold                             ? style.holdPedestalAlphaBias
        : runtime.scroll                           ? style.scrollPedestalAlphaBias
        : runtime.drag                             ? style.dragPedestalAlphaBias
                                                   : 0.0f;
    const auto appearanceSemantics = BuildWin32MouseCompanionRealRendererAppearanceSemantics(runtime, style);
    const auto& mood = appearanceSemantics.mood;
    const auto actionTint = profile.overlayAccentColor;
    scene.glowColor = BlendToward(
        palette.glowColor,
        actionTint,
        emphasis * style.glowActionTintMix * mood.glowTintMixScale);
    scene.bodyFill = BlendToward(baseBody, actionTint, emphasis * style.bodyActionTintMix);
    scene.bodyFillRear = Darken(scene.bodyFill, style.bodyRearDarkenFactor);
    scene.bodyStroke = MakeColor(
        static_cast<BYTE>(std::clamp(style.bodyStrokeBaseAlpha + emphasis * style.bodyStrokeActionAlphaScale, 0.0f, 255.0f)),
        BlendChannel(palette.bodyStroke.GetR(), actionTint.GetR(), emphasis * style.strokeActionTintMix),
        BlendChannel(palette.bodyStroke.GetG(), actionTint.GetG(), emphasis * style.strokeActionTintMix),
        BlendChannel(palette.bodyStroke.GetB(), actionTint.GetB(), emphasis * style.strokeActionTintMix));
    scene.headFill = BlendToward(palette.headFill, actionTint, emphasis * style.headActionTintMix);
    scene.headFillRear = BlendToward(palette.headFillRear, actionTint, emphasis * style.headActionTintMix * 0.7f);
    scene.earFill = palette.earFill;
    scene.earFillRear = MakeColor(
        static_cast<BYTE>(std::clamp(style.rearEarFillAlpha, 0.0f, 255.0f)),
        BlendChannel(palette.earFillRear.GetR(), actionTint.GetR(), emphasis * style.rearEarActionTintMix),
        BlendChannel(palette.earFillRear.GetG(), actionTint.GetG(), emphasis * style.rearEarActionTintMix),
        BlendChannel(palette.earFillRear.GetB(), actionTint.GetB(), emphasis * style.rearEarActionTintMix));
    scene.earStroke = MakeColor(
        static_cast<BYTE>(std::clamp(style.frontEarStrokeAlpha, 0.0f, 255.0f)),
        scene.bodyStroke.GetR(),
        scene.bodyStroke.GetG(),
        scene.bodyStroke.GetB());
    scene.earStrokeRear = MakeColor(
        static_cast<BYTE>(std::clamp(style.rearEarStrokeAlpha, 0.0f, 255.0f)),
        BlendChannel(palette.bodyStroke.GetR(), actionTint.GetR(), emphasis * style.rearEarStrokeActionTintMix),
        BlendChannel(palette.bodyStroke.GetG(), actionTint.GetG(), emphasis * style.rearEarStrokeActionTintMix),
        BlendChannel(palette.bodyStroke.GetB(), actionTint.GetB(), emphasis * style.rearEarStrokeActionTintMix));
    scene.earStrokeWidth = style.frontEarStrokeWidth;
    scene.earStrokeWidthRear = style.rearEarStrokeWidth;
    scene.earInnerBaseInsetPx = style.frontEarInnerBaseInsetPx;
    scene.earInnerBaseInsetPxRear = style.rearEarInnerBaseInsetPx;
    scene.earInnerMidInsetPx = style.frontEarInnerMidInsetPx;
    scene.earInnerMidInsetPxRear = style.rearEarInnerMidInsetPx;
    scene.earInnerTipInsetPx = style.frontEarInnerTipInsetPx;
    scene.earInnerTipInsetPxRear = style.rearEarInnerTipInsetPx;
    scene.earOcclusionCapAlpha = style.earOcclusionCapAlpha;
    scene.earRootCuffFill = MakeColor(
        static_cast<BYTE>(std::clamp(style.frontEarRootCuffAlpha, 0.0f, 255.0f)),
        scene.headFill.GetR(),
        scene.headFill.GetG(),
        scene.headFill.GetB());
    scene.earRootCuffFillRear = MakeColor(
        static_cast<BYTE>(std::clamp(style.rearEarRootCuffAlpha, 0.0f, 255.0f)),
        BlendChannel(palette.headFillRear.GetR(), actionTint.GetR(), emphasis * style.rearEarRootCuffActionTintMix),
        BlendChannel(palette.headFillRear.GetG(), actionTint.GetG(), emphasis * style.rearEarRootCuffActionTintMix),
        BlendChannel(palette.headFillRear.GetB(), actionTint.GetB(), emphasis * style.rearEarRootCuffActionTintMix));
    scene.earInner = MakeColor(
        static_cast<BYTE>(std::clamp(style.frontEarInnerAlpha, 0.0f, 255.0f)),
        palette.earInner.GetR(),
        palette.earInner.GetG(),
        palette.earInner.GetB());
    scene.earInnerRear = MakeColor(
        static_cast<BYTE>(std::clamp(style.rearEarInnerAlpha, 0.0f, 255.0f)),
        BlendChannel(palette.earInnerRear.GetR(), actionTint.GetR(), emphasis * style.rearEarInnerActionTintMix),
        BlendChannel(palette.earInnerRear.GetG(), actionTint.GetG(), emphasis * style.rearEarInnerActionTintMix),
        BlendChannel(palette.earInnerRear.GetB(), actionTint.GetB(), emphasis * style.rearEarInnerActionTintMix));
    scene.eyeFill = palette.eyeFill;
    scene.mouthFill = palette.mouthFill;
    scene.blushFill = MakeColor(
        static_cast<BYTE>(std::clamp(profile.blushAlpha, 0.0f, 255.0f)),
        palette.blushRgb.GetR(),
        palette.blushRgb.GetG(),
        palette.blushRgb.GetB());
    scene.tailFill = Darken(scene.bodyFill, style.tailDarkenFactor);
    const auto rearTailBase = Darken(scene.bodyFillRear, style.tailRearDarkenFactor);
    scene.tailFillRear = MakeColor(
        static_cast<BYTE>(std::clamp(style.tailRearAlpha, 0.0f, 255.0f)),
        BlendChannel(
            rearTailBase.GetR(),
            actionTint.GetR(),
            emphasis * style.tailRearActionTintMix),
        BlendChannel(
            rearTailBase.GetG(),
            actionTint.GetG(),
            emphasis * style.tailRearActionTintMix),
        BlendChannel(
            rearTailBase.GetB(),
            actionTint.GetB(),
            emphasis * style.tailRearActionTintMix));
    scene.tailMidFill = MakeColor(
        static_cast<BYTE>(std::clamp(style.tailMidAlpha, 0.0f, 255.0f)),
        BlendChannel(
            scene.tailFillRear.GetR(),
            scene.headFill.GetR(),
            style.tailMidBlendTowardTip + emphasis * style.tailMidActionTintMix),
        BlendChannel(
            scene.tailFillRear.GetG(),
            scene.headFill.GetG(),
            style.tailMidBlendTowardTip + emphasis * style.tailMidActionTintMix),
        BlendChannel(
            scene.tailFillRear.GetB(),
            scene.headFill.GetB(),
            style.tailMidBlendTowardTip + emphasis * style.tailMidActionTintMix));
    scene.tailTipFill = MakeColor(
        static_cast<BYTE>(std::clamp(style.tailTipAlpha, 0.0f, 255.0f)),
        BlendChannel(scene.headFill.GetR(), actionTint.GetR(), emphasis * style.tailTipActionTintMix),
        BlendChannel(scene.headFill.GetG(), actionTint.GetG(), emphasis * style.tailTipActionTintMix),
        BlendChannel(scene.headFill.GetB(), actionTint.GetB(), emphasis * style.tailTipActionTintMix));
    scene.tailStroke = MakeColor(
        static_cast<BYTE>(std::clamp(style.tailTipStrokeAlpha, 0.0f, 255.0f)),
        scene.bodyStroke.GetR(),
        scene.bodyStroke.GetG(),
        scene.bodyStroke.GetB());
    scene.accentFill = MakeColor(
        static_cast<BYTE>(std::clamp(style.accentBaseAlpha + emphasis * style.accentActionAlphaScale, 0.0f, 255.0f)),
        BlendChannel(palette.accentFill.GetR(), actionTint.GetR(), emphasis * style.accentActionTintMix * mood.accentTintMixScale),
        BlendChannel(palette.accentFill.GetG(), actionTint.GetG(), emphasis * style.accentActionTintMix * mood.accentTintMixScale),
        BlendChannel(palette.accentFill.GetB(), actionTint.GetB(), emphasis * style.accentActionTintMix * mood.accentTintMixScale));
    scene.shadowFill = MakeColor(
        static_cast<BYTE>(std::clamp(
            style.shadowBaseAlpha + shadowEmphasis * style.shadowActionAlphaScale + shadowAlphaBias + mood.shadowAlphaBias,
            0.0f,
            255.0f)),
        BlendChannel(palette.pedestalFill.GetR(), actionTint.GetR(), emphasis * style.shadowActionTintMix * mood.shadowTintMixScale),
        BlendChannel(palette.pedestalFill.GetG(), actionTint.GetG(), emphasis * style.shadowActionTintMix * mood.shadowTintMixScale),
        BlendChannel(palette.pedestalFill.GetB(), actionTint.GetB(), emphasis * style.shadowActionTintMix * mood.shadowTintMixScale));
    scene.pedestalFill = MakeColor(
        static_cast<BYTE>(std::clamp(
            style.pedestalBaseAlpha + shadowEmphasis * style.pedestalActionAlphaScale + pedestalAlphaBias + mood.pedestalAlphaBias,
            0.0f,
            255.0f)),
        BlendChannel(palette.pedestalFill.GetR(), actionTint.GetR(), emphasis * style.pedestalActionTintMix * mood.pedestalTintMixScale),
        BlendChannel(palette.pedestalFill.GetG(), actionTint.GetG(), emphasis * style.pedestalActionTintMix * mood.pedestalTintMixScale),
        BlendChannel(palette.pedestalFill.GetB(), actionTint.GetB(), emphasis * style.pedestalActionTintMix * mood.pedestalTintMixScale));
    scene.badgeReadyFill = palette.badgeReadyFill;
    scene.badgePendingFill = palette.badgePendingFill;
    scene.accessoryFill = palette.accessoryFill;
    scene.accessoryStroke = palette.accessoryStroke;
}

} // namespace mousefx::windows

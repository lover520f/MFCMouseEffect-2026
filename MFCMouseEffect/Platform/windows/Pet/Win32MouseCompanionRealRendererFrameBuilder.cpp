#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererFrameBuilder.h"

#include <algorithm>

namespace mousefx::windows {

Win32MouseCompanionRealRendererLayoutMetrics BuildWin32MouseCompanionRealRendererFrame(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererMotionProfile& profile,
    const Win32MouseCompanionRealRendererStyleProfile& style,
    int width,
    int height,
    Win32MouseCompanionRealRendererScene& scene) {
    Win32MouseCompanionRealRendererLayoutMetrics metrics{};
    metrics.bodyWidth = static_cast<float>(width) * (runtime.hold ? style.holdBodyWidthRatio : style.bodyWidthRatio);
    metrics.bodyHeight = static_cast<float>(height) * (runtime.hold ? style.holdBodyHeightRatio : style.bodyHeightRatio);
    metrics.headWidth = metrics.bodyWidth * style.headWidthRatio;
    metrics.headHeight = metrics.bodyHeight * style.headHeightRatio;

    const float glowBoost = std::max({profile.actionIntensity, profile.reactiveIntensity, profile.scrollIntensity});
    const float facingOffset = runtime.facingSign *
        std::min(runtime.facingMomentumPx * style.facingMomentumScale, style.facingMomentumClampPx);
    const float glowScale = 1.0f + glowBoost * style.glowActionScale;

    scene.centerX = static_cast<float>(width) * style.centerXRatio + facingOffset + profile.bodyForward + profile.idleHeadSway;
    scene.centerY = static_cast<float>(height) * (runtime.hold ? style.holdCenterYRatio : style.idleCenterYRatio) -
        profile.stateLift - profile.breathLift;
    scene.facingSign = runtime.facingSign;
    scene.bodyTiltDeg = profile.scrollLean + profile.dragLean;
    scene.glowAlpha =
        style.glowBaseAlpha + glowBoost * style.glowActionAlphaScale + runtime.headTintAmount * style.glowHeadTintAlphaScale;
    scene.bodyStrokeWidth = runtime.click ? (1.9f + profile.actionIntensity * 1.0f)
        : runtime.hold      ? (1.8f + profile.actionIntensity * 0.9f)
        : runtime.scroll    ? (1.8f + profile.scrollIntensity * 0.8f)
                            : 1.8f;
    scene.headStrokeWidth = runtime.drag ? (1.9f + profile.actionIntensity * 0.9f)
        : runtime.click                  ? (1.8f + profile.actionIntensity * 0.8f)
        : runtime.follow                 ? (1.8f + profile.actionIntensity * 0.6f)
                                         : 1.8f;
    scene.limbStrokeWidth = runtime.follow ? (1.2f + profile.actionIntensity * 0.7f)
        : runtime.hold                     ? (1.2f + profile.actionIntensity * 0.5f)
                                           : 1.2f;
    scene.tailStrokeWidth = runtime.follow ? (1.2f + profile.actionIntensity * 0.6f)
        : runtime.scroll                   ? (1.2f + profile.scrollIntensity * 0.5f)
                                           : 1.2f;
    scene.chestStrokeWidth = runtime.hold ? (1.1f + profile.actionIntensity * 0.5f)
        : runtime.click                   ? (1.1f + profile.actionIntensity * 0.4f)
                                          : 1.1f;
    scene.chestFillAlpha = runtime.hold ? (255.0f - profile.actionIntensity * 18.0f)
        : runtime.follow                 ? (248.0f - profile.actionIntensity * 10.0f)
                                         : 255.0f;

    scene.glowRect = Gdiplus::RectF(
        scene.centerX - metrics.bodyWidth * style.glowXOffsetRatio,
        scene.centerY - metrics.bodyHeight * style.glowYOffsetRatio - profile.stateLift * style.glowStateLiftScale,
        metrics.bodyWidth * style.glowWidthScale * glowScale,
        metrics.bodyHeight * style.glowHeightScale * glowScale);
    scene.shadowRect = Gdiplus::RectF(
        scene.centerX - metrics.bodyWidth * style.shadowXOffsetRatio * profile.shadowScale,
        scene.centerY + metrics.bodyHeight * style.shadowYRatio,
        metrics.bodyWidth * style.shadowWidthScale * profile.shadowScale *
            (1.0f - (profile.breathScale - 1.0f) * style.shadowBreathScale),
        metrics.bodyHeight * style.shadowHeightScale);
    scene.bodyRect = Gdiplus::RectF(
        scene.centerX - metrics.bodyWidth * style.bodyXOffsetRatio,
        scene.centerY - metrics.bodyHeight * style.bodyYOffsetRatio + profile.clickSquash * style.bodyClickSquashLiftPx,
        metrics.bodyWidth * (1.0f + profile.clickSquash * style.bodyClickSquashWidthScale) * profile.breathScale,
        metrics.bodyHeight * (1.0f - profile.clickSquash * style.bodyClickSquashHeightScale) * profile.breathScale);
    scene.chestRect = Gdiplus::RectF(
        scene.centerX - metrics.bodyWidth * style.chestXOffsetRatio,
        scene.centerY - metrics.bodyHeight * style.chestYOffsetRatio,
        metrics.bodyWidth * style.chestWidthRatio,
        metrics.bodyHeight * style.chestHeightRatio);
    scene.headRect = Gdiplus::RectF(
        scene.centerX - metrics.headWidth * style.headXOffsetRatio + profile.dragLean * style.headDragLeanScale +
            profile.bodyForward * style.headBodyForwardScale,
        scene.bodyRect.Y - metrics.headHeight * style.headYOffsetRatio - profile.actionIntensity * style.headActionLiftPx +
            profile.headNod,
        metrics.headWidth,
        metrics.headHeight);
    scene.neckBridgeRect = Gdiplus::RectF(
        scene.centerX - metrics.bodyWidth * style.neckBridgeXOffsetRatio + profile.bodyForward * 0.08f,
        scene.headRect.GetBottom() - metrics.bodyHeight * style.neckBridgeYOffsetRatio,
        metrics.bodyWidth * style.neckBridgeWidthRatio,
        metrics.bodyHeight * style.neckBridgeHeightRatio);
    scene.leftShoulderPatchRect = Gdiplus::RectF(
        scene.bodyRect.X + metrics.bodyWidth * style.shoulderPatchXOffsetRatio,
        scene.bodyRect.Y + metrics.bodyHeight * style.shoulderPatchYOffsetRatio,
        metrics.bodyWidth * style.shoulderPatchWidthRatio,
        metrics.bodyHeight * style.shoulderPatchHeightRatio);
    scene.rightShoulderPatchRect = Gdiplus::RectF(
        scene.bodyRect.GetRight() - metrics.bodyWidth * (style.shoulderPatchXOffsetRatio + style.shoulderPatchWidthRatio),
        scene.bodyRect.Y + metrics.bodyHeight * style.shoulderPatchYOffsetRatio,
        metrics.bodyWidth * style.shoulderPatchWidthRatio,
        metrics.bodyHeight * style.shoulderPatchHeightRatio);
    scene.leftHipPatchRect = Gdiplus::RectF(
        scene.bodyRect.X + metrics.bodyWidth * style.hipPatchXOffsetRatio,
        scene.bodyRect.GetBottom() - metrics.bodyHeight * (style.hipPatchYOffsetRatio + style.hipPatchHeightRatio),
        metrics.bodyWidth * style.hipPatchWidthRatio,
        metrics.bodyHeight * style.hipPatchHeightRatio);
    scene.rightHipPatchRect = Gdiplus::RectF(
        scene.bodyRect.GetRight() - metrics.bodyWidth * (style.hipPatchXOffsetRatio + style.hipPatchWidthRatio),
        scene.bodyRect.GetBottom() - metrics.bodyHeight * (style.hipPatchYOffsetRatio + style.hipPatchHeightRatio),
        metrics.bodyWidth * style.hipPatchWidthRatio,
        metrics.bodyHeight * style.hipPatchHeightRatio);
    scene.pedestalRect = Gdiplus::RectF(
        scene.centerX - metrics.bodyWidth * style.pedestalXOffsetRatio,
        scene.shadowRect.GetBottom() - metrics.bodyHeight * style.pedestalYOffsetRatio,
        metrics.bodyWidth * style.pedestalWidthScale,
        metrics.bodyHeight * style.pedestalHeightScale);

    return metrics;
}

} // namespace mousefx::windows

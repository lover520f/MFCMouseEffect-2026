#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"
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
    const float bodyWidthRatio = runtime.hold ? style.holdBodyWidthRatio : style.bodyWidthRatio;
    const float bodyHeightRatio = runtime.hold ? style.holdBodyHeightRatio : style.bodyHeightRatio;
    const float bodyWidthScale = runtime.follow ? style.followBodyWidthScale
        : runtime.click                    ? style.clickBodyWidthScale
        : runtime.drag                     ? style.dragBodyWidthScale
                                           : 1.0f;
    const float bodyHeightScale = runtime.follow ? style.followBodyHeightScale
        : runtime.click                     ? style.clickBodyHeightScale
                                            : 1.0f;
    const auto appearanceSemantics =
        BuildWin32MouseCompanionRealRendererAppearanceSemantics(runtime, style);
    const auto& skinTuning = appearanceSemantics.frame;
    metrics.bodyWidth = static_cast<float>(width) * bodyWidthRatio * bodyWidthScale * skinTuning.bodyWidthScale;
    metrics.bodyHeight = static_cast<float>(height) * bodyHeightRatio * bodyHeightScale * skinTuning.bodyHeightScale;
    const float headScale = runtime.follow ? style.followHeadScale
        : runtime.hold                    ? style.holdHeadScale
        : runtime.click                   ? style.clickHeadScale
                                          : 1.0f;
    metrics.headWidth = metrics.bodyWidth * style.headWidthRatio * headScale * skinTuning.headWidthScale;
    metrics.headHeight = metrics.bodyHeight * style.headHeightRatio * headScale * skinTuning.headHeightScale;

    const float glowBoost = std::max({profile.actionIntensity, profile.reactiveIntensity, profile.scrollIntensity});
    const float facingOffset = runtime.facingSign *
        std::min(runtime.facingMomentumPx * style.facingMomentumScale, style.facingMomentumClampPx);
    const float glowStateScale = runtime.follow ? style.followGlowScale
        : runtime.hold                        ? style.holdGlowScale
        : runtime.click                       ? style.clickGlowScale
        : runtime.drag                        ? style.dragGlowScale
                                              : 1.0f;
    const float glowAlphaBias = runtime.follow ? style.followGlowAlphaBias
        : runtime.hold                        ? style.holdGlowAlphaBias
        : runtime.click                       ? style.clickGlowAlphaBias
        : runtime.scroll                      ? style.scrollGlowAlphaBias
        : runtime.drag                        ? style.dragGlowAlphaBias
                                              : 0.0f;
    const float glowStateXOffset = runtime.drag ? style.dragGlowXOffsetPx * runtime.facingSign
        : runtime.scroll                        ? style.scrollGlowXOffsetPx * runtime.facingSign
                                                : 0.0f;
    const float glowStateYOffset = runtime.follow ? style.followGlowYOffsetPx
        : runtime.hold                           ? style.holdGlowYOffsetPx
        : runtime.click                          ? style.clickGlowYOffsetPx
                                                 : 0.0f;
    const float glowScale = (1.0f + glowBoost * style.glowActionScale) * glowStateScale;
    const float shadowStateScale = runtime.follow ? style.followShadowScale
        : runtime.hold                          ? style.holdShadowScale
        : runtime.scroll                        ? style.scrollShadowScale
        : runtime.drag                          ? style.dragShadowScale
                                                : 1.0f;
    const float pedestalStateScale = runtime.follow ? style.followPedestalScale
        : runtime.hold                            ? style.holdPedestalScale
        : runtime.scroll                          ? style.scrollPedestalScale
                                                  : 1.0f;
    const float shadowStateXOffset = runtime.follow ? style.followShadowXOffsetPx
        : runtime.scroll                           ? style.scrollShadowXOffsetPx
        : runtime.drag                             ? style.dragShadowXOffsetPx
                                                   : 0.0f;
    const float shadowStateYOffset = runtime.hold ? style.holdShadowYOffsetPx : 0.0f;
    const float pedestalStateXOffset = runtime.follow ? style.followPedestalXOffsetPx
        : runtime.scroll                             ? style.scrollPedestalXOffsetPx
        : runtime.drag                               ? style.dragPedestalXOffsetPx
                                                     : 0.0f;
    const float pedestalStateYOffset = runtime.hold ? style.holdPedestalYOffsetPx : 0.0f;

    float comboGlowStateScale = 1.0f;
    float comboShadowStateScale = 1.0f;
    float comboPedestalStateScale = 1.0f;
    float comboCenterYOffset = 0.0f;
    switch (appearanceSemantics.comboPreset) {
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Dreamy:
        if (runtime.follow) {
            comboGlowStateScale = 1.06f;
            comboShadowStateScale = 0.95f;
            comboPedestalStateScale = 0.96f;
            comboCenterYOffset = -1.5f;
        }
        break;
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Agile:
        if (runtime.drag) {
            comboGlowStateScale = 0.97f;
            comboShadowStateScale = 0.94f;
            comboPedestalStateScale = 0.95f;
            comboCenterYOffset = -0.8f;
        } else if (runtime.follow) {
            comboShadowStateScale = 0.97f;
        }
        break;
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Charming:
        if (runtime.hold) {
            comboGlowStateScale = 1.04f;
            comboShadowStateScale = 1.03f;
            comboPedestalStateScale = 1.04f;
            comboCenterYOffset = -0.6f;
        }
        break;
    case Win32MouseCompanionRealRendererAppearanceComboPreset::None:
        break;
    }

    const float poseAdapterInfluence =
        ResolveWin32MouseCompanionRealRendererPoseAdapterInfluence(runtime);
    const float poseHandReachX = runtime.leftHandPose && runtime.rightHandPose
        ? (runtime.leftHandPose->position[0] + runtime.rightHandPose->position[0]) * 0.5f
        : runtime.leftHandPose ? runtime.leftHandPose->position[0]
        : runtime.rightHandPose ? runtime.rightHandPose->position[0]
                                : 0.0f;
    const float poseHandLiftY = runtime.leftHandPose && runtime.rightHandPose
        ? (runtime.leftHandPose->position[1] + runtime.rightHandPose->position[1]) * 0.5f
        : runtime.leftHandPose ? runtime.leftHandPose->position[1]
        : runtime.rightHandPose ? runtime.rightHandPose->position[1]
                                : 0.0f;
    const float poseLegReachX = runtime.leftLegPose && runtime.rightLegPose
        ? (runtime.leftLegPose->position[0] + runtime.rightLegPose->position[0]) * 0.5f
        : runtime.leftLegPose ? runtime.leftLegPose->position[0]
        : runtime.rightLegPose ? runtime.rightLegPose->position[0]
                               : 0.0f;
    const float poseLegLiftY = runtime.leftLegPose && runtime.rightLegPose
        ? (runtime.leftLegPose->position[1] + runtime.rightLegPose->position[1]) * 0.5f
        : runtime.leftLegPose ? runtime.leftLegPose->position[1]
        : runtime.rightLegPose ? runtime.rightLegPose->position[1]
                               : 0.0f;
    const float poseAnchorX =
        (poseHandReachX * metrics.bodyWidth * 0.045f + poseLegReachX * metrics.bodyWidth * 0.030f) *
        poseAdapterInfluence;
    const float poseAnchorY = (-poseHandLiftY * metrics.bodyHeight * 0.035f) * poseAdapterInfluence;
    const float poseHeadX = poseHandReachX * metrics.headWidth * 0.030f * poseAdapterInfluence;
    const float poseHeadY = -poseHandLiftY * metrics.headHeight * 0.035f * poseAdapterInfluence;
    const float poseGroundingX =
        (poseLegReachX * metrics.bodyWidth * 0.030f + poseHandReachX * metrics.bodyWidth * 0.012f) *
        poseAdapterInfluence;
    const float poseGroundingY = (-poseLegLiftY * metrics.bodyHeight * 0.020f) * poseAdapterInfluence;
    const float poseGroundingScale = 1.0f + std::abs(poseLegReachX) * 0.030f * poseAdapterInfluence;
    scene.shadowAlphaScale = 1.0f + poseAdapterInfluence * 0.08f;
    scene.pedestalAlphaScale = 1.0f + poseAdapterInfluence * 0.06f;

    scene.centerX = static_cast<float>(width) * style.centerXRatio + facingOffset + profile.bodyForward +
        profile.idleHeadSway + poseAnchorX;
    scene.centerY = static_cast<float>(height) * (runtime.hold ? style.holdCenterYRatio : style.idleCenterYRatio) -
        (runtime.follow ? static_cast<float>(height) * style.followCenterYOffsetRatio : 0.0f) -
        (runtime.drag ? static_cast<float>(height) * style.dragCenterYOffsetRatio : 0.0f) -
        profile.stateLift - profile.breathLift + comboCenterYOffset + poseAnchorY;
    scene.facingSign = runtime.facingSign;
    scene.bodyTiltDeg = profile.scrollLean + profile.dragLean;
    scene.glowAlpha =
        style.glowBaseAlpha + glowBoost * style.glowActionAlphaScale + runtime.headTintAmount * style.glowHeadTintAlphaScale +
        glowAlphaBias;
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
        scene.centerX - metrics.bodyWidth * style.glowXOffsetRatio + glowStateXOffset,
        scene.centerY - metrics.bodyHeight * style.glowYOffsetRatio - profile.stateLift * style.glowStateLiftScale +
            glowStateYOffset,
        metrics.bodyWidth * style.glowWidthScale * glowScale * comboGlowStateScale,
        metrics.bodyHeight * style.glowHeightScale * glowScale * comboGlowStateScale);
    scene.shadowRect = Gdiplus::RectF(
        scene.centerX - metrics.bodyWidth * style.shadowXOffsetRatio * profile.shadowScale * shadowStateScale * comboShadowStateScale +
            shadowStateXOffset * runtime.facingSign + poseGroundingX,
        scene.centerY + metrics.bodyHeight * style.shadowYRatio + shadowStateYOffset + poseGroundingY,
        metrics.bodyWidth * style.shadowWidthScale * profile.shadowScale * shadowStateScale * comboShadowStateScale *
            poseGroundingScale *
            (1.0f - (profile.breathScale - 1.0f) * style.shadowBreathScale),
        metrics.bodyHeight * style.shadowHeightScale * shadowStateScale * comboShadowStateScale);
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
            profile.bodyForward * style.headBodyForwardScale + poseHeadX,
        scene.bodyRect.Y - metrics.headHeight * style.headYOffsetRatio - profile.actionIntensity * style.headActionLiftPx +
            profile.headNod + poseHeadY,
        metrics.headWidth,
        metrics.headHeight);
    scene.neckBridgeRect = Gdiplus::RectF(
        scene.centerX - metrics.bodyWidth * style.neckBridgeXOffsetRatio + profile.bodyForward * 0.08f,
        scene.headRect.GetBottom() - metrics.bodyHeight * style.neckBridgeYOffsetRatio,
        metrics.bodyWidth * style.neckBridgeWidthRatio,
        metrics.bodyHeight * style.neckBridgeHeightRatio);
    scene.leftHeadShoulderBridgeRect = Gdiplus::RectF(
        scene.centerX - metrics.bodyWidth * style.headShoulderBridgeXRatio,
        scene.headRect.GetBottom() - metrics.bodyHeight * style.headShoulderBridgeYRatio,
        metrics.bodyWidth * style.headShoulderBridgeWidthRatio,
        metrics.bodyHeight * style.headShoulderBridgeHeightRatio);
    scene.rightHeadShoulderBridgeRect = Gdiplus::RectF(
        scene.centerX + metrics.bodyWidth * style.headShoulderBridgeXRatio -
            metrics.bodyWidth * style.headShoulderBridgeWidthRatio,
        scene.headRect.GetBottom() - metrics.bodyHeight * style.headShoulderBridgeYRatio,
        metrics.bodyWidth * style.headShoulderBridgeWidthRatio,
        metrics.bodyHeight * style.headShoulderBridgeHeightRatio);
    scene.leftShoulderPatchRect = Gdiplus::RectF(
        scene.bodyRect.X + metrics.bodyWidth * style.shoulderPatchXOffsetRatio,
        scene.bodyRect.Y + metrics.bodyHeight * style.shoulderPatchYOffsetRatio,
        metrics.bodyWidth * style.shoulderPatchWidthRatio * skinTuning.shoulderPatchScale,
        metrics.bodyHeight * style.shoulderPatchHeightRatio * skinTuning.shoulderPatchScale);
    scene.rightShoulderPatchRect = Gdiplus::RectF(
        scene.bodyRect.GetRight() - metrics.bodyWidth *
            (style.shoulderPatchXOffsetRatio + style.shoulderPatchWidthRatio * skinTuning.shoulderPatchScale),
        scene.bodyRect.Y + metrics.bodyHeight * style.shoulderPatchYOffsetRatio,
        metrics.bodyWidth * style.shoulderPatchWidthRatio * skinTuning.shoulderPatchScale,
        metrics.bodyHeight * style.shoulderPatchHeightRatio * skinTuning.shoulderPatchScale);
    scene.leftHipPatchRect = Gdiplus::RectF(
        scene.bodyRect.X + metrics.bodyWidth * style.hipPatchXOffsetRatio,
        scene.bodyRect.GetBottom() - metrics.bodyHeight *
            (style.hipPatchYOffsetRatio + style.hipPatchHeightRatio * skinTuning.hipPatchScale),
        metrics.bodyWidth * style.hipPatchWidthRatio * skinTuning.hipPatchScale,
        metrics.bodyHeight * style.hipPatchHeightRatio * skinTuning.hipPatchScale);
    scene.rightHipPatchRect = Gdiplus::RectF(
        scene.bodyRect.GetRight() - metrics.bodyWidth *
            (style.hipPatchXOffsetRatio + style.hipPatchWidthRatio * skinTuning.hipPatchScale),
        scene.bodyRect.GetBottom() - metrics.bodyHeight *
            (style.hipPatchYOffsetRatio + style.hipPatchHeightRatio * skinTuning.hipPatchScale),
        metrics.bodyWidth * style.hipPatchWidthRatio * skinTuning.hipPatchScale,
        metrics.bodyHeight * style.hipPatchHeightRatio * skinTuning.hipPatchScale);
    scene.bellyContourRect = Gdiplus::RectF(
        scene.centerX - metrics.bodyWidth * style.bellyContourWidthRatio * 0.5f,
        scene.bodyRect.Y + metrics.bodyHeight * style.bellyContourYRatio,
        metrics.bodyWidth * style.bellyContourWidthRatio,
        metrics.bodyHeight * style.bellyContourHeightRatio);
    scene.sternumContourRect = Gdiplus::RectF(
        scene.centerX - metrics.bodyWidth * style.sternumContourWidthRatio * 0.5f,
        scene.bodyRect.Y + metrics.bodyHeight * style.sternumContourYRatio,
        metrics.bodyWidth * style.sternumContourWidthRatio,
        metrics.bodyHeight * style.sternumContourHeightRatio);
    scene.upperTorsoContourRect = Gdiplus::RectF(
        scene.centerX - metrics.bodyWidth * style.upperTorsoContourWidthRatio * 0.5f,
        scene.bodyRect.Y + metrics.bodyHeight * style.upperTorsoContourYRatio,
        metrics.bodyWidth * style.upperTorsoContourWidthRatio,
        metrics.bodyHeight * style.upperTorsoContourHeightRatio);
    scene.leftTorsoCadenceBridgeRect = Gdiplus::RectF(
        scene.bodyRect.X + metrics.bodyWidth * style.torsoCadenceBridgeXRatio,
        scene.bodyRect.Y + metrics.bodyHeight * style.torsoCadenceBridgeYRatio,
        metrics.bodyWidth * style.torsoCadenceBridgeWidthRatio,
        metrics.bodyHeight * style.torsoCadenceBridgeHeightRatio);
    scene.rightTorsoCadenceBridgeRect = Gdiplus::RectF(
        scene.bodyRect.GetRight() - metrics.bodyWidth * (style.torsoCadenceBridgeXRatio + style.torsoCadenceBridgeWidthRatio),
        scene.bodyRect.Y + metrics.bodyHeight * style.torsoCadenceBridgeYRatio,
        metrics.bodyWidth * style.torsoCadenceBridgeWidthRatio,
        metrics.bodyHeight * style.torsoCadenceBridgeHeightRatio);
    scene.leftBackContourRect = Gdiplus::RectF(
        scene.bodyRect.X + metrics.bodyWidth * style.backContourXRatio,
        scene.bodyRect.Y + metrics.bodyHeight * style.backContourYRatio,
        metrics.bodyWidth * style.backContourWidthRatio,
        metrics.bodyHeight * style.backContourHeightRatio);
    scene.rightBackContourRect = Gdiplus::RectF(
        scene.bodyRect.GetRight() - metrics.bodyWidth * (style.backContourXRatio + style.backContourWidthRatio),
        scene.bodyRect.Y + metrics.bodyHeight * style.backContourYRatio,
        metrics.bodyWidth * style.backContourWidthRatio,
        metrics.bodyHeight * style.backContourHeightRatio);
    scene.leftFlankContourRect = Gdiplus::RectF(
        scene.bodyRect.X + metrics.bodyWidth * style.flankContourXRatio,
        scene.bodyRect.Y + metrics.bodyHeight * style.flankContourYRatio,
        metrics.bodyWidth * style.flankContourWidthRatio,
        metrics.bodyHeight * style.flankContourHeightRatio);
    scene.rightFlankContourRect = Gdiplus::RectF(
        scene.bodyRect.GetRight() - metrics.bodyWidth * (style.flankContourXRatio + style.flankContourWidthRatio),
        scene.bodyRect.Y + metrics.bodyHeight * style.flankContourYRatio,
        metrics.bodyWidth * style.flankContourWidthRatio,
        metrics.bodyHeight * style.flankContourHeightRatio);
    const bool leftRearQuarterFront = scene.facingSign < 0.0f;
    const bool rightRearQuarterFront = !leftRearQuarterFront;
    const float leftTailHaunchWidthScale =
        leftRearQuarterFront ? style.frontTailHaunchBridgeWidthScale : style.rearTailHaunchBridgeWidthScale;
    const float rightTailHaunchWidthScale =
        rightRearQuarterFront ? style.frontTailHaunchBridgeWidthScale : style.rearTailHaunchBridgeWidthScale;
    scene.leftTailHaunchBridgeRect = Gdiplus::RectF(
        scene.bodyRect.X + metrics.bodyWidth * style.tailHaunchBridgeXRatio,
        scene.bodyRect.Y + metrics.bodyHeight * style.tailHaunchBridgeYRatio,
        metrics.bodyWidth * style.tailHaunchBridgeWidthRatio * leftTailHaunchWidthScale,
        metrics.bodyHeight * style.tailHaunchBridgeHeightRatio);
    scene.rightTailHaunchBridgeRect = Gdiplus::RectF(
        scene.bodyRect.GetRight() - metrics.bodyWidth *
            (style.tailHaunchBridgeXRatio + style.tailHaunchBridgeWidthRatio * rightTailHaunchWidthScale),
        scene.bodyRect.Y + metrics.bodyHeight * style.tailHaunchBridgeYRatio,
        metrics.bodyWidth * style.tailHaunchBridgeWidthRatio * rightTailHaunchWidthScale,
        metrics.bodyHeight * style.tailHaunchBridgeHeightRatio);
    scene.pedestalRect = Gdiplus::RectF(
        scene.centerX - metrics.bodyWidth * style.pedestalXOffsetRatio * pedestalStateScale * comboPedestalStateScale +
            pedestalStateXOffset * runtime.facingSign + poseGroundingX * 0.8f,
        scene.shadowRect.GetBottom() - metrics.bodyHeight * style.pedestalYOffsetRatio + pedestalStateYOffset +
            poseGroundingY * 0.6f,
        metrics.bodyWidth * style.pedestalWidthScale * pedestalStateScale * comboPedestalStateScale *
            poseGroundingScale,
        metrics.bodyHeight * style.pedestalHeightScale * pedestalStateScale * comboPedestalStateScale);

    return metrics;
}

} // namespace mousefx::windows

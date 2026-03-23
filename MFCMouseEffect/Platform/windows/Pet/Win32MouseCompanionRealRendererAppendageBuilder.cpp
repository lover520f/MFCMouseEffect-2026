#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppendageBuilder.h"

#include <algorithm>

namespace mousefx::windows {

namespace {

Gdiplus::RectF BuildPadRect(
    const Gdiplus::RectF& limbRect,
    const Win32MouseCompanionRealRendererStyleProfile& style) {
    const float padWidth = limbRect.Width * style.pawPadWidthRatio;
    const float padHeight = limbRect.Height * style.pawPadHeightRatio;
    return Gdiplus::RectF(
        limbRect.X + (limbRect.Width - padWidth) * 0.5f,
        limbRect.GetBottom() - padHeight - limbRect.Height * style.pawPadBottomInsetRatio,
        padWidth,
        padHeight);
}

float ClampEarSpread(float value, float headWidth) {
    return std::clamp(value, -headWidth * 0.18f, headWidth * 0.18f);
}

float ClampEarSwing(float value, float headWidth) {
    return std::clamp(value, -headWidth * 0.10f, headWidth * 0.10f);
}

float ClampEarTipOffset(float value, float headWidth) {
    return std::clamp(value, headWidth * 0.12f, headWidth * 0.34f);
}

std::array<Gdiplus::PointF, 4> ScaleEarPolygon(
    const std::array<Gdiplus::PointF, 4>& points,
    float scale) {
    if (scale == 1.0f) {
        return points;
    }
    std::array<Gdiplus::PointF, 4> scaled = points;
    const Gdiplus::PointF anchor = points[0];
    for (size_t i = 1; i < scaled.size(); ++i) {
        scaled[i].X = anchor.X + (scaled[i].X - anchor.X) * scale;
        scaled[i].Y = anchor.Y + (scaled[i].Y - anchor.Y) * scale;
    }
    return scaled;
}

} // namespace

void BuildWin32MouseCompanionRealRendererAppendages(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererMotionProfile& profile,
    const Win32MouseCompanionRealRendererStyleProfile& style,
    const Win32MouseCompanionRealRendererLayoutMetrics& metrics,
    Win32MouseCompanionRealRendererScene& scene) {
    const float poseAdapterInfluence = runtime.poseAdapterProfile.influence;
    const float poseEarLift = runtime.leftEarPose
        ? runtime.leftEarPose->position[1] * style.leftEarPoseLiftScale * poseAdapterInfluence
        : 0.0f;
    const float poseEarSpread = runtime.leftEarPose
        ? -runtime.leftEarPose->position[0] * style.leftEarPoseSpreadScale *
              poseAdapterInfluence
        : 0.0f;
    const float poseRightEarLift = runtime.rightEarPose
        ? runtime.rightEarPose->position[1] * style.rightEarPoseLiftScale *
              poseAdapterInfluence
        : 0.0f;
    const float poseRightEarSpread = runtime.rightEarPose
        ? runtime.rightEarPose->position[0] * style.rightEarPoseSpreadScale *
              poseAdapterInfluence
        : 0.0f;
    const float poseLeftHandLift = runtime.leftHandPose
        ? -runtime.leftHandPose->position[1] * style.leftHandPoseLiftScale *
              poseAdapterInfluence
        : 0.0f;
    const float poseRightHandLift = runtime.rightHandPose
        ? -runtime.rightHandPose->position[1] * style.rightHandPoseLiftScale *
              poseAdapterInfluence
        : 0.0f;
    const float poseLeftLegShift = runtime.leftLegPose
        ? runtime.leftLegPose->position[0] * style.leftLegPoseShiftScale *
              poseAdapterInfluence
        : 0.0f;
    const float poseRightLegShift = runtime.rightLegPose
        ? runtime.rightLegPose->position[0] * style.rightLegPoseShiftScale *
              poseAdapterInfluence
        : 0.0f;
    const auto appearanceSemantics =
        BuildWin32MouseCompanionRealRendererAppearanceSemantics(runtime, style);
    const auto& skinTuning = appearanceSemantics.appendage;
    const auto& assetTransform = runtime.assetNodeTransformProfile;
    const float handReachBias = runtime.follow ? metrics.bodyWidth * style.followHandReachRatio * skinTuning.followHandReachScale
        : runtime.hold                        ? -metrics.bodyWidth * style.holdHandTuckRatio
        : runtime.drag                        ? metrics.bodyWidth * style.dragHandReachRatio * skinTuning.dragHandReachScale
                                              : 0.0f;
    const float legStanceBias = runtime.follow ? metrics.bodyWidth * style.followLegStanceRatio * skinTuning.followLegStanceScale
        : runtime.hold                        ? -metrics.bodyWidth * style.holdLegStanceRatio * skinTuning.holdLegStanceScale
                                              : 0.0f;
    const float tailWidthScale = runtime.follow ? style.followTailWidthScale * skinTuning.followTailWidthScale
        : runtime.hold                         ? style.holdTailWidthScale
                                               : 1.0f;
    const float tailHeightScale = runtime.scroll ? style.scrollTailHeightScale * skinTuning.scrollTailHeightScale : 1.0f;
    const float tailDirection = scene.facingSign >= 0.0f ? 1.0f : -1.0f;
    const float earSpreadScale = runtime.follow ? style.followEarSpreadScale * skinTuning.followEarSpreadScale
        : runtime.hold                         ? style.holdEarSpreadScale
                                               : 1.0f;
    const float clickEarLift = runtime.click ? style.clickEarLiftPx * skinTuning.clickEarLiftScale : 0.0f;

    float comboHandReachBias = 0.0f;
    float comboLegStanceBias = 0.0f;
    float comboTailLiftBias = 0.0f;
    float comboEarLiftBias = 0.0f;
    switch (appearanceSemantics.comboPreset) {
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Dreamy:
        if (runtime.follow) {
            comboTailLiftBias = -1.6f;
            comboEarLiftBias = 0.9f;
        }
        break;
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Agile:
        if (runtime.drag) {
            comboHandReachBias = metrics.bodyWidth * 0.020f;
            comboLegStanceBias = metrics.bodyWidth * 0.012f;
            comboTailLiftBias = -0.8f;
            comboEarLiftBias = 0.5f;
        } else if (runtime.follow) {
            comboLegStanceBias = metrics.bodyWidth * 0.008f;
        }
        break;
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Charming:
        if (runtime.hold) {
            comboHandReachBias = -metrics.bodyWidth * 0.012f;
            comboLegStanceBias = -metrics.bodyWidth * 0.010f;
            comboEarLiftBias = 1.2f;
        } else if (runtime.click) {
            comboEarLiftBias = 1.0f;
        }
        break;
    case Win32MouseCompanionRealRendererAppearanceComboPreset::None:
        break;
    }

    scene.tailRect = Gdiplus::RectF(
        scene.bodyRect.GetRight() - metrics.bodyWidth * style.tailXOffsetRatio -
            scene.facingSign * metrics.bodyWidth * style.tailFacingRatio +
            profile.idleTailSway * scene.facingSign + profile.tailSwing,
        scene.centerY - metrics.bodyHeight * style.tailYOffsetRatio + profile.tailLift + comboTailLiftBias,
        metrics.bodyWidth * style.tailWidthRatio * tailWidthScale * skinTuning.tailWidthScale,
        metrics.bodyHeight * style.tailHeightRatio * tailHeightScale * skinTuning.tailHeightScale);
    scene.tailRootCuffRect = Gdiplus::RectF(
        scene.tailRect.X - scene.tailRect.Width * style.tailRootCuffXOffsetRatio +
            tailDirection * style.tailRootCuffFacingXOffsetPx,
        scene.tailRect.Y + scene.tailRect.Height * style.tailRootCuffYOffsetRatio,
        scene.tailRect.Width * style.tailRootCuffWidthRatio * style.tailRootCuffFacingWidthScale,
        scene.tailRect.Height * style.tailRootCuffHeightRatio * style.tailRootCuffFacingHeightScale);
    scene.tailBridgeRect = Gdiplus::RectF(
        scene.tailRect.X + scene.tailRect.Width * style.tailBridgeXOffsetRatio +
            tailDirection * style.tailBridgeFacingXOffsetPx,
        scene.tailRect.Y + scene.tailRect.Height * style.tailBridgeYOffsetRatio +
            style.tailBridgeFacingYOffsetPx,
        scene.tailRect.Width * style.tailBridgeWidthRatio * style.tailBridgeFacingWidthScale,
        scene.tailRect.Height * style.tailBridgeHeightRatio * style.tailBridgeFacingHeightScale);
    scene.tailMidContourRect = Gdiplus::RectF(
        scene.tailRect.X + scene.tailRect.Width * style.tailMidContourXOffsetRatio +
            tailDirection * style.tailMidContourFacingXOffsetPx,
        scene.tailRect.Y + scene.tailRect.Height * style.tailMidContourYOffsetRatio +
            style.tailMidContourFacingYOffsetPx,
        scene.tailRect.Width * style.tailMidContourWidthRatio * style.tailMidContourFacingWidthScale *
            style.tailMidContourTipwardWidthScale,
        scene.tailRect.Height * style.tailMidContourHeightRatio * style.tailMidContourFacingHeightScale *
            style.tailMidContourTipwardHeightScale);
    scene.tailTipBridgeRect = Gdiplus::RectF(
        scene.tailRect.X + scene.tailRect.Width * style.tailTipBridgeXOffsetRatio +
            tailDirection * style.tailTipBridgeFacingXOffsetPx,
        scene.tailRect.Y + scene.tailRect.Height * style.tailTipBridgeYOffsetRatio +
            style.tailTipBridgeFacingYOffsetPx,
        scene.tailRect.Width * style.tailTipBridgeWidthRatio * style.tailTipBridgeFacingWidthScale,
        scene.tailRect.Height * style.tailTipBridgeHeightRatio * style.tailTipBridgeFacingHeightScale);
    scene.tailTipRect = Gdiplus::RectF(
        scene.tailRect.GetRight() - scene.tailRect.Width *
            (style.tailTipXOffsetRatio + style.tailTipWidthRatio * style.tailTipFacingWidthScale) +
            tailDirection * style.tailTipFacingXOffsetPx,
        scene.tailRect.Y + scene.tailRect.Height * style.tailTipYOffsetRatio - profile.tailLift * 0.08f +
            style.tailTipFacingYOffsetPx,
        scene.tailRect.Width * style.tailTipWidthRatio * style.tailTipFacingWidthScale,
        scene.tailRect.Height * style.tailTipHeightRatio * style.tailTipFacingHeightScale);

    const float earBaseY = scene.headRect.Y + scene.headRect.Height * style.earBaseYOffsetRatio;
    const float earTipY = scene.headRect.Y - scene.headRect.Height * style.earTipYOffsetRatio - profile.earLift - poseEarLift -
        profile.idleEarCadence - clickEarLift - comboEarLiftBias;
    const float rightEarTipY = scene.headRect.Y - scene.headRect.Height * style.earTipYOffsetRatio - profile.earLift -
        poseRightEarLift + profile.idleEarCadence * style.earIdleCadenceRatio - clickEarLift - comboEarLiftBias;
    const float earBaseOffset = scene.headRect.Width * style.earBaseOffsetRatio;
    const float leftEarSwing = ClampEarSwing(-profile.earSwing, scene.headRect.Width);
    const float rightEarSwing = ClampEarSwing(profile.earSwing, scene.headRect.Width);
    const bool leftEarFront = scene.facingSign < 0.0f;
    const bool rightEarFront = !leftEarFront;
    const float leftEarScale = (leftEarFront ? style.frontEarScale : style.rearEarScale) * skinTuning.earScale;
    const float rightEarScale = (rightEarFront ? style.frontEarScale : style.rearEarScale) * skinTuning.earScale;
    const float leftEarOuterSpreadScale = leftEarFront ? style.frontEarOuterSpreadScale : style.rearEarOuterSpreadScale;
    const float rightEarOuterSpreadScale = rightEarFront ? style.frontEarOuterSpreadScale : style.rearEarOuterSpreadScale;
    const float leftEarTipOffsetScale = leftEarFront ? style.frontEarTipOffsetScale : style.rearEarTipOffsetScale;
    const float rightEarTipOffsetScale = rightEarFront ? style.frontEarTipOffsetScale : style.rearEarTipOffsetScale;
    const float leftEarRootCuffWidthScale = leftEarFront ? style.frontEarRootCuffWidthScale : style.rearEarRootCuffWidthScale;
    const float rightEarRootCuffWidthScale = rightEarFront ? style.frontEarRootCuffWidthScale : style.rearEarRootCuffWidthScale;
    const float leftEarRootCuffHeightScale = leftEarFront ? style.frontEarRootCuffHeightScale : style.rearEarRootCuffHeightScale;
    const float rightEarRootCuffHeightScale = rightEarFront ? style.frontEarRootCuffHeightScale : style.rearEarRootCuffHeightScale;
    const float leftEarRootCuffXOffset = leftEarFront ? -style.frontEarRootCuffXOffsetPx : -style.rearEarRootCuffXOffsetPx;
    const float rightEarRootCuffXOffset = rightEarFront ? style.frontEarRootCuffXOffsetPx : style.rearEarRootCuffXOffsetPx;
    const float leftEarRootCuffYOffset = leftEarFront ? style.frontEarRootCuffYOffsetPx : style.rearEarRootCuffYOffsetPx;
    const float rightEarRootCuffYOffset = rightEarFront ? style.frontEarRootCuffYOffsetPx : style.rearEarRootCuffYOffsetPx;
    const float leftEarOcclusionCapWidthScale =
        leftEarFront ? style.frontEarOcclusionCapWidthScale : style.rearEarOcclusionCapWidthScale;
    const float rightEarOcclusionCapWidthScale =
        rightEarFront ? style.frontEarOcclusionCapWidthScale : style.rearEarOcclusionCapWidthScale;
    const float leftEarOcclusionCapHeightScale =
        leftEarFront ? style.frontEarOcclusionCapHeightScale : style.rearEarOcclusionCapHeightScale;
    const float rightEarOcclusionCapHeightScale =
        rightEarFront ? style.frontEarOcclusionCapHeightScale : style.rearEarOcclusionCapHeightScale;
    const float leftEarOcclusionCapXOffset =
        leftEarFront ? -style.frontEarOcclusionCapXOffsetPx : -style.rearEarOcclusionCapXOffsetPx;
    const float rightEarOcclusionCapXOffset =
        rightEarFront ? style.frontEarOcclusionCapXOffsetPx : style.rearEarOcclusionCapXOffsetPx;
    const float leftEarOcclusionCapYOffset =
        leftEarFront ? style.frontEarOcclusionCapYOffsetPx : style.rearEarOcclusionCapYOffsetPx;
    const float rightEarOcclusionCapYOffset =
        rightEarFront ? style.frontEarOcclusionCapYOffsetPx : style.rearEarOcclusionCapYOffsetPx;
    const float clampedLeftPoseSpread = ClampEarSpread(poseEarSpread, scene.headRect.Width);
    const float clampedRightPoseSpread = ClampEarSpread(poseRightEarSpread, scene.headRect.Width);
    const float clampedEarSpreadPulse =
        ClampEarSpread(profile.earSpreadPulse * earSpreadScale, scene.headRect.Width);
    const float clampedIdleHeadSway = ClampEarSwing(profile.idleHeadSway, scene.headRect.Width);
    const float leftOuterX = scene.centerX - earBaseOffset -
        scene.headRect.Width * style.earOuterSpreadRatio * leftEarOuterSpreadScale +
        clampedLeftPoseSpread + leftEarSwing - clampedEarSpreadPulse - clampedIdleHeadSway;
    const float rightOuterX = scene.centerX + earBaseOffset +
        scene.headRect.Width * style.earOuterSpreadRatio * rightEarOuterSpreadScale +
        clampedRightPoseSpread - rightEarSwing + clampedEarSpreadPulse + clampedIdleHeadSway;
    const float leftTipOffset = ClampEarTipOffset(
        scene.headRect.Width * style.earTipSpreadRatio * earSpreadScale * leftEarTipOffsetScale -
            clampedLeftPoseSpread + leftEarSwing * style.leftEarSwingScale + clampedEarSpreadPulse + clampedIdleHeadSway,
        scene.headRect.Width);
    const float rightTipOffset = ClampEarTipOffset(
        scene.headRect.Width * style.earTipSpreadRatio * earSpreadScale * rightEarTipOffsetScale +
            clampedRightPoseSpread - rightEarSwing * style.rightEarSwingScale + clampedEarSpreadPulse + clampedIdleHeadSway,
        scene.headRect.Width);
    const float earRootCuffWidth = scene.headRect.Width * style.earRootCuffWidthRatio;
    const float earRootCuffHeight = scene.headRect.Height * style.earRootCuffHeightRatio;
    const float earOcclusionCapWidth = scene.headRect.Width * style.earOcclusionCapWidthRatio;
    const float earOcclusionCapHeight = scene.headRect.Height * style.earOcclusionCapHeightRatio;
    scene.leftEarRootCuffRect = Gdiplus::RectF(
        scene.centerX - earBaseOffset - earRootCuffWidth * leftEarRootCuffWidthScale * 0.55f + leftEarRootCuffXOffset,
        earBaseY - earRootCuffHeight * leftEarRootCuffHeightScale * (0.5f + style.earRootCuffYOffsetRatio) +
            leftEarRootCuffYOffset,
        earRootCuffWidth * leftEarRootCuffWidthScale,
        earRootCuffHeight * leftEarRootCuffHeightScale);
    scene.rightEarRootCuffRect = Gdiplus::RectF(
        scene.centerX + earBaseOffset - earRootCuffWidth * rightEarRootCuffWidthScale * 0.45f + rightEarRootCuffXOffset,
        earBaseY - earRootCuffHeight * rightEarRootCuffHeightScale * (0.5f + style.earRootCuffYOffsetRatio) +
            rightEarRootCuffYOffset,
        earRootCuffWidth * rightEarRootCuffWidthScale,
        earRootCuffHeight * rightEarRootCuffHeightScale);
    scene.leftEarOcclusionCapRect = Gdiplus::RectF(
        scene.centerX - earBaseOffset -
            earOcclusionCapWidth * leftEarOcclusionCapWidthScale * (0.5f + style.earOcclusionCapInsetRatio) +
            leftEarOcclusionCapXOffset,
        scene.headRect.Y + scene.headRect.Height * style.earOcclusionCapYOffsetRatio + leftEarOcclusionCapYOffset,
        earOcclusionCapWidth * leftEarOcclusionCapWidthScale,
        earOcclusionCapHeight * leftEarOcclusionCapHeightScale);
    scene.rightEarOcclusionCapRect = Gdiplus::RectF(
        scene.centerX + earBaseOffset -
            earOcclusionCapWidth * rightEarOcclusionCapWidthScale * (0.5f - style.earOcclusionCapInsetRatio) +
            rightEarOcclusionCapXOffset,
        scene.headRect.Y + scene.headRect.Height * style.earOcclusionCapYOffsetRatio + rightEarOcclusionCapYOffset,
        earOcclusionCapWidth * rightEarOcclusionCapWidthScale,
        earOcclusionCapHeight * rightEarOcclusionCapHeightScale);
    const std::array<Gdiplus::PointF, 4> leftEarPoints = {{
        Gdiplus::PointF(scene.centerX - earBaseOffset, earBaseY),
        Gdiplus::PointF(
            leftOuterX,
            scene.headRect.Y + scene.headRect.Height * style.earBaseYRatio),
        Gdiplus::PointF(
            scene.centerX - leftTipOffset,
            earTipY),
        Gdiplus::PointF(
            scene.centerX - scene.headRect.Width * style.earRootInsetRatio,
            scene.headRect.Y + scene.headRect.Height * style.earRootYRatio),
    }};
    const std::array<Gdiplus::PointF, 4> rightEarPoints = {{
        Gdiplus::PointF(scene.centerX + earBaseOffset, earBaseY),
        Gdiplus::PointF(
            rightOuterX,
            scene.headRect.Y + scene.headRect.Height * style.earBaseYRatio),
        Gdiplus::PointF(
            scene.centerX + rightTipOffset,
            rightEarTipY),
        Gdiplus::PointF(
            scene.centerX + scene.headRect.Width * style.earRootInsetRatio,
            scene.headRect.Y + scene.headRect.Height * style.earRootYRatio),
    }};
    scene.leftEar = ScaleEarPolygon(leftEarPoints, leftEarScale);
    scene.rightEar = ScaleEarPolygon(rightEarPoints, rightEarScale);

    scene.leftHandRect = Gdiplus::RectF(
        scene.bodyRect.X - metrics.bodyWidth * style.handXOffsetRatio - profile.handSwing - handReachBias - comboHandReachBias,
        scene.centerY - metrics.bodyHeight * style.handYOffsetRatio - profile.handLift - poseLeftHandLift + profile.headNod * style.handHeadNodRatio -
            profile.idleHandFloat,
        metrics.bodyWidth * style.handWidthRatio,
        metrics.bodyHeight * style.handHeightRatio);
    scene.rightHandRect = Gdiplus::RectF(
        scene.bodyRect.GetRight() - metrics.bodyWidth * style.tailXOffsetRatio + profile.handSwing + handReachBias + comboHandReachBias,
        scene.centerY - metrics.bodyHeight * style.handYOffsetRatio - profile.handLift - poseRightHandLift + profile.headNod * style.handHeadNodRatio +
            profile.idleHandFloat,
        metrics.bodyWidth * style.handWidthRatio,
        metrics.bodyHeight * style.handHeightRatio);
    scene.leftHandRootCuffRect = Gdiplus::RectF(
        scene.leftHandRect.X + scene.leftHandRect.Width * style.handRootCuffInsetRatio,
        scene.leftHandRect.Y - scene.leftHandRect.Height * style.handRootCuffHeightRatio * 0.35f,
        scene.leftHandRect.Width * style.handRootCuffWidthRatio,
        scene.leftHandRect.Height * style.handRootCuffHeightRatio);
    scene.rightHandRootCuffRect = Gdiplus::RectF(
        scene.rightHandRect.GetRight() - scene.rightHandRect.Width * (style.handRootCuffInsetRatio + style.handRootCuffWidthRatio),
        scene.rightHandRect.Y - scene.rightHandRect.Height * style.handRootCuffHeightRatio * 0.35f,
        scene.rightHandRect.Width * style.handRootCuffWidthRatio,
        scene.rightHandRect.Height * style.handRootCuffHeightRatio);
    scene.leftHandSilhouetteBridgeRect = Gdiplus::RectF(
        scene.leftHandRect.GetRight() - scene.leftHandRect.Width * style.handSilhouetteBridgeWidthRatio,
        scene.leftHandRootCuffRect.Y + scene.leftHandRootCuffRect.Height * 0.15f,
        scene.leftHandRect.Width * style.handSilhouetteBridgeWidthRatio,
        scene.leftHandRect.Height * style.handSilhouetteBridgeHeightRatio);
    scene.rightHandSilhouetteBridgeRect = Gdiplus::RectF(
        scene.rightHandRect.X,
        scene.rightHandRootCuffRect.Y + scene.rightHandRootCuffRect.Height * 0.15f,
        scene.rightHandRect.Width * style.handSilhouetteBridgeWidthRatio,
        scene.rightHandRect.Height * style.handSilhouetteBridgeHeightRatio);
    scene.leftHandCadenceBridgeRect = Gdiplus::RectF(
        scene.leftHandSilhouetteBridgeRect.GetRight() - scene.leftHandRect.Width * style.handCadenceBridgeWidthRatio,
        scene.leftHandRootCuffRect.Y + scene.leftHandRootCuffRect.Height * 0.05f,
        scene.leftHandRect.Width * style.handCadenceBridgeWidthRatio,
        scene.leftHandRect.Height * style.handCadenceBridgeHeightRatio);
    scene.rightHandCadenceBridgeRect = Gdiplus::RectF(
        scene.rightHandSilhouetteBridgeRect.X,
        scene.rightHandRootCuffRect.Y + scene.rightHandRootCuffRect.Height * 0.05f,
        scene.rightHandRect.Width * style.handCadenceBridgeWidthRatio,
        scene.rightHandRect.Height * style.handCadenceBridgeHeightRatio);
    scene.leftLegRect = Gdiplus::RectF(
        scene.centerX - metrics.bodyWidth * style.legLeftXRatio - profile.legStride * style.legStrideRatio + poseLeftLegShift - legStanceBias - comboLegStanceBias,
        scene.bodyRect.GetBottom() - metrics.bodyHeight * style.legYOffsetRatio + (runtime.follow ? style.followLegPhasePx : 0.0f) -
            profile.legLift,
        metrics.bodyWidth * style.legWidthRatio,
        metrics.bodyHeight * style.legHeightRatio);
    scene.rightLegRect = Gdiplus::RectF(
        scene.centerX + metrics.bodyWidth * style.legRightXRatio + profile.legStride * style.legStrideRatio + poseRightLegShift + legStanceBias + comboLegStanceBias,
        scene.bodyRect.GetBottom() - metrics.bodyHeight * style.legYOffsetRatio - (runtime.follow ? style.followLegPhasePx : 0.0f) +
            profile.legLift * 0.45f,
        metrics.bodyWidth * style.legWidthRatio,
        metrics.bodyHeight * style.legHeightRatio);
    scene.leftLegRootCuffRect = Gdiplus::RectF(
        scene.leftLegRect.X + scene.leftLegRect.Width * style.legRootCuffInsetRatio,
        scene.leftLegRect.Y - scene.leftLegRect.Height * style.legRootCuffHeightRatio * 0.30f,
        scene.leftLegRect.Width * style.legRootCuffWidthRatio,
        scene.leftLegRect.Height * style.legRootCuffHeightRatio);
    scene.rightLegRootCuffRect = Gdiplus::RectF(
        scene.rightLegRect.GetRight() - scene.rightLegRect.Width * (style.legRootCuffInsetRatio + style.legRootCuffWidthRatio),
        scene.rightLegRect.Y - scene.rightLegRect.Height * style.legRootCuffHeightRatio * 0.30f,
        scene.rightLegRect.Width * style.legRootCuffWidthRatio,
        scene.rightLegRect.Height * style.legRootCuffHeightRatio);
    scene.leftLegSilhouetteBridgeRect = Gdiplus::RectF(
        scene.leftLegRect.GetRight() - scene.leftLegRect.Width * style.legSilhouetteBridgeWidthRatio,
        scene.leftLegRootCuffRect.Y + scene.leftLegRootCuffRect.Height * 0.10f,
        scene.leftLegRect.Width * style.legSilhouetteBridgeWidthRatio,
        scene.leftLegRect.Height * style.legSilhouetteBridgeHeightRatio);
    scene.rightLegSilhouetteBridgeRect = Gdiplus::RectF(
        scene.rightLegRect.X,
        scene.rightLegRootCuffRect.Y + scene.rightLegRootCuffRect.Height * 0.10f,
        scene.rightLegRect.Width * style.legSilhouetteBridgeWidthRatio,
        scene.rightLegRect.Height * style.legSilhouetteBridgeHeightRatio);
    scene.leftLegCadenceBridgeRect = Gdiplus::RectF(
        scene.leftLegSilhouetteBridgeRect.GetRight() - scene.leftLegRect.Width * style.legCadenceBridgeWidthRatio,
        scene.leftLegRootCuffRect.Y + scene.leftLegRootCuffRect.Height * 0.02f,
        scene.leftLegRect.Width * style.legCadenceBridgeWidthRatio,
        scene.leftLegRect.Height * style.legCadenceBridgeHeightRatio);
    scene.rightLegCadenceBridgeRect = Gdiplus::RectF(
        scene.rightLegSilhouetteBridgeRect.X,
        scene.rightLegRootCuffRect.Y + scene.rightLegRootCuffRect.Height * 0.02f,
        scene.rightLegRect.Width * style.legCadenceBridgeWidthRatio,
        scene.rightLegRect.Height * style.legCadenceBridgeHeightRatio);
    scene.leftHandPadRect = BuildPadRect(scene.leftHandRect, style);
    scene.rightHandPadRect = BuildPadRect(scene.rightHandRect, style);
    scene.leftLegPadRect = BuildPadRect(scene.leftLegRect, style);
    scene.rightLegPadRect = BuildPadRect(scene.rightLegRect, style);
    const float appendageMinX = std::min(
        std::min(scene.leftHandRect.X, scene.rightHandRect.X),
        std::min(scene.leftLegRect.X, scene.rightLegRect.X));
    const float appendageMaxX = std::max(
        std::max(scene.leftHandRect.GetRight(), scene.rightHandRect.GetRight()),
        std::max(scene.leftLegRect.GetRight(), scene.rightLegRect.GetRight()));
    const float appendageMinY = std::min(
        std::min(scene.leftHandRect.Y, scene.rightHandRect.Y),
        std::min(scene.leftLegRect.Y, scene.rightLegRect.Y));
    const float appendageMaxY = std::max(
        std::max(scene.leftHandRect.GetBottom(), scene.rightHandRect.GetBottom()),
        std::max(scene.leftLegRect.GetBottom(), scene.rightLegRect.GetBottom()));
    scene.appendageAnchor = Gdiplus::PointF(
        (appendageMinX + appendageMaxX) * 0.5f,
        (appendageMinY + appendageMaxY) * 0.5f);
    scene.appendageAnchorScale = runtime.assetNodeParentSpaceProfile.appendageEntry.resolved
        ? runtime.assetNodeParentSpaceProfile.appendageEntry.parentSpaceScale
        : 1.0f;
}

} // namespace mousefx::windows

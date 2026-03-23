#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererFaceBuilder.h"

#include <algorithm>

namespace mousefx::windows {
namespace {

void SetWhiskerPair(
    float anchorX,
    float anchorY,
    float direction,
    float length,
    float verticalOffset,
    float spread,
    float tilt,
    Gdiplus::PointF& start,
    Gdiplus::PointF& end) {
    start = Gdiplus::PointF(anchorX, anchorY + verticalOffset);
    end = Gdiplus::PointF(
        anchorX + direction * length,
        anchorY + verticalOffset + verticalOffset * spread + tilt);
}

} // namespace

void BuildWin32MouseCompanionRealRendererFace(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererMotionProfile& profile,
    const Win32MouseCompanionRealRendererStyleProfile& style,
    Win32MouseCompanionRealRendererScene& scene) {
    const auto appearanceSemantics =
        BuildWin32MouseCompanionRealRendererAppearanceSemantics(runtime, style);
    const auto& skinTuning = appearanceSemantics.face;
    const auto& nodeAdapter = runtime.modelNodeAdapterProfile;
    const float poseFaceYOffset =
        nodeAdapter.faceChannel.offsetY * nodeAdapter.faceChannel.influence * scene.headRect.Height;
    const float poseFaceXOffset =
        nodeAdapter.faceChannel.offsetX * nodeAdapter.faceChannel.influence * scene.headRect.Width;
    const float poseWhiskerBias = nodeAdapter.whiskerBias;
    const float poseBlushLift = nodeAdapter.blushLift;
    const float eyeH = std::max(3.0f, scene.headRect.Height * style.eyeHeightRatio * profile.eyeOpen);
    const float pupilH = std::max(1.2f, eyeH * style.pupilHeightRatio);
    const float pupilOffsetX = profile.pupilFocusX * style.pupilFocusXScale * skinTuning.pupilFocusScale;
    const float pupilOffsetY = profile.pupilFocusY * std::max(0.6f, eyeH * style.pupilFocusYScale) * skinTuning.pupilFocusScale;
    scene.leftEyeRect = Gdiplus::RectF(
        scene.centerX - scene.headRect.Width * style.eyeLeftXRatio + poseFaceXOffset,
        scene.headRect.Y + scene.headRect.Height * style.eyeYRatio + poseFaceYOffset,
        style.eyeWidthPx,
        eyeH);
    scene.rightEyeRect = Gdiplus::RectF(
        scene.centerX + scene.headRect.Width * style.eyeRightXRatio - style.eyeWidthPx + poseFaceXOffset,
        scene.headRect.Y + scene.headRect.Height * style.eyeYRatio + poseFaceYOffset,
        style.eyeWidthPx,
        eyeH);
    scene.leftPupilRect = Gdiplus::RectF(
        scene.leftEyeRect.X + (scene.leftEyeRect.Width - style.pupilWidthPx) * 0.5f + pupilOffsetX,
        scene.leftEyeRect.Y + (scene.leftEyeRect.Height - pupilH) * 0.5f + pupilOffsetY,
        style.pupilWidthPx,
        pupilH);
    scene.rightPupilRect = Gdiplus::RectF(
        scene.rightEyeRect.X + (scene.rightEyeRect.Width - style.pupilWidthPx) * 0.5f + pupilOffsetX,
        scene.rightEyeRect.Y + (scene.rightEyeRect.Height - pupilH) * 0.5f + pupilOffsetY,
        style.pupilWidthPx,
        pupilH);
    scene.leftEyeHighlightRect = Gdiplus::RectF(
        scene.leftEyeRect.X + style.eyeHighlightInsetXPx,
        scene.leftEyeRect.Y + style.eyeHighlightInsetYPx,
        style.eyeHighlightSizePx,
        style.eyeHighlightSizePx);
    scene.rightEyeHighlightRect = Gdiplus::RectF(
        scene.rightEyeRect.X + style.eyeHighlightInsetXPx,
        scene.rightEyeRect.Y + style.eyeHighlightInsetYPx,
        style.eyeHighlightSizePx,
        style.eyeHighlightSizePx);
    scene.noseRect = Gdiplus::RectF(
        scene.centerX - style.noseWidthPx * 0.5f + poseFaceXOffset,
        scene.headRect.Y + scene.headRect.Height * style.noseYRatio + poseFaceYOffset,
        style.noseWidthPx,
        style.noseHeightPx);
    scene.mouthRect = Gdiplus::RectF(
        scene.centerX - style.mouthWidthPx * 0.5f + poseFaceXOffset,
        scene.headRect.Y + scene.headRect.Height * style.mouthYRatio + poseFaceYOffset,
        style.mouthWidthPx,
        style.mouthHeightBasePx + profile.reactiveIntensity * style.mouthReactiveHeightPx * skinTuning.mouthReactiveScale);
    const float browY = scene.headRect.Y + scene.headRect.Height * style.browYRatio;
    scene.leftBrowStart = Gdiplus::PointF(
        scene.centerX - scene.headRect.Width * style.leftBrowStartXRatio,
        browY + profile.browLift - profile.browTilt * skinTuning.browTiltScale * style.leftBrowStartTiltScale);
    scene.leftBrowEnd = Gdiplus::PointF(
        scene.centerX - scene.headRect.Width * style.leftBrowEndXRatio,
        browY + profile.browLift + profile.browTilt * skinTuning.browTiltScale * style.leftBrowEndTiltScale);
    scene.rightBrowStart = Gdiplus::PointF(
        scene.centerX + scene.headRect.Width * style.rightBrowStartXRatio,
        browY + profile.browLift - profile.browTilt * skinTuning.browTiltScale * style.rightBrowStartTiltScale);
    scene.rightBrowEnd = Gdiplus::PointF(
        scene.centerX + scene.headRect.Width * style.rightBrowEndXRatio,
        browY + profile.browLift + profile.browTilt * skinTuning.browTiltScale * style.rightBrowEndTiltScale);
    scene.mouthStartDeg = profile.mouthStartDeg;
    scene.mouthSweepDeg = profile.mouthSweepDeg;
    scene.mouthStrokeWidth = profile.mouthStrokeWidth;
    scene.leftBlushRect = Gdiplus::RectF(
        scene.centerX - scene.headRect.Width * style.blushXRatio,
        scene.headRect.Y + scene.headRect.Height * style.blushYRatio - poseBlushLift,
        style.blushWidthPx * skinTuning.blushWidthScale,
        style.blushHeightPx);
    scene.rightBlushRect = Gdiplus::RectF(
        scene.centerX + scene.headRect.Width * style.blushXRatio - style.blushWidthPx * skinTuning.blushWidthScale,
        scene.headRect.Y + scene.headRect.Height * style.blushYRatio - poseBlushLift,
        style.blushWidthPx * skinTuning.blushWidthScale,
        style.blushHeightPx);
    scene.leftCheekContourRect = Gdiplus::RectF(
        scene.centerX - scene.headRect.Width * style.cheekContourXRatio,
        scene.headRect.Y + scene.headRect.Height * style.cheekContourYRatio,
        scene.headRect.Width * style.cheekContourWidthRatio * skinTuning.cheekWidthScale,
        scene.headRect.Height * style.cheekContourHeightRatio * skinTuning.cheekHeightScale);
    scene.rightCheekContourRect = Gdiplus::RectF(
        scene.centerX + scene.headRect.Width * style.cheekContourXRatio -
            scene.headRect.Width * style.cheekContourWidthRatio * skinTuning.cheekWidthScale,
        scene.headRect.Y + scene.headRect.Height * style.cheekContourYRatio,
        scene.headRect.Width * style.cheekContourWidthRatio * skinTuning.cheekWidthScale,
        scene.headRect.Height * style.cheekContourHeightRatio * skinTuning.cheekHeightScale);
    scene.jawContourRect = Gdiplus::RectF(
        scene.centerX - scene.headRect.Width * style.jawContourWidthRatio * 0.5f,
        scene.headRect.Y + scene.headRect.Height * style.jawContourYRatio,
        scene.headRect.Width * style.jawContourWidthRatio,
        scene.headRect.Height * style.jawContourHeightRatio * skinTuning.jawHeightScale);
    scene.muzzlePadRect = Gdiplus::RectF(
        scene.centerX - scene.headRect.Width * style.muzzlePadWidthRatio * skinTuning.muzzleWidthScale * 0.5f,
        scene.headRect.Y + scene.headRect.Height * style.muzzlePadYRatio,
        scene.headRect.Width * style.muzzlePadWidthRatio * skinTuning.muzzleWidthScale,
        scene.headRect.Height * style.muzzlePadHeightRatio * skinTuning.muzzleHeightScale);
    scene.foreheadPadRect = Gdiplus::RectF(
        scene.centerX - scene.headRect.Width * style.foreheadPadWidthRatio * skinTuning.foreheadWidthScale * 0.5f,
        scene.headRect.Y + scene.headRect.Height * style.foreheadPadYRatio,
        scene.headRect.Width * style.foreheadPadWidthRatio * skinTuning.foreheadWidthScale,
        scene.headRect.Height * style.foreheadPadHeightRatio * skinTuning.foreheadHeightScale);
    scene.crownPadRect = Gdiplus::RectF(
        scene.centerX - scene.headRect.Width * style.crownPadWidthRatio * 0.5f,
        scene.headRect.Y + scene.headRect.Height * style.crownPadYRatio,
        scene.headRect.Width * style.crownPadWidthRatio,
        scene.headRect.Height * style.crownPadHeightRatio);
    const bool leftHeadFront = scene.facingSign < 0.0f;
    const bool rightHeadFront = !leftHeadFront;
    const float leftParietalWidthScale =
        leftHeadFront ? style.frontParietalBridgeWidthScale : style.rearParietalBridgeWidthScale;
    const float rightParietalWidthScale =
        rightHeadFront ? style.frontParietalBridgeWidthScale : style.rearParietalBridgeWidthScale;
    const float leftEarSkullWidthScale =
        leftHeadFront ? style.frontEarSkullBridgeWidthScale : style.rearEarSkullBridgeWidthScale;
    const float rightEarSkullWidthScale =
        rightHeadFront ? style.frontEarSkullBridgeWidthScale : style.rearEarSkullBridgeWidthScale;
    scene.leftParietalBridgeRect = Gdiplus::RectF(
        scene.centerX - scene.headRect.Width * style.parietalBridgeXRatio,
        scene.headRect.Y + scene.headRect.Height * style.parietalBridgeYRatio,
        scene.headRect.Width * style.parietalBridgeWidthRatio * leftParietalWidthScale,
        scene.headRect.Height * style.parietalBridgeHeightRatio);
    scene.rightParietalBridgeRect = Gdiplus::RectF(
        scene.centerX + scene.headRect.Width * style.parietalBridgeXRatio -
            scene.headRect.Width * style.parietalBridgeWidthRatio * rightParietalWidthScale,
        scene.headRect.Y + scene.headRect.Height * style.parietalBridgeYRatio,
        scene.headRect.Width * style.parietalBridgeWidthRatio * rightParietalWidthScale,
        scene.headRect.Height * style.parietalBridgeHeightRatio);
    scene.leftEarSkullBridgeRect = Gdiplus::RectF(
        scene.centerX - scene.headRect.Width * style.earSkullBridgeXRatio,
        scene.headRect.Y + scene.headRect.Height * style.earSkullBridgeYRatio,
        scene.headRect.Width * style.earSkullBridgeWidthRatio * leftEarSkullWidthScale,
        scene.headRect.Height * style.earSkullBridgeHeightRatio);
    scene.rightEarSkullBridgeRect = Gdiplus::RectF(
        scene.centerX + scene.headRect.Width * style.earSkullBridgeXRatio -
            scene.headRect.Width * style.earSkullBridgeWidthRatio * rightEarSkullWidthScale,
        scene.headRect.Y + scene.headRect.Height * style.earSkullBridgeYRatio,
        scene.headRect.Width * style.earSkullBridgeWidthRatio * rightEarSkullWidthScale,
        scene.headRect.Height * style.earSkullBridgeHeightRatio);
    scene.leftOccipitalContourRect = Gdiplus::RectF(
        scene.centerX - scene.headRect.Width * style.occipitalContourXRatio,
        scene.headRect.Y + scene.headRect.Height * style.occipitalContourYRatio,
        scene.headRect.Width * style.occipitalContourWidthRatio,
        scene.headRect.Height * style.occipitalContourHeightRatio);
    scene.rightOccipitalContourRect = Gdiplus::RectF(
        scene.centerX + scene.headRect.Width * style.occipitalContourXRatio - scene.headRect.Width * style.occipitalContourWidthRatio,
        scene.headRect.Y + scene.headRect.Height * style.occipitalContourYRatio,
        scene.headRect.Width * style.occipitalContourWidthRatio,
        scene.headRect.Height * style.occipitalContourHeightRatio);
    scene.leftTempleContourRect = Gdiplus::RectF(
        scene.centerX - scene.headRect.Width * style.templeContourXRatio,
        scene.headRect.Y + scene.headRect.Height * style.templeContourYRatio,
        scene.headRect.Width * style.templeContourWidthRatio,
        scene.headRect.Height * style.templeContourHeightRatio * skinTuning.templeHeightScale);
    scene.rightTempleContourRect = Gdiplus::RectF(
        scene.centerX + scene.headRect.Width * style.templeContourXRatio - scene.headRect.Width * style.templeContourWidthRatio,
        scene.headRect.Y + scene.headRect.Height * style.templeContourYRatio,
        scene.headRect.Width * style.templeContourWidthRatio,
        scene.headRect.Height * style.templeContourHeightRatio * skinTuning.templeHeightScale);
    scene.leftUnderEyeContourRect = Gdiplus::RectF(
        scene.centerX - scene.headRect.Width * style.underEyeContourXRatio,
        scene.headRect.Y + scene.headRect.Height * style.underEyeContourYRatio,
        scene.headRect.Width * style.underEyeContourWidthRatio,
        scene.headRect.Height * style.underEyeContourHeightRatio);
    scene.rightUnderEyeContourRect = Gdiplus::RectF(
        scene.centerX + scene.headRect.Width * style.underEyeContourXRatio - scene.headRect.Width * style.underEyeContourWidthRatio,
        scene.headRect.Y + scene.headRect.Height * style.underEyeContourYRatio,
        scene.headRect.Width * style.underEyeContourWidthRatio,
        scene.headRect.Height * style.underEyeContourHeightRatio);
    scene.noseBridgeRect = Gdiplus::RectF(
        scene.centerX - scene.headRect.Width * style.noseBridgeWidthRatio * 0.5f,
        scene.headRect.Y + scene.headRect.Height * style.noseBridgeYRatio,
        scene.headRect.Width * style.noseBridgeWidthRatio,
        scene.headRect.Height * style.noseBridgeHeightRatio);
    scene.eyeHighlightAlpha = profile.eyeHighlightAlpha * skinTuning.highlightAlphaScale;
    scene.whiskerStrokeWidth = 1.0f + profile.whiskerSpread * skinTuning.whiskerSpreadScale * 0.35f;

    switch (appearanceSemantics.comboPreset) {
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Dreamy:
        if (runtime.follow) {
            scene.leftBlushRect.Y -= 1.2f;
            scene.rightBlushRect.Y -= 1.2f;
            scene.mouthRect.Y -= 0.8f;
        }
        break;
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Agile:
        if (runtime.drag || runtime.follow) {
            scene.leftEyeRect.Y -= 0.8f;
            scene.rightEyeRect.Y -= 0.8f;
            scene.leftPupilRect.Y -= 0.5f;
            scene.rightPupilRect.Y -= 0.5f;
        }
        break;
    case Win32MouseCompanionRealRendererAppearanceComboPreset::Charming:
        if (runtime.hold || runtime.click) {
            scene.leftBlushRect.Height *= 1.08f;
            scene.rightBlushRect.Height *= 1.08f;
            scene.mouthRect.Height *= 1.06f;
        }
        break;
    case Win32MouseCompanionRealRendererAppearanceComboPreset::None:
        break;
    }

    const float whiskerAnchorY = scene.headRect.Y + scene.headRect.Height * style.whiskerAnchorYRatio;
    const float leftWhiskerX = scene.centerX - scene.headRect.Width * style.whiskerInnerXRatio;
    const float rightWhiskerX = scene.centerX + scene.headRect.Width * style.whiskerInnerXRatio;
    const float whiskerSpread =
        profile.whiskerSpread * style.whiskerSpreadScale * skinTuning.whiskerSpreadScale +
        poseWhiskerBias;
    const float whiskerTilt = profile.whiskerTilt * style.whiskerTiltScale + poseWhiskerBias * 1.8f;
    SetWhiskerPair(
        leftWhiskerX,
        whiskerAnchorY,
        -1.0f,
        style.whiskerOuterLengthPx,
        -style.whiskerTopYOffsetPx,
        whiskerSpread,
        -whiskerTilt,
        scene.leftWhiskerStart[0],
        scene.leftWhiskerEnd[0]);
    SetWhiskerPair(
        leftWhiskerX,
        whiskerAnchorY,
        -1.0f,
        style.whiskerMiddleLengthPx,
        0.0f,
        0.0f,
        -whiskerTilt * 0.5f,
        scene.leftWhiskerStart[1],
        scene.leftWhiskerEnd[1]);
    SetWhiskerPair(
        leftWhiskerX,
        whiskerAnchorY,
        -1.0f,
        style.whiskerLowerLengthPx,
        style.whiskerLowerYOffsetPx,
        -whiskerSpread,
        -whiskerTilt,
        scene.leftWhiskerStart[2],
        scene.leftWhiskerEnd[2]);
    SetWhiskerPair(
        rightWhiskerX,
        whiskerAnchorY,
        1.0f,
        style.whiskerOuterLengthPx,
        -style.whiskerTopYOffsetPx,
        whiskerSpread,
        whiskerTilt,
        scene.rightWhiskerStart[0],
        scene.rightWhiskerEnd[0]);
    SetWhiskerPair(
        rightWhiskerX,
        whiskerAnchorY,
        1.0f,
        style.whiskerMiddleLengthPx,
        0.0f,
        0.0f,
        whiskerTilt * 0.5f,
        scene.rightWhiskerStart[1],
        scene.rightWhiskerEnd[1]);
    SetWhiskerPair(
        rightWhiskerX,
        whiskerAnchorY,
        1.0f,
        style.whiskerLowerLengthPx,
        style.whiskerLowerYOffsetPx,
        -whiskerSpread,
        whiskerTilt,
        scene.rightWhiskerStart[2],
        scene.rightWhiskerEnd[2]);
}

} // namespace mousefx::windows

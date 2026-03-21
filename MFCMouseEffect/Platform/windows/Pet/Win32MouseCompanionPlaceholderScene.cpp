#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderActionProfile.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderAccessory.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderScene.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderAdornment.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderGait.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderMotion.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderPalette.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderPosture.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderRhythm.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderSilhouette.h"

#include <algorithm>

namespace mousefx::windows {
namespace {

constexpr int kBoneLeftEar = 0;
constexpr int kBoneRightEar = 1;
constexpr int kBoneLeftHand = 2;
constexpr int kBoneRightHand = 3;
constexpr int kBoneLeftLeg = 4;
constexpr int kBoneRightLeg = 5;

float ClampUnit(float value) {
    return std::clamp(value, 0.0f, 1.0f);
}

float ResolveSignedUnit(float value) {
    return std::clamp(value, -1.0f, 1.0f);
}

const MouseCompanionPetPoseSample* FindPoseSample(
    const MouseCompanionPetPoseFrame& frame,
    int boneIndex) {
    for (const auto& sample : frame.samples) {
        if (sample.boneIndex == boneIndex) {
            return &sample;
        }
    }
    return nullptr;
}

} // namespace

Win32MouseCompanionPlaceholderScene BuildWin32MouseCompanionPlaceholderScene(
    const Win32MouseCompanionVisualState& state,
    int width,
    int height) {
    Win32MouseCompanionPlaceholderScene scene{};
    if (width <= 0 || height <= 0) {
        return scene;
    }

    const float intensity = ClampUnit(state.lastActionIntensity);
    const float signedIntensity = ResolveSignedUnit(state.lastActionIntensity);
    const bool follow = state.lastActionName == "follow";
    const bool click = state.lastActionName == "click_react" || state.lastActionName == "drag";
    const bool drag = state.lastActionName == "drag";
    const bool hold = state.lastActionName == "hold_react";
    const bool scroll = state.lastActionName == "scroll_react";
    const float tint = ClampUnit(state.lastHeadTintAmount);
    const Win32MouseCompanionActionSample& clipSample = state.latestActionClipSample;
    const float clickSquash = click ? (0.10f + intensity * 0.14f) : 0.0f;
    const float holdSquat = hold ? (0.10f + intensity * 0.16f) : 0.0f;
    const float scrollLean = scroll ? signedIntensity * 0.12f : 0.0f;

    const MouseCompanionPetPoseFrame& poseFrame = state.latestPoseFrame;
    const MouseCompanionPetPoseSample* leftEarPose = FindPoseSample(poseFrame, kBoneLeftEar);
    const MouseCompanionPetPoseSample* rightEarPose = FindPoseSample(poseFrame, kBoneRightEar);
    const MouseCompanionPetPoseSample* leftHandPose = FindPoseSample(poseFrame, kBoneLeftHand);
    const MouseCompanionPetPoseSample* rightHandPose = FindPoseSample(poseFrame, kBoneRightHand);
    const MouseCompanionPetPoseSample* leftLegPose = FindPoseSample(poseFrame, kBoneLeftLeg);
    const MouseCompanionPetPoseSample* rightLegPose = FindPoseSample(poseFrame, kBoneRightLeg);
    const float poseEarLift = leftEarPose ? leftEarPose->position[1] * 24.0f : 0.0f;
    const float poseEarSpread = leftEarPose ? -leftEarPose->position[0] * 18.0f : 0.0f;
    const float poseRightEarLift = rightEarPose ? rightEarPose->position[1] * 24.0f : 0.0f;
    const float poseRightEarSpread = rightEarPose ? rightEarPose->position[0] * 18.0f : 0.0f;
    const float poseLeftHandLift = leftHandPose ? -leftHandPose->position[1] * 32.0f : 0.0f;
    const float poseRightHandLift = rightHandPose ? -rightHandPose->position[1] * 32.0f : 0.0f;
    const float poseLeftHandSpread = leftHandPose ? leftHandPose->position[0] * 18.0f : 0.0f;
    const float poseRightHandSpread = rightHandPose ? rightHandPose->position[0] * 18.0f : 0.0f;
    const float poseLeftLegShift = leftLegPose ? leftLegPose->position[0] * 20.0f : 0.0f;
    const float poseRightLegShift = rightLegPose ? rightLegPose->position[0] * 20.0f : 0.0f;

    scene.centerX = static_cast<float>(width) * 0.5f;
    scene.facingSign = (state.facingDirection >= 0) ? 1.0f : -1.0f;
    scene.motion = BuildWin32MouseCompanionPlaceholderMotion(state);
    scene.posture = BuildWin32MouseCompanionPlaceholderPosture(state, scene.motion, scene.facingSign);
    scene.actionProfile = BuildWin32MouseCompanionPlaceholderActionProfile(
        state,
        scene.motion,
        scene.posture,
        scene.facingSign);
    scene.rhythm = BuildWin32MouseCompanionPlaceholderRhythm(scene.motion, scene.facingSign);
    const float centerY =
        static_cast<float>(height) * (hold ? 0.68f : 0.63f) + scene.posture.bodyCenterYOffset;
    const float clipBodyScaleX = clipSample.valid ? std::clamp(clipSample.bodyScaleX, 0.8f, 1.25f) : 1.0f;
    const float clipBodyScaleY = clipSample.valid ? std::clamp(clipSample.bodyScaleY, 0.72f, 1.20f) : 1.0f;
    const float bodyW = static_cast<float>(width) * (hold ? 0.35f : 0.30f) *
        (1.0f + clickSquash) * clipBodyScaleX * scene.motion.bodyScaleX * scene.posture.bodyWidthScale;
    const float bodyH = static_cast<float>(height) * (hold ? 0.24f : 0.29f) *
        (1.0f - clickSquash * 0.55f - holdSquat * 0.35f) *
        clipBodyScaleY * scene.motion.breathScaleY * scene.motion.bodyScaleY * scene.posture.bodyHeightScale;
    const float headW = static_cast<float>(width) * 0.30f * (1.0f + clickSquash * 0.18f) *
        scene.motion.headScaleX * scene.posture.headWidthScale;
    const float headH = static_cast<float>(height) * 0.24f * (1.0f - clickSquash * 0.42f) *
        scene.motion.headScaleY * scene.posture.headHeightScale;
    const float clipHeadPitch = clipSample.valid ? clipSample.headPitch : 0.0f;
    const float clipHeadYaw = clipSample.valid ? clipSample.headYaw : 0.0f;
    const float headY = centerY - bodyH * (hold ? 0.76f : 0.82f) - clickSquash * 3.0f +
        holdSquat * 2.0f - clipHeadPitch * 10.0f + scene.motion.headBobPx + scene.posture.headAnchorYOffset;
    const float earLift = (follow ? 7.0f + intensity * 3.0f : 1.5f) + (scroll ? 12.0f * std::abs(signedIntensity) : 0.0f);
    const float earTilt = (follow ? 0.12f + intensity * 0.24f : 0.04f) + (scroll ? signedIntensity * 0.30f : 0.0f);
    const float handLift = hold ? (10.0f + intensity * 12.0f) : (follow ? 3.0f + intensity * 2.0f : 0.0f);
    const float handSpread = drag ? (5.0f + intensity * 7.0f) : (scroll ? std::abs(signedIntensity) * 3.0f : 0.0f);
    const float legShift = follow ? (7.0f + intensity * 7.0f) : 0.0f;
    const float followSideBias = (follow ? 4.0f + intensity * 5.0f : 0.0f) + (drag ? 3.0f + intensity * 4.0f : 0.0f);
    const float headSideBias = scene.facingSign * followSideBias;
    const float earSideBias = scene.facingSign * (followSideBias * 0.85f);
    const float bodySideBias = scene.facingSign * (followSideBias * 0.45f);
    scene.bodyLeanPx =
        (scrollLean + (clipSample.valid ? clipSample.bodyLean * 0.85f : 0.0f)) * static_cast<float>(width) * 0.30f;
    scene.bodyLeanPx += scene.motion.reactive.scrollDirection * scene.motion.reactive.scrollKick * 5.5f;
    scene.bodyLeanPx += scene.motion.reactive.dragTension * scene.facingSign * 3.0f;
    scene.bodyTiltDeg = scene.motion.bodyTiltDeg;
    scene.glowAlpha =
        28.0f + intensity * 18.0f + tint * 16.0f +
        (scroll ? 12.0f * std::abs(signedIntensity) : 0.0f) +
        scene.motion.glowBoostAlpha;
    const float pawY = centerY - bodyH * 0.10f - handLift + scene.posture.pawYOffset;

    scene.palette = BuildWin32MouseCompanionPlaceholderPalette(
        state.appearanceProfile.skinVariantId,
        tint,
        scroll,
        signedIntensity);
    scene.bodyFill = scene.palette.bodyFill;
    scene.bodyStroke = scene.palette.bodyStroke;
    scene.headFill = scene.palette.headFill;
    scene.headFillRear = scene.palette.headFillRear;
    scene.earInner = scene.palette.earInner;
    scene.earInnerRear = scene.palette.earInnerRear;
    scene.accent = scene.palette.accent;
    scene.frontSideIsLeft = (scene.facingSign < 0.0f);

    scene.shadowRect = Gdiplus::RectF(
        scene.centerX - bodyW * 0.56f + scene.bodyLeanPx * 0.25f + bodySideBias * 0.45f,
        centerY + bodyH * 0.54f + scene.posture.shadowOffsetPx,
        bodyW * 1.12f * scene.motion.shadowScaleX,
        bodyH * 0.30f * scene.motion.shadowScaleY);
    scene.bodyRect = Gdiplus::RectF(
        scene.centerX - bodyW * 0.5f + scene.bodyLeanPx + scene.facingSign * 2.0f + bodySideBias +
            scene.posture.bodyForwardBiasPx + scene.rhythm.silhouetteNudgePx * 0.25f + scene.posture.silhouetteBiasPx,
        centerY - bodyH * 0.5f + scene.motion.bodyBobPx - scene.actionProfile.bodyArchPx,
        bodyW,
        bodyH);
    scene.headRect = Gdiplus::RectF(
        scene.centerX - headW * 0.5f +
            scene.bodyLeanPx * 0.45f +
            scene.facingSign * (6.0f + clipHeadYaw * 10.0f) +
            headSideBias + scene.posture.headForwardBiasPx + scene.actionProfile.headForwardPx +
            scene.motion.headSwayPx + scene.rhythm.silhouetteNudgePx * 0.15f + scene.posture.silhouetteBiasPx * 0.5f,
        headY - headH * 0.5f - scene.actionProfile.headLiftPx,
        headW,
        headH);
    scene.tailRect = Gdiplus::RectF(
        scene.bodyRect.X + (scene.facingSign > 0.0f ? -bodyW * 0.10f : bodyW * 0.86f) +
            scene.motion.tailSwingPx * 0.35f + scene.actionProfile.tailCurlPx,
        scene.bodyRect.Y + bodyH * 0.32f - scene.motion.tailLiftPx -
            scene.rhythm.tailTipLiftPx * 0.2f - scene.actionProfile.tailRootLiftPx,
        bodyW * 0.20f * scene.motion.tailWidthScale,
        bodyH * 0.20f);
    scene.tailTipRect = Gdiplus::RectF(
        scene.tailRect.X + (scene.facingSign > 0.0f ? -scene.tailRect.Width * 0.26f : scene.tailRect.Width * 0.70f) +
            scene.actionProfile.tailCurlPx * 0.35f,
        scene.tailRect.Y + scene.tailRect.Height * 0.18f - scene.rhythm.tailTipLiftPx,
        scene.tailRect.Width * 0.42f,
        scene.tailRect.Height * 0.52f);
    scene.chestRect = Gdiplus::RectF(
        scene.bodyRect.X + bodyW * 0.24f,
        scene.bodyRect.Y + bodyH * 0.20f - scene.motion.cheekLiftPx * 0.20f -
            scene.motion.chestBobPx - scene.actionProfile.chestLiftPx,
        bodyW * 0.36f * scene.actionProfile.chestWidthScale,
        bodyH * 0.28f);

    const float earBaseY = scene.headRect.Y + scene.headRect.Height * 0.18f;
    const float earTopY = scene.headRect.Y - scene.headRect.Height * (1.20f + intensity * 0.22f) -
        earLift - poseEarLift - scene.motion.frontEarLiftPx;
    const float rightEarTopY = scene.headRect.Y - scene.headRect.Height * (1.20f + intensity * 0.22f) -
        earLift - poseRightEarLift - scene.motion.rearEarLiftPx;
    const float earBaseOffset = scene.headRect.Width * 0.24f + scene.posture.earBaseSpreadPx;
    const float earTipOffset = scene.headRect.Width * 0.15f + std::abs(earTilt) * 18.0f;

    scene.leftEar = {{
        { scene.centerX - earBaseOffset + headSideBias * 0.55f, earBaseY },
        { scene.centerX - earBaseOffset - scene.headRect.Width * 0.08f - earTilt * 16.0f - poseEarSpread - scene.facingSign * 2.0f + earSideBias + scene.motion.frontEarSwingPx, scene.headRect.Y - scene.headRect.Height * 0.16f },
        { scene.centerX - earTipOffset - scene.headRect.Width * 0.02f - poseEarSpread - scene.facingSign * 3.0f + earSideBias + scene.motion.frontEarSwingPx + scene.rhythm.earSyncBoost, earTopY - scene.rhythm.earCycle * 1.2f - scene.posture.earHeightBoostPx },
        { scene.centerX - earBaseOffset + scene.headRect.Width * 0.03f + headSideBias * 0.35f, scene.headRect.Y + scene.headRect.Height * 0.02f },
    }};
    scene.rightEar = {{
        { scene.centerX + earBaseOffset + headSideBias * 0.55f, earBaseY },
        { scene.centerX + earBaseOffset + scene.headRect.Width * 0.08f - earTilt * 16.0f + poseRightEarSpread - scene.facingSign * 2.0f + earSideBias + scene.motion.rearEarSwingPx, scene.headRect.Y - scene.headRect.Height * 0.16f },
        { scene.centerX + earTipOffset + scene.headRect.Width * 0.02f + poseRightEarSpread - scene.facingSign * 3.0f + earSideBias + scene.motion.rearEarSwingPx - scene.rhythm.earSyncBoost * 0.55f, rightEarTopY + scene.rhythm.earCycle * 0.8f - scene.posture.earHeightBoostPx * 0.82f },
        { scene.centerX + earBaseOffset - scene.headRect.Width * 0.03f + headSideBias * 0.35f, scene.headRect.Y + scene.headRect.Height * 0.02f },
    }};

    scene.leftHandRect = Gdiplus::RectF(
        scene.centerX - bodyW * 0.50f + scene.bodyLeanPx - handSpread - scene.posture.foreStanceSpreadPx +
            poseLeftHandSpread + scene.facingSign * 2.5f + bodySideBias + scene.rhythm.foreCycle * 0.9f,
        pawY - poseLeftHandLift - scene.motion.frontPawLiftPx,
        bodyW * 0.20f,
        bodyH * 0.34f);
    scene.rightHandRect = Gdiplus::RectF(
        scene.centerX + bodyW * 0.30f + scene.bodyLeanPx + handSpread + scene.posture.foreStanceSpreadPx +
            poseRightHandSpread + scene.facingSign * 1.0f + bodySideBias - scene.rhythm.foreCycle * 0.8f,
        pawY - poseRightHandLift - scene.motion.rearPawLiftPx,
        bodyW * 0.20f,
        bodyH * 0.34f);
    scene.leftLegRect = Gdiplus::RectF(
        scene.centerX - bodyW * 0.34f - legShift * 0.25f - scene.posture.rearStanceSpreadPx +
            scene.bodyLeanPx * 0.28f + poseLeftLegShift + scene.rhythm.rearCycle * 0.8f,
        centerY + bodyH * 0.34f - scene.motion.frontLegLiftPx,
        bodyW * 0.18f,
        bodyH * 0.24f);
    scene.rightLegRect = Gdiplus::RectF(
        scene.centerX + bodyW * 0.16f + legShift * 0.25f + scene.posture.rearStanceSpreadPx +
            scene.bodyLeanPx * 0.28f + poseRightLegShift - scene.rhythm.rearCycle * 0.8f,
        centerY + bodyH * 0.34f - scene.motion.rearLegLiftPx,
        bodyW * 0.18f,
        bodyH * 0.24f);
    scene.leftPawPadRect = Gdiplus::RectF(
        scene.leftHandRect.X + scene.leftHandRect.Width * 0.18f,
        scene.leftHandRect.Y + scene.leftHandRect.Height * 0.60f,
        scene.leftHandRect.Width * 0.44f,
        scene.leftHandRect.Height * 0.20f);
    scene.rightPawPadRect = Gdiplus::RectF(
        scene.rightHandRect.X + scene.rightHandRect.Width * 0.18f,
        scene.rightHandRect.Y + scene.rightHandRect.Height * 0.60f,
        scene.rightHandRect.Width * 0.44f,
        scene.rightHandRect.Height * 0.20f);

    scene.modelAssetAvailable = state.modelAssetAvailable;
    scene.actionLibraryAvailable = state.actionLibraryAvailable;
    scene.poseBadgeVisible = state.poseBindingConfigured || state.poseFrameAvailable;
    scene.accessoryVisible = !state.appearanceProfile.enabledAccessoryIds.empty();
    scene.eyeColor = scene.palette.eyeColor;
    scene.mouthColor = scene.palette.mouthColor;
    scene.blushColor = scene.palette.blushColor;
    scene.chestFill = scene.palette.chestFill;
    scene.tailFill = scene.palette.tailFill;
    scene.tailTipFill = scene.palette.tailTipFill;
    scene.pawPadFill = scene.palette.pawPadFill;
    scene.shadowColor = scene.palette.shadowColor;
    scene.accentGlow = scene.palette.accentGlow;
    scene.eyeHeight =
        (hold ? 4.5f : (click ? 3.5f : 8.0f)) *
        std::max(0.18f, scene.motion.eyeOpenScale);
    scene.leftEyeRect = Gdiplus::RectF(
        scene.centerX - scene.headRect.Width * 0.20f + scene.bodyLeanPx * 0.45f,
        scene.headRect.Y + scene.headRect.Height * 0.42f - scene.motion.cheekLiftPx * 0.10f,
        5.0f,
        scene.eyeHeight);
    scene.rightEyeRect = Gdiplus::RectF(
        scene.centerX + scene.headRect.Width * 0.20f - 5.0f + scene.bodyLeanPx * 0.45f,
        scene.headRect.Y + scene.headRect.Height * 0.42f - scene.motion.cheekLiftPx * 0.10f,
        5.0f,
        scene.eyeHeight);
    scene.mouthRect = Gdiplus::RectF(
        scene.centerX - 6.0f + scene.bodyLeanPx * 0.45f,
        scene.headRect.Y + scene.headRect.Height * 0.56f + scene.motion.mouthOpenPx * 0.10f,
        12.0f,
        8.0f + scene.motion.mouthOpenPx);
    scene.noseRect = Gdiplus::RectF(
        scene.centerX - 2.0f + scene.bodyLeanPx * 0.45f,
        scene.headRect.Y + scene.headRect.Height * 0.52f - scene.motion.cheekLiftPx * 0.08f,
        4.0f,
        3.0f);
    scene.leftBlushRect = Gdiplus::RectF(
        scene.centerX - scene.headRect.Width * 0.32f + scene.bodyLeanPx * 0.45f,
        scene.headRect.Y + scene.headRect.Height * 0.58f - scene.motion.cheekLiftPx * 0.12f,
        9.0f,
        5.0f + scene.motion.reactive.attentionFocus * 1.6f);
    scene.rightBlushRect = Gdiplus::RectF(
        scene.centerX + scene.headRect.Width * 0.32f - 9.0f + scene.bodyLeanPx * 0.45f,
        scene.headRect.Y + scene.headRect.Height * 0.58f - scene.motion.cheekLiftPx * 0.12f,
        9.0f,
        5.0f + scene.motion.reactive.attentionFocus * 1.6f);
    const float whiskerYTop = scene.headRect.Y + scene.headRect.Height * 0.56f;
    const float whiskerYBottom = scene.headRect.Y + scene.headRect.Height * 0.64f;
    const float whiskerStartInset = scene.headRect.Width * 0.14f;
    const float whiskerLength = 8.0f + scene.motion.whiskerSpreadPx;
    scene.leftWhiskerTop = {{
        Gdiplus::PointF(scene.headRect.X + whiskerStartInset + scene.bodyLeanPx * 0.20f, whiskerYTop),
        Gdiplus::PointF(scene.headRect.X - whiskerLength + scene.bodyLeanPx * 0.10f, whiskerYTop - 1.8f - scene.motion.reactive.scrollDirection * 1.2f),
    }};
    scene.leftWhiskerBottom = {{
        Gdiplus::PointF(scene.headRect.X + whiskerStartInset + scene.bodyLeanPx * 0.20f, whiskerYBottom),
        Gdiplus::PointF(scene.headRect.X - whiskerLength * 0.92f + scene.bodyLeanPx * 0.10f, whiskerYBottom + 1.8f + scene.motion.reactive.scrollDirection * 1.1f),
    }};
    scene.rightWhiskerTop = {{
        Gdiplus::PointF(scene.headRect.X + scene.headRect.Width - whiskerStartInset + scene.bodyLeanPx * 0.20f, whiskerYTop),
        Gdiplus::PointF(scene.headRect.X + scene.headRect.Width + whiskerLength + scene.bodyLeanPx * 0.30f, whiskerYTop - 1.8f + scene.motion.reactive.scrollDirection * 1.2f),
    }};
    scene.rightWhiskerBottom = {{
        Gdiplus::PointF(scene.headRect.X + scene.headRect.Width - whiskerStartInset + scene.bodyLeanPx * 0.20f, whiskerYBottom),
        Gdiplus::PointF(scene.headRect.X + scene.headRect.Width + whiskerLength * 0.92f + scene.bodyLeanPx * 0.30f, whiskerYBottom + 1.8f - scene.motion.reactive.scrollDirection * 1.1f),
    }};
    scene.expression = BuildWin32MouseCompanionPlaceholderExpression(
        scene.leftEyeRect,
        scene.rightEyeRect,
        scene.mouthRect,
        scene.headRect,
        scene.bodyLeanPx,
        scene.motion,
        scene.eyeColor,
        scene.mouthColor);
    scene.adornment = BuildWin32MouseCompanionPlaceholderAdornment(
        scene.bodyRect,
        scene.headRect,
        scene.bodyLeanPx,
        scene.facingSign,
        scene.motion,
        scene.accent,
        scene.bodyStroke,
        scene.headFill);
    scene.gait = BuildWin32MouseCompanionPlaceholderGait(
        scene.bodyRect,
        scene.headRect,
        scene.leftHandRect,
        scene.rightHandRect,
        scene.leftLegRect,
        scene.rightLegRect,
        scene.frontSideIsLeft,
        scene.motion,
        scene.bodyFill,
        scene.headFill,
        scene.bodyStroke);
    scene.gait.bridgeWidth += scene.rhythm.strideAccent * 0.25f;
    scene.accessory = BuildWin32MouseCompanionPlaceholderAccessory(
        state.appearanceProfile.enabledAccessoryIds,
        scene.headRect,
        scene.bodyLeanPx,
        scene.facingSign,
        scene.accent,
        scene.bodyStroke);
    scene.silhouette = BuildWin32MouseCompanionPlaceholderSilhouette(
        scene.bodyRect,
        scene.headRect,
        scene.tailRect,
        scene.chestRect,
        scene.frontSideIsLeft,
        scene.facingSign,
        scene.rhythm.silhouetteNudgePx,
        scene.actionProfile.frontDepthBiasPx,
        scene.actionProfile.rearDepthBiasPx,
        scene.actionProfile.shoulderPatchScale,
        scene.actionProfile.hipPatchScale,
        scene.actionProfile.earRootScale,
        scene.bodyFill,
        scene.headFill,
        scene.bodyStroke);
    return scene;
}

} // namespace mousefx::windows

#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererActionOverlayBuilder.h"

#include <algorithm>

namespace mousefx::windows {
namespace {

float ClampAlpha(float value) {
    return std::clamp(value, 0.0f, 255.0f);
}

float ResolveNodeSourceConfidence(const std::string& sourceTag) {
    if (sourceTag.rfind("bound:", 0) == 0) {
        return 1.0f;
    }
    if (sourceTag.rfind("pose:", 0) == 0) {
        return 0.92f;
    }
    if (sourceTag.rfind("manifest:", 0) == 0) {
        return 0.84f;
    }
    if (sourceTag.rfind("source:", 0) == 0) {
        return 0.72f;
    }
    if (sourceTag.rfind("stub:", 0) == 0) {
        return 0.38f;
    }
    return 0.0f;
}

float ResolveNodePathSignal(const std::string& path) {
    if (path.empty()) {
        return 0.0f;
    }
    float signal = 0.25f;
    if (path.find("/overlay") != std::string::npos) {
        signal += 0.40f;
    }
    if (path.find("/fx/") != std::string::npos) {
        signal += 0.20f;
    }
    return std::min(signal, 1.0f);
}

float ResolveSelectorSignal(const std::string& selectorKey, const std::string& candidateNodeName) {
    float signal = 0.0f;
    if (!selectorKey.empty() && selectorKey.find('|') != std::string::npos) {
        signal += 0.45f;
    }
    if (!candidateNodeName.empty() && candidateNodeName != "unknown") {
        signal += 0.25f;
    }
    if (selectorKey.find("vrm_root:") != std::string::npos ||
        selectorKey.find("scene_root:") != std::string::npos ||
        selectorKey.find("fbx_root:") != std::string::npos) {
        signal += 0.20f;
    }
    return std::min(signal, 1.0f);
}

float ResolvePlanSignal(
    const std::string& parserLocator,
    const std::string& probeLabel,
    float planConfidence) {
    float signal = planConfidence * 0.62f;
    if (!parserLocator.empty() && parserLocator.rfind("parser://", 0) == 0) {
        signal += 0.24f;
    }
    if (!probeLabel.empty() && probeLabel.find("@") != std::string::npos) {
        signal += 0.14f;
    }
    return std::min(signal, 1.0f);
}

Gdiplus::RectF MakeCenteredRect(float centerX, float centerY, float width, float height) {
    return Gdiplus::RectF(centerX - width * 0.5f, centerY - height * 0.5f, width, height);
}

} // namespace

void BuildWin32MouseCompanionRealRendererActionOverlay(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererMotionProfile& profile,
    const Win32MouseCompanionRealRendererStyleProfile& style,
    const Win32MouseCompanionRealRendererLayoutMetrics& metrics,
    Win32MouseCompanionRealRendererScene& scene) {
    const auto mood = BuildWin32MouseCompanionRealRendererAppearanceSemantics(runtime, style).mood;
    const float poseReadabilityBias = runtime.poseAdapterProfile.readabilityBias;
    const float assetOverlayWeight =
        runtime.assetNodeBindingProfile.overlayEntry.resolved
            ? runtime.assetNodeBindingProfile.overlayEntry.bindingWeight
            : 0.0f;
    const float registryOverlayWeight =
        runtime.modelNodeRegistryProfile.overlayEntry.resolved
            ? runtime.modelNodeRegistryProfile.overlayEntry.registryWeight
            : 0.0f;
    const auto& finalTargetResolver = runtime.assetNodeTargetResolverProfile;
    const auto& matchQuery = runtime.assetNodeMatchQueryProfile;
    const float overlayIdentitySignal =
        ResolveNodeSourceConfidence(finalTargetResolver.overlayEntry.sourceTag) *
        std::min(
            1.0f,
            std::max(
            ResolveNodePathSignal(finalTargetResolver.overlayEntry.modelNodePath),
                ResolveNodePathSignal(finalTargetResolver.overlayEntry.assetNodePath)) +
                ResolveSelectorSignal(
                    finalTargetResolver.overlayEntry.selectorKey,
                    finalTargetResolver.overlayEntry.candidateNodeName) +
                ResolvePlanSignal(
                    matchQuery.overlayEntry.queryLocator,
                    matchQuery.overlayEntry.queryNodeLabel,
                    matchQuery.overlayEntry.queryConfidence));
    const auto& assetTargetResolver = runtime.assetNodeTargetResolverProfile;
    const float transformOverlayWeight = assetTargetResolver.overlayEntry.resolved
        ? assetTargetResolver.overlayEntry.resolvedWeight
        : 0.0f;
    const float overlayAlphaScale =
        1.0f + poseReadabilityBias * 0.10f + registryOverlayWeight * 0.08f + assetOverlayWeight * 0.06f +
        transformOverlayWeight * 0.05f + overlayIdentitySignal * 0.06f;
    const float overlayStrokeScale =
        1.0f + poseReadabilityBias * 0.08f + registryOverlayWeight * 0.10f + assetOverlayWeight * 0.08f +
        transformOverlayWeight * 0.07f + overlayIdentitySignal * 0.08f;
    const float poseOverlayCenterX =
        runtime.modelNodeBindingProfile.overlayEntry.worldOffsetX * metrics.bodyWidth;
    const float poseOverlayCenterY =
        runtime.modelNodeBindingProfile.overlayEntry.worldOffsetY * metrics.bodyHeight;
    const float transformOverlayCenterX = assetTargetResolver.overlayEntry.resolved
        ? assetTargetResolver.overlayEntry.resolvedOffsetX * metrics.bodyWidth
        : 0.0f;
    const float transformOverlayCenterY = assetTargetResolver.overlayEntry.resolved
        ? assetTargetResolver.overlayEntry.resolvedOffsetY * metrics.bodyHeight
        : 0.0f;
    scene.overlayAnchor = Gdiplus::PointF(
        scene.centerX + poseOverlayCenterX + transformOverlayCenterX +
            metrics.bodyWidth * overlayIdentitySignal * 0.012f * runtime.facingSign,
        scene.centerY + poseOverlayCenterY + transformOverlayCenterY -
            metrics.bodyHeight * overlayIdentitySignal * 0.010f);
    scene.overlayAnchorScale = assetTargetResolver.overlayEntry.resolved
        ? assetTargetResolver.overlayEntry.resolvedScale
        : 1.0f;
    scene.actionOverlay.accentColor = profile.overlayAccentColor;

    if (runtime.click) {
        const float ringSize = std::max(scene.headRect.Width, scene.headRect.Height) *
            (style.clickRingScale + profile.actionIntensity * style.clickRingIntensityScale);
        scene.actionOverlay.clickRingVisible = true;
        scene.actionOverlay.clickRingStrokeWidth =
            (2.2f + profile.actionIntensity * 1.4f) * overlayStrokeScale;
        scene.actionOverlay.clickRingAlpha =
            ClampAlpha((205.0f + profile.actionIntensity * 36.0f) * mood.clickRingAlphaScale * overlayAlphaScale);
        scene.actionOverlay.clickRingRect = MakeCenteredRect(
            scene.overlayAnchor.X,
            scene.headRect.Y + scene.headRect.Height * style.clickRingCenterYRatio + poseOverlayCenterY * 0.45f +
                transformOverlayCenterY * 0.30f,
            ringSize,
            ringSize * style.clickRingHeightScale);
    }

    if (runtime.hold) {
        scene.actionOverlay.holdBandVisible = true;
        scene.actionOverlay.holdBandAlpha =
            ClampAlpha((145.0f + profile.actionIntensity * 78.0f) * mood.holdBandAlphaScale * overlayAlphaScale);
        scene.actionOverlay.holdBandRect = Gdiplus::RectF(
            scene.leftHandRect.X + scene.leftHandRect.Width * style.holdBandInsetRatio,
            std::min(scene.leftHandRect.Y, scene.rightHandRect.Y) + metrics.bodyHeight * style.holdBandYOffsetRatio +
                poseOverlayCenterY * 0.35f + transformOverlayCenterY * 0.25f,
            (scene.rightHandRect.GetRight() - scene.leftHandRect.X) - scene.leftHandRect.Width * style.holdBandWidthInsetRatio,
            metrics.bodyHeight * style.holdBandHeightRatio);
    }

    if (runtime.scroll) {
        scene.actionOverlay.scrollArcVisible = true;
        scene.actionOverlay.scrollArcStrokeWidth =
            (3.0f + profile.scrollIntensity * 1.6f) * overlayStrokeScale;
        scene.actionOverlay.scrollArcAlpha =
            ClampAlpha((180.0f + profile.scrollIntensity * 56.0f) * mood.scrollArcAlphaScale * overlayAlphaScale);
        scene.actionOverlay.scrollArcRect = MakeCenteredRect(
            scene.overlayAnchor.X,
            scene.centerY - metrics.bodyHeight * style.scrollArcCenterYRatio + poseOverlayCenterY * 0.30f +
                transformOverlayCenterY * 0.25f,
            metrics.bodyWidth * style.scrollArcWidthRatio,
            metrics.bodyHeight * style.scrollArcHeightRatio);
        scene.actionOverlay.scrollArcStartDeg = runtime.scrollSignedIntensity >= 0.0f ? 204.0f : 24.0f;
        scene.actionOverlay.scrollArcSweepDeg = runtime.scrollSignedIntensity >= 0.0f ? 148.0f : -148.0f;
    }

    if (runtime.drag) {
        scene.actionOverlay.dragLineVisible = true;
        scene.actionOverlay.dragLineStrokeWidth =
            (2.4f + profile.actionIntensity * 1.2f) * overlayStrokeScale;
        scene.actionOverlay.dragLineAlpha =
            ClampAlpha((178.0f + profile.actionIntensity * 52.0f) * mood.dragLineAlphaScale * overlayAlphaScale);
        scene.actionOverlay.dragLineStart = Gdiplus::PointF(
            scene.overlayAnchor.X - runtime.facingSign * metrics.bodyWidth * style.dragLineStartXRatio,
            scene.centerY - metrics.bodyHeight * style.dragLineStartYRatio + poseOverlayCenterY * 0.45f +
                transformOverlayCenterY * 0.30f);
        scene.actionOverlay.dragLineEnd = Gdiplus::PointF(
            scene.overlayAnchor.X + runtime.facingSign * metrics.bodyWidth * style.dragLineEndXRatio,
            scene.centerY - metrics.bodyHeight * style.dragLineEndYRatio + poseOverlayCenterY * 0.25f +
                transformOverlayCenterY * 0.20f);
    }

    if (runtime.follow) {
        scene.actionOverlay.followTrailVisible = true;
        scene.actionOverlay.followTrailBaseAlpha =
            ClampAlpha((150.0f + profile.actionIntensity * 68.0f) * mood.followTrailAlphaScale * overlayAlphaScale);
        const float trailBaseX =
            scene.overlayAnchor.X - runtime.facingSign * metrics.bodyWidth * style.followTrailBaseXRatio;
        const float trailBaseY =
            scene.centerY + metrics.bodyHeight * style.followTrailBaseYRatio + poseOverlayCenterY * 0.25f +
            transformOverlayCenterY * 0.20f;
        for (size_t i = 0; i < scene.actionOverlay.followTrailRects.size(); ++i) {
            const float scale = 1.0f - static_cast<float>(i) * style.followTrailShrinkPerStep;
            scene.actionOverlay.followTrailRects[i] = MakeCenteredRect(
                trailBaseX - runtime.facingSign * metrics.bodyWidth * style.followTrailStepXRatio * static_cast<float>(i),
                trailBaseY + metrics.bodyHeight * style.followTrailStepYRatio * static_cast<float>(i),
                metrics.bodyWidth * style.followTrailWidthRatio * scale,
                metrics.bodyHeight * style.followTrailHeightRatio * scale);
        }
    }
}

} // namespace mousefx::windows

#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAppearanceSemantics.h"
#include "Platform/windows/Pet/Win32MouseCompanionRealRendererAdornmentBuilder.h"

#include <algorithm>
#include <cmath>

namespace mousefx::windows {
namespace {

std::array<Gdiplus::PointF, 5> BuildStarPoints(
    float centerX,
    float centerY,
    float outerRadius,
    float innerRadius,
    float innerYOffsetRatio) {
    std::array<Gdiplus::PointF, 5> points{};
    for (size_t i = 0; i < points.size(); ++i) {
        const float angle = -3.1415926f * 0.5f + static_cast<float>(i) * (3.1415926f * 2.0f / 5.0f);
        const float x = centerX + std::cos(angle) * outerRadius;
        const float y = centerY + std::sin(angle) * outerRadius;
        points[i] = Gdiplus::PointF(x, y - innerRadius * innerYOffsetRatio);
    }
    return points;
}

std::array<Gdiplus::PointF, 6> BuildMoonPoints(
    const Gdiplus::RectF& bounds,
    float insetRatio) {
    const float left = bounds.X;
    const float right = bounds.GetRight();
    const float top = bounds.Y;
    const float bottom = bounds.GetBottom();
    const float midX = bounds.X + bounds.Width * (0.48f + insetRatio * 0.10f);
    const float innerX = bounds.X + bounds.Width * (0.40f + insetRatio);
    const float upperY = bounds.Y + bounds.Height * 0.18f;
    const float lowerY = bounds.Y + bounds.Height * 0.82f;
    const float midY = bounds.Y + bounds.Height * 0.50f;
    return {{
        Gdiplus::PointF(left, lowerY),
        Gdiplus::PointF(bounds.X + bounds.Width * 0.16f, top),
        Gdiplus::PointF(right, upperY),
        Gdiplus::PointF(midX, midY),
        Gdiplus::PointF(right, lowerY),
        Gdiplus::PointF(innerX, bottom),
    }};
}

std::array<Gdiplus::PointF, 4> BuildLeafPoints(const Gdiplus::RectF& bounds) {
    const float centerX = bounds.X + bounds.Width * 0.50f;
    return {{
        Gdiplus::PointF(centerX, bounds.Y),
        Gdiplus::PointF(bounds.GetRight(), bounds.Y + bounds.Height * 0.48f),
        Gdiplus::PointF(centerX, bounds.GetBottom()),
        Gdiplus::PointF(bounds.X, bounds.Y + bounds.Height * 0.48f),
    }};
}

std::array<Gdiplus::PointF, 4> BuildRibbonWing(
    const Gdiplus::RectF& bounds,
    bool leftWing,
    float insetScale) {
    const float tipX = leftWing ? bounds.X : bounds.GetRight();
    const float innerX = leftWing
        ? bounds.X + bounds.Width * (0.56f + insetScale * 0.10f)
        : bounds.X + bounds.Width * (0.44f - insetScale * 0.10f);
    const float midY = bounds.Y + bounds.Height * 0.50f;
    return {{
        Gdiplus::PointF(innerX, bounds.Y),
        Gdiplus::PointF(tipX, bounds.Y + bounds.Height * 0.26f),
        Gdiplus::PointF(innerX, midY),
        Gdiplus::PointF(tipX, bounds.GetBottom()),
    }};
}

Gdiplus::RectF ScaleRectFromCenter(
    const Gdiplus::RectF& rect,
    float widthScale,
    float heightScale) {
    const float newWidth = rect.Width * widthScale;
    const float newHeight = rect.Height * heightScale;
    return Gdiplus::RectF(
        rect.X + (rect.Width - newWidth) * 0.5f,
        rect.Y + (rect.Height - newHeight) * 0.5f,
        newWidth,
        newHeight);
}

float ResolveAveragePoseX(
    const MouseCompanionPetPoseSample* a,
    const MouseCompanionPetPoseSample* b) {
    if (a && b) {
        return (a->position[0] + b->position[0]) * 0.5f;
    }
    if (a) {
        return a->position[0];
    }
    if (b) {
        return b->position[0];
    }
    return 0.0f;
}

float ResolveAveragePoseY(
    const MouseCompanionPetPoseSample* a,
    const MouseCompanionPetPoseSample* b) {
    if (a && b) {
        return (a->position[1] + b->position[1]) * 0.5f;
    }
    if (a) {
        return a->position[1];
    }
    if (b) {
        return b->position[1];
    }
    return 0.0f;
}

} // namespace

void BuildWin32MouseCompanionRealRendererAdornment(
    const Win32MouseCompanionRealRendererSceneRuntime& runtime,
    const Win32MouseCompanionRealRendererStyleProfile& style,
    const Win32MouseCompanionRealRendererLayoutMetrics& metrics,
    Win32MouseCompanionRealRendererScene& scene) {
    const float badgeY = scene.pedestalRect.GetBottom() + metrics.bodyHeight * style.badgeYOffsetRatio;
    const float badgeW = metrics.bodyWidth * style.badgeWidthRatio;
    const float badgeH = metrics.bodyHeight * style.badgeHeightRatio;
    for (size_t i = 0; i < scene.laneBadgeRects.size(); ++i) {
        const float x = scene.centerX - badgeW * style.badgeStartXScale + static_cast<float>(i) * (badgeW * style.badgeGapScale);
        scene.laneBadgeRects[i] = Gdiplus::RectF(x, badgeY, badgeW, badgeH);
    }
    scene.laneReady = {
        runtime.assets->modelReady,
        runtime.assets->actionLibraryReady,
        runtime.assets->appearanceProfileReady,
    };

    const float poseAdapterInfluence =
        ResolveWin32MouseCompanionRealRendererPoseAdapterInfluence(runtime);
    const float poseHandReachX = ResolveAveragePoseX(runtime.leftHandPose, runtime.rightHandPose);
    const float poseHandLiftY = ResolveAveragePoseY(runtime.leftHandPose, runtime.rightHandPose);
    const float poseLegReachX = ResolveAveragePoseX(runtime.leftLegPose, runtime.rightLegPose);
    const float poseLegLiftY = ResolveAveragePoseY(runtime.leftLegPose, runtime.rightLegPose);
    const float poseAdornmentX =
        (poseHandReachX * metrics.bodyWidth * 0.020f + poseLegReachX * metrics.bodyWidth * 0.015f) *
        poseAdapterInfluence;
    const float poseAdornmentY =
        (-poseHandLiftY * metrics.bodyHeight * 0.018f - poseLegLiftY * metrics.bodyHeight * 0.010f) *
        poseAdapterInfluence;
    scene.poseBadgeAlpha = 196.0f + poseAdapterInfluence * 59.0f;
    scene.accessoryAlphaScale = 1.0f + poseAdapterInfluence * 0.10f;
    scene.accessoryStrokeWidth = 1.0f + poseAdapterInfluence * 0.16f;

    scene.poseBadgeVisible =
        runtime.poseBindingConfigured ||
        runtime.sceneRuntimeAdapterMode != "runtime_only" ||
        runtime.sceneRuntimePoseSampleCount > 0;
    scene.poseBadgeRect = Gdiplus::RectF(
        scene.headRect.GetRight() - scene.headRect.Width * style.poseBadgeXRatio,
        scene.headRect.Y - scene.headRect.Height * style.poseBadgeYRatio + poseAdornmentY * 0.35f,
        scene.headRect.Width * style.poseBadgeSizeRatio,
        scene.headRect.Width * style.poseBadgeSizeRatio);

    scene.accessoryVisible = !runtime.assets->appearanceAccessoryIds.empty();
    if (!scene.accessoryVisible) {
        scene.accessoryShape = Win32MouseCompanionRealRendererAccessoryShape::None;
        return;
    }
    const auto semantics = BuildWin32MouseCompanionRealRendererAppearanceSemantics(runtime, style);
    const auto& adornment = semantics.adornment;

    float centerX =
        scene.headRect.GetRight() - scene.headRect.Width * (style.accessoryXRatio - adornment.xOffsetRatio) +
        poseAdornmentX;
    float centerY =
        scene.headRect.Y + scene.headRect.Height * (style.accessoryYRatio + adornment.yOffsetRatio) + poseAdornmentY;
    const float baseWidth = scene.headRect.Width * style.accessoryOuterRatio * 2.0f * adornment.widthScale;
    const float baseHeight = scene.headRect.Width * style.accessoryOuterRatio * 2.0f * adornment.heightScale;
    const float actionIntensity = std::clamp(runtime.actionIntensity + runtime.reactiveActionIntensity * 0.6f, 0.0f, 1.0f);
    const float followLiftPx =
        runtime.follow ? style.accessoryMotionMaxPx * style.accessoryFollowLiftScale * adornment.followLiftScale : 0.0f;
    const float scrollLiftPx =
        runtime.scroll ? style.accessoryMotionMaxPx * style.accessoryScrollLiftScale * adornment.scrollLiftScale : 0.0f;
    const float dragShiftPx = runtime.drag
        ? style.accessoryMotionMaxPx * style.accessoryDragShiftScale * adornment.dragShiftScale * runtime.facingSign
        : 0.0f;
    const float clickBouncePx =
        runtime.click ? style.accessoryMotionMaxPx * style.accessoryClickBounceScale * adornment.clickBounceScale : 0.0f;
    const float holdSettlePx =
        runtime.hold ? style.accessoryMotionMaxPx * style.accessoryHoldSettleScale * adornment.holdSettleScale : 0.0f;

    if (adornment.family == Win32MouseCompanionRealRendererAppearanceAccessoryFamily::Moon) {
        scene.accessoryShape = Win32MouseCompanionRealRendererAccessoryShape::Moon;
        centerX += dragShiftPx * 0.35f;
        centerY -= followLiftPx + scrollLiftPx + clickBouncePx * 0.2f;
        scene.accessoryBounds = ScaleRectFromCenter(
            Gdiplus::RectF(
            centerX - baseWidth * 0.5f,
            centerY - baseHeight * 0.5f,
            baseWidth,
            baseHeight),
            runtime.follow ? style.moonFollowScale : 1.0f,
            runtime.scroll ? style.moonScrollScale : 1.0f);
        scene.accessoryMoon = BuildMoonPoints(scene.accessoryBounds, style.accessoryMoonInsetRatio);
        scene.accessoryMoonInsetRect = Gdiplus::RectF(
            scene.accessoryBounds.X + scene.accessoryBounds.Width * 0.38f,
            scene.accessoryBounds.Y + scene.accessoryBounds.Height * 0.20f,
            scene.accessoryBounds.Width * style.accessoryMoonHighlightWidthScale,
            scene.accessoryBounds.Height * style.accessoryMoonHighlightHeightScale);
        return;
    }
    if (adornment.family == Win32MouseCompanionRealRendererAppearanceAccessoryFamily::Leaf) {
        scene.accessoryShape = Win32MouseCompanionRealRendererAccessoryShape::Leaf;
        centerX += dragShiftPx;
        centerY -= followLiftPx * 0.25f + scrollLiftPx * 0.4f;
        scene.accessoryBounds = Gdiplus::RectF(
            centerX - baseWidth * 0.5f,
            centerY - baseHeight * 0.5f,
            baseWidth,
            baseHeight);
        const float dragMix = runtime.drag ? actionIntensity : 0.0f;
        const float scrollMix = runtime.scroll ? actionIntensity : 0.0f;
        const float leafWidthScale =
            1.0f
            + dragMix * (style.leafDragWidthScale - 1.0f)
            + scrollMix * (style.leafScrollWidthScale - 1.0f);
        const float leafHeightScale =
            1.0f
            + dragMix * (style.leafDragHeightScale - 1.0f)
            + scrollMix * (style.leafScrollHeightScale - 1.0f);
        const Gdiplus::RectF leafBounds(
            scene.accessoryBounds.X + scene.accessoryBounds.Width * (1.0f - style.accessoryLeafWidthScale * leafWidthScale) * 0.5f,
            scene.accessoryBounds.Y + scene.accessoryBounds.Height * (1.0f - style.accessoryLeafHeightScale * leafHeightScale) * 0.5f,
            scene.accessoryBounds.Width * style.accessoryLeafWidthScale * leafWidthScale,
            scene.accessoryBounds.Height * style.accessoryLeafHeightScale * leafHeightScale);
        scene.accessoryLeaf = BuildLeafPoints(leafBounds);
        scene.accessoryLeafVeinStart = Gdiplus::PointF(
            leafBounds.X + leafBounds.Width * 0.50f,
            leafBounds.Y + leafBounds.Height * style.accessoryLeafVeinInsetRatio);
        scene.accessoryLeafVeinEnd = Gdiplus::PointF(
            leafBounds.X + leafBounds.Width * 0.50f,
            leafBounds.GetBottom() - leafBounds.Height * style.accessoryLeafVeinInsetRatio);
        return;
    }
    if (adornment.family == Win32MouseCompanionRealRendererAppearanceAccessoryFamily::RibbonBow) {
        scene.accessoryShape = Win32MouseCompanionRealRendererAccessoryShape::RibbonBow;
        centerX += dragShiftPx * 0.20f;
        centerY -= followLiftPx * 0.15f + clickBouncePx - holdSettlePx;
        scene.accessoryBounds = ScaleRectFromCenter(
            Gdiplus::RectF(
            centerX - baseWidth * 0.5f,
            centerY - baseHeight * 0.5f,
            baseWidth,
            baseHeight),
            runtime.click ? style.ribbonClickWidthScale : (runtime.hold ? style.ribbonHoldWidthScale : 1.0f),
            runtime.click ? style.ribbonClickHeightScale : (runtime.hold ? style.ribbonHoldHeightScale : 1.0f));
        const Gdiplus::RectF ribbonBounds(
            scene.accessoryBounds.X + scene.accessoryBounds.Width * (1.0f - style.accessoryRibbonWidthScale) * 0.5f,
            scene.accessoryBounds.Y + scene.accessoryBounds.Height * (1.0f - style.accessoryRibbonHeightScale) * 0.5f,
            scene.accessoryBounds.Width * style.accessoryRibbonWidthScale,
            scene.accessoryBounds.Height * style.accessoryRibbonHeightScale);
        scene.accessoryRibbonLeft = BuildRibbonWing(ribbonBounds, true, style.accessoryRibbonWingInsetScale);
        scene.accessoryRibbonRight = BuildRibbonWing(ribbonBounds, false, style.accessoryRibbonWingInsetScale);
        scene.accessoryRibbonCenter = Gdiplus::RectF(
            ribbonBounds.X + ribbonBounds.Width * (0.5f - style.accessoryRibbonCenterScale * 0.5f),
            ribbonBounds.Y + ribbonBounds.Height * (0.5f - style.accessoryRibbonCenterScale * 0.5f),
            ribbonBounds.Width * style.accessoryRibbonCenterScale,
            ribbonBounds.Height * style.accessoryRibbonCenterScale);
        const float foldInset = ribbonBounds.Width * style.accessoryRibbonFoldInsetRatio;
        scene.accessoryRibbonLeftFoldStart = Gdiplus::PointF(
            scene.accessoryRibbonCenter.X,
            scene.accessoryRibbonCenter.Y + scene.accessoryRibbonCenter.Height * 0.52f);
        scene.accessoryRibbonLeftFoldEnd = Gdiplus::PointF(
            ribbonBounds.X + foldInset,
            ribbonBounds.Y + ribbonBounds.Height * 0.58f);
        scene.accessoryRibbonRightFoldStart = Gdiplus::PointF(
            scene.accessoryRibbonCenter.GetRight(),
            scene.accessoryRibbonCenter.Y + scene.accessoryRibbonCenter.Height * 0.52f);
        scene.accessoryRibbonRightFoldEnd = Gdiplus::PointF(
            ribbonBounds.GetRight() - foldInset,
            ribbonBounds.Y + ribbonBounds.Height * 0.58f);
        return;
    }

    scene.accessoryShape = Win32MouseCompanionRealRendererAccessoryShape::Star;
    scene.accessoryBounds = Gdiplus::RectF(
        centerX - baseWidth * 0.5f,
        centerY - baseHeight * 0.5f,
        baseWidth,
        baseHeight);
    scene.accessoryStar = BuildStarPoints(
        centerX,
        centerY,
        scene.headRect.Width * style.accessoryOuterRatio,
        scene.headRect.Width * style.accessoryInnerRatio,
        style.accessoryStarInnerYOffsetRatio);
}

} // namespace mousefx::windows

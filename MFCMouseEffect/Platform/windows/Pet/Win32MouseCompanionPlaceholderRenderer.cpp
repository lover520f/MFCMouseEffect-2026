#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderRenderer.h"
#include "Platform/windows/Pet/Win32MouseCompanionPlaceholderScene.h"

#include <algorithm>
#include <cmath>

namespace mousefx::windows {
namespace {

Gdiplus::Color BlendColor(
    const Gdiplus::Color& base,
    const Gdiplus::Color& target,
    float t) {
    const float clamped = std::clamp(t, 0.0f, 1.0f);
    const auto lerp = [clamped](BYTE a, BYTE b) -> BYTE {
        return static_cast<BYTE>(std::lround(
            static_cast<double>(a) + (static_cast<double>(b) - static_cast<double>(a)) * clamped));
    };
    return Gdiplus::Color(
        lerp(base.GetA(), target.GetA()),
        lerp(base.GetR(), target.GetR()),
        lerp(base.GetG(), target.GetG()),
        lerp(base.GetB(), target.GetB()));
}

void FillRoundedEllipse(
    Gdiplus::Graphics* graphics,
    const Gdiplus::RectF& rect,
    const Gdiplus::Color& fill,
    const Gdiplus::Color& stroke,
    float strokeWidth) {
    if (!graphics) {
        return;
    }
    Gdiplus::SolidBrush brush(fill);
    Gdiplus::Pen pen(stroke, strokeWidth);
    pen.SetAlignment(Gdiplus::PenAlignmentCenter);
    graphics->FillEllipse(&brush, rect);
    graphics->DrawEllipse(&pen, rect);
}

void FillRotatedRoundedEllipse(
    Gdiplus::Graphics* graphics,
    const Gdiplus::RectF& rect,
    float rotationDeg,
    const Gdiplus::Color& fill,
    const Gdiplus::Color& stroke,
    float strokeWidth) {
    if (!graphics) {
        return;
    }
    const Gdiplus::GraphicsState saved = graphics->Save();
    const float cx = rect.X + rect.Width * 0.5f;
    const float cy = rect.Y + rect.Height * 0.5f;
    graphics->TranslateTransform(cx, cy);
    graphics->RotateTransform(rotationDeg);
    graphics->TranslateTransform(-cx, -cy);
    FillRoundedEllipse(graphics, rect, fill, stroke, strokeWidth);
    graphics->Restore(saved);
}

void FillEar(
    Gdiplus::Graphics* graphics,
    const Gdiplus::PointF* points,
    int pointCount,
    const Gdiplus::Color& fill,
    const Gdiplus::Color& inner) {
    if (!graphics || !points || pointCount < 3) {
        return;
    }
    Gdiplus::GraphicsPath path;
    path.AddClosedCurve(points, pointCount);
    Gdiplus::SolidBrush fillBrush(fill);
    graphics->FillPath(&fillBrush, &path);

    Gdiplus::Matrix matrix;
    matrix.Translate(0.0f, 6.0f);
    path.Transform(&matrix);
    Gdiplus::SolidBrush innerBrush(inner);
    graphics->FillPath(&innerBrush, &path);
}

void DrawEarLayer(
    Gdiplus::Graphics* graphics,
    const std::array<Gdiplus::PointF, 4>& points,
    const Gdiplus::Color& fill,
    const Gdiplus::Color& inner) {
    FillEar(graphics, points.data(), static_cast<int>(points.size()), fill, inner);
}

void DrawLinePair(
    Gdiplus::Graphics* graphics,
    const std::array<Gdiplus::PointF, 2>& points,
    const Gdiplus::Color& color,
    float width) {
    if (!graphics) {
        return;
    }
    Gdiplus::Pen pen(color, width);
    pen.SetStartCap(Gdiplus::LineCapRound);
    pen.SetEndCap(Gdiplus::LineCapRound);
    graphics->DrawLine(&pen, points[0], points[1]);
}

void FillSimpleEllipse(
    Gdiplus::Graphics* graphics,
    const Gdiplus::RectF& rect,
    const Gdiplus::Color& fill) {
    if (!graphics) {
        return;
    }
    Gdiplus::SolidBrush brush(fill);
    graphics->FillEllipse(&brush, rect);
}

void DrawAdornmentDust(
    Gdiplus::Graphics* graphics,
    const std::array<Gdiplus::RectF, 3>& dustRects,
    const Gdiplus::Color& dustFill,
    float dustAlpha) {
    if (!graphics) {
        return;
    }
    const BYTE alpha = static_cast<BYTE>(std::clamp(dustAlpha, 0.0f, 255.0f));
    Gdiplus::SolidBrush brush(Gdiplus::Color(
        alpha,
        dustFill.GetR(),
        dustFill.GetG(),
        dustFill.GetB()));
    for (const auto& rect : dustRects) {
        graphics->FillEllipse(&brush, rect);
    }
}

void DrawAccessoryStar(
    Gdiplus::Graphics* graphics,
    const std::array<Gdiplus::PointF, 5>& points,
    const Gdiplus::Color& fill,
    const Gdiplus::Color& stroke) {
    if (!graphics) {
        return;
    }
    Gdiplus::GraphicsPath path;
    path.AddClosedCurve(points.data(), static_cast<int>(points.size()));
    Gdiplus::SolidBrush brush(fill);
    Gdiplus::Pen pen(stroke, 1.0f);
    graphics->FillPath(&brush, &path);
    graphics->DrawPath(&pen, &path);
}

} // namespace

void Win32MouseCompanionPlaceholderRenderer::Render(
    const Win32MouseCompanionVisualState& state,
    Gdiplus::Graphics* graphics,
    int width,
    int height) const {
    if (!graphics || width <= 0 || height <= 0) {
        return;
    }

    graphics->SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    graphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    const Win32MouseCompanionPlaceholderScene scene =
        BuildWin32MouseCompanionPlaceholderScene(state, width, height);

    Gdiplus::SolidBrush glowBrush(Gdiplus::Color(
        static_cast<BYTE>(std::clamp(scene.glowAlpha, 0.0f, 255.0f)),
        scene.accent.GetR(),
        scene.accent.GetG(),
        scene.accent.GetB()));
    Gdiplus::SolidBrush shadowBrush(scene.shadowColor);
    graphics->FillEllipse(&shadowBrush, scene.shadowRect);
    graphics->FillEllipse(
        &glowBrush,
        Gdiplus::RectF(
            scene.centerX - scene.bodyRect.Width * 1.2f + scene.bodyLeanPx * 0.18f,
            scene.bodyRect.Y - scene.bodyRect.Height * 0.05f,
            scene.bodyRect.Width * 2.4f,
            scene.bodyRect.Height * 1.9f));
    DrawAdornmentDust(
        graphics,
        scene.adornment.dustRects,
        scene.adornment.dustFill,
        scene.adornment.dustAlpha);
    FillRoundedEllipse(
        graphics,
        scene.gait.neckBridgeRect,
        scene.gait.neckBridgeFill,
        scene.bodyStroke,
        1.0f + scene.rhythm.strideAccent * 0.15f);
    FillRoundedEllipse(graphics, scene.tailRect, scene.tailFill, scene.bodyStroke, 1.2f);
    FillSimpleEllipse(graphics, scene.tailTipRect, scene.tailTipFill);

    if (scene.frontSideIsLeft) {
        DrawEarLayer(graphics, scene.rightEar, scene.headFillRear, scene.earInnerRear);
        DrawEarLayer(graphics, scene.leftEar, scene.headFill, scene.earInner);
    } else {
        DrawEarLayer(graphics, scene.leftEar, scene.headFillRear, scene.earInnerRear);
        DrawEarLayer(graphics, scene.rightEar, scene.headFill, scene.earInner);
    }
    FillSimpleEllipse(graphics, scene.silhouette.leftEarRootRect, scene.silhouette.rootFill);
    FillSimpleEllipse(graphics, scene.silhouette.rightEarRootRect, scene.silhouette.rootFill);
    FillRotatedRoundedEllipse(graphics, scene.bodyRect, scene.bodyTiltDeg, scene.bodyFill, scene.bodyStroke, 2.0f);
    FillSimpleEllipse(
        graphics,
        scene.silhouette.frontDepthPatchRect,
        scene.frontSideIsLeft ? scene.silhouette.patchFill : scene.silhouette.rearPatchFill);
    FillSimpleEllipse(
        graphics,
        scene.silhouette.rearDepthPatchRect,
        scene.frontSideIsLeft ? scene.silhouette.rearPatchFill : scene.silhouette.patchFill);
    FillRoundedEllipse(
        graphics,
        scene.gait.backStripeRect,
        scene.frontSideIsLeft ? scene.gait.stripeFill : scene.gait.stripeRearFill,
        scene.bodyStroke,
        0.8f + scene.rhythm.strideAccent * 0.08f);
    FillRoundedEllipse(
        graphics,
        scene.gait.bellyStripeRect,
        scene.frontSideIsLeft ? scene.gait.stripeRearFill : scene.gait.stripeFill,
        scene.bodyStroke,
        0.8f + scene.rhythm.strideAccent * 0.06f);
    FillRoundedEllipse(
        graphics,
        scene.gait.hipPatchRect,
        scene.gait.stripeRearFill,
        scene.bodyStroke,
        0.8f);
    FillSimpleEllipse(graphics, scene.silhouette.shoulderPatchRect, scene.silhouette.patchFill);
    FillSimpleEllipse(graphics, scene.silhouette.hipPatchRect, scene.silhouette.rearPatchFill);
    FillRoundedEllipse(
        graphics,
        scene.adornment.collarRect,
        scene.adornment.collarFill,
        scene.adornment.collarStroke,
        1.0f);
    FillRoundedEllipse(graphics, scene.chestRect, scene.chestFill, scene.bodyStroke, 1.1f);
    FillRoundedEllipse(graphics, scene.headRect, scene.headFill, scene.bodyStroke, 2.0f);
    FillSimpleEllipse(graphics, scene.adornment.charmRect, scene.adornment.charmFill);
    FillSimpleEllipse(graphics, scene.silhouette.tailRootRect, scene.silhouette.rootFill);
    if (scene.frontSideIsLeft) {
        FillRoundedEllipse(graphics, scene.rightLegRect, scene.headFillRear, scene.bodyStroke, 1.4f);
        FillRoundedEllipse(graphics, scene.rightHandRect, scene.headFillRear, scene.bodyStroke, 1.6f);
        FillRoundedEllipse(graphics, scene.leftLegRect, scene.headFill, scene.bodyStroke, 1.4f);
        FillRoundedEllipse(graphics, scene.leftHandRect, scene.headFill, scene.bodyStroke, 1.6f);
    } else {
        FillRoundedEllipse(graphics, scene.leftLegRect, scene.headFillRear, scene.bodyStroke, 1.4f);
        FillRoundedEllipse(graphics, scene.leftHandRect, scene.headFillRear, scene.bodyStroke, 1.6f);
        FillRoundedEllipse(graphics, scene.rightLegRect, scene.headFill, scene.bodyStroke, 1.4f);
        FillRoundedEllipse(graphics, scene.rightHandRect, scene.headFill, scene.bodyStroke, 1.6f);
    }
    DrawLinePair(graphics, scene.gait.leftForeBridge, scene.gait.bridgeColor, scene.gait.bridgeWidth);
    DrawLinePair(graphics, scene.gait.rightForeBridge, scene.gait.bridgeColor, scene.gait.bridgeWidth);
    DrawLinePair(graphics, scene.gait.leftRearBridge, scene.gait.bridgeColor, scene.gait.bridgeWidth);
    DrawLinePair(graphics, scene.gait.rightRearBridge, scene.gait.bridgeColor, scene.gait.bridgeWidth);
    FillSimpleEllipse(graphics, scene.leftPawPadRect, scene.pawPadFill);
    FillSimpleEllipse(graphics, scene.rightPawPadRect, scene.pawPadFill);

    const float badgeSize = 7.0f;
    const float badgeTop = 10.0f;
    float badgeX = static_cast<float>(width) - 14.0f;
    if (scene.modelAssetAvailable) {
        Gdiplus::SolidBrush badge(Gdiplus::Color(220, 79, 193, 255));
        graphics->FillEllipse(&badge, Gdiplus::RectF(badgeX, badgeTop, badgeSize, badgeSize));
        badgeX -= 10.0f;
    }
    if (scene.actionLibraryAvailable) {
        Gdiplus::SolidBrush badge(Gdiplus::Color(220, 100, 208, 164));
        graphics->FillEllipse(&badge, Gdiplus::RectF(badgeX, badgeTop, badgeSize, badgeSize));
        badgeX -= 10.0f;
    }
    if (scene.poseBadgeVisible) {
        Gdiplus::SolidBrush badge(Gdiplus::Color(220, 255, 188, 96));
        graphics->FillEllipse(&badge, Gdiplus::RectF(badgeX, badgeTop, badgeSize, badgeSize));
    }

    if (scene.accessoryVisible) {
        if (scene.accessory.starVisible) {
            DrawAccessoryStar(
                graphics,
                scene.accessory.starPoints,
                scene.accessory.fill,
                scene.accessory.stroke);
        } else {
            FillRoundedEllipse(
                graphics,
                scene.accessory.anchorRect,
                scene.accessory.fill,
                scene.accessory.stroke,
                0.9f);
        }
        FillSimpleEllipse(graphics, scene.accessory.gemRect, scene.accessory.fill);
    }

    Gdiplus::SolidBrush eyeBrush(scene.eyeColor);
    Gdiplus::SolidBrush noseBrush(BlendColor(scene.blushColor, scene.mouthColor, 0.35f));
    Gdiplus::Pen mouthPen(scene.expression.browColor, scene.expression.mouthStrokeWidth);
    Gdiplus::SolidBrush blushBrush(scene.blushColor);
    Gdiplus::SolidBrush accentBrush(Gdiplus::Color(
        72,
        scene.accentGlow.GetR(),
        scene.accentGlow.GetG(),
        scene.accentGlow.GetB()));
    graphics->FillEllipse(
        &accentBrush,
        Gdiplus::RectF(
            scene.headRect.X + scene.headRect.Width * 0.16f,
            scene.headRect.Y + scene.headRect.Height * 0.24f,
            scene.headRect.Width * 0.68f,
            scene.headRect.Height * 0.44f));
    graphics->FillEllipse(&eyeBrush, scene.leftEyeRect);
    graphics->FillEllipse(&eyeBrush, scene.rightEyeRect);
    FillSimpleEllipse(graphics, scene.expression.leftPupilRect, scene.expression.pupilColor);
    FillSimpleEllipse(graphics, scene.expression.rightPupilRect, scene.expression.pupilColor);
    DrawLinePair(graphics, scene.expression.leftBrow, scene.expression.browColor, 1.3f);
    DrawLinePair(graphics, scene.expression.rightBrow, scene.expression.browColor, 1.3f);
    DrawLinePair(graphics, scene.leftWhiskerTop, scene.mouthColor, 1.1f);
    DrawLinePair(graphics, scene.leftWhiskerBottom, scene.mouthColor, 1.1f);
    DrawLinePair(graphics, scene.rightWhiskerTop, scene.mouthColor, 1.1f);
    DrawLinePair(graphics, scene.rightWhiskerBottom, scene.mouthColor, 1.1f);
    graphics->FillEllipse(&noseBrush, scene.noseRect);
    graphics->DrawArc(&mouthPen, scene.mouthRect, scene.expression.mouthStartDeg, scene.expression.mouthSweepDeg);
    graphics->FillEllipse(&blushBrush, scene.leftBlushRect);
    graphics->FillEllipse(&blushBrush, scene.rightBlushRect);
}

} // namespace mousefx::windows

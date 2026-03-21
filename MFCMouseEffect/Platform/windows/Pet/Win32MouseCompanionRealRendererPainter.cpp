#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPainter.h"

#include <algorithm>

namespace mousefx::windows {
namespace {

void FillEllipse(
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

void FillRoundedRect(
    Gdiplus::Graphics* graphics,
    const Gdiplus::RectF& rect,
    const Gdiplus::Color& fill,
    const Gdiplus::Color& stroke,
    float strokeWidth) {
    if (!graphics) {
        return;
    }
    Gdiplus::GraphicsPath path;
    const float radius = std::min(rect.Width, rect.Height) * 0.42f;
    path.AddArc(rect.X, rect.Y, radius, radius, 180.0f, 90.0f);
    path.AddArc(rect.GetRight() - radius, rect.Y, radius, radius, 270.0f, 90.0f);
    path.AddArc(rect.GetRight() - radius, rect.GetBottom() - radius, radius, radius, 0.0f, 90.0f);
    path.AddArc(rect.X, rect.GetBottom() - radius, radius, radius, 90.0f, 90.0f);
    path.CloseFigure();
    Gdiplus::SolidBrush brush(fill);
    Gdiplus::Pen pen(stroke, strokeWidth);
    graphics->FillPath(&brush, &path);
    graphics->DrawPath(&pen, &path);
}

void FillEar(
    Gdiplus::Graphics* graphics,
    const std::array<Gdiplus::PointF, 4>& points,
    const Gdiplus::Color& fill,
    const Gdiplus::Color& inner,
    const Gdiplus::Color& stroke) {
    if (!graphics) {
        return;
    }
    Gdiplus::GraphicsPath path;
    path.AddClosedCurve(points.data(), static_cast<int>(points.size()));
    Gdiplus::SolidBrush fillBrush(fill);
    Gdiplus::Pen pen(stroke, 1.4f);
    graphics->FillPath(&fillBrush, &path);
    graphics->DrawPath(&pen, &path);

    Gdiplus::GraphicsPath innerPath;
    const Gdiplus::PointF innerPoints[4] = {
        Gdiplus::PointF(points[0].X, points[0].Y + 3.0f),
        Gdiplus::PointF((points[0].X + points[1].X) * 0.5f, (points[0].Y + points[1].Y) * 0.5f + 4.0f),
        Gdiplus::PointF(points[2].X, points[2].Y + 8.0f),
        Gdiplus::PointF((points[0].X + points[3].X) * 0.5f, (points[0].Y + points[3].Y) * 0.5f + 4.0f),
    };
    innerPath.AddClosedCurve(innerPoints, 4);
    Gdiplus::SolidBrush innerBrush(inner);
    graphics->FillPath(&innerBrush, &innerPath);
}

void DrawStar(
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
    Gdiplus::Pen pen(stroke, 1.1f);
    graphics->FillPath(&brush, &path);
    graphics->DrawPath(&pen, &path);
}

void DrawActionOverlay(
    Gdiplus::Graphics* graphics,
    const Win32MouseCompanionRealRendererActionOverlay& overlay,
    const Gdiplus::Color& stroke) {
    if (!graphics) {
        return;
    }

    if (overlay.followTrailVisible) {
        for (size_t i = 0; i < overlay.followTrailRects.size(); ++i) {
            const BYTE alpha = static_cast<BYTE>(std::max(48, 150 - static_cast<int>(i) * 34));
            FillEllipse(
                graphics,
                overlay.followTrailRects[i],
                Gdiplus::Color(alpha, overlay.accentColor.GetR(), overlay.accentColor.GetG(), overlay.accentColor.GetB()),
                Gdiplus::Color(0, 0, 0, 0),
                0.0f);
        }
    }

    if (overlay.scrollArcVisible) {
        Gdiplus::Pen arcPen(overlay.accentColor, 3.0f);
        arcPen.SetStartCap(Gdiplus::LineCapRound);
        arcPen.SetEndCap(Gdiplus::LineCapRound);
        graphics->DrawArc(
            &arcPen,
            overlay.scrollArcRect,
            overlay.scrollArcStartDeg,
            overlay.scrollArcSweepDeg);
    }

    if (overlay.dragLineVisible) {
        Gdiplus::Pen dashPen(overlay.accentColor, 2.4f);
        dashPen.SetStartCap(Gdiplus::LineCapRound);
        dashPen.SetEndCap(Gdiplus::LineCapRound);
        const Gdiplus::REAL dashPattern[2] = {4.0f, 3.0f};
        dashPen.SetDashPattern(dashPattern, 2);
        graphics->DrawLine(&dashPen, overlay.dragLineStart, overlay.dragLineEnd);
    }

    if (overlay.clickRingVisible) {
        Gdiplus::Pen ringPen(overlay.accentColor, 2.2f);
        ringPen.SetAlignment(Gdiplus::PenAlignmentCenter);
        graphics->DrawEllipse(&ringPen, overlay.clickRingRect);
    }

    if (overlay.holdBandVisible) {
        FillRoundedRect(
            graphics,
            overlay.holdBandRect,
            Gdiplus::Color(150, overlay.accentColor.GetR(), overlay.accentColor.GetG(), overlay.accentColor.GetB()),
            stroke,
            1.0f);
    }
}

} // namespace

void Win32MouseCompanionRealRendererPainter::Paint(
    const Win32MouseCompanionRealRendererScene& scene,
    Gdiplus::Graphics* graphics,
    int width,
    int height) const {
    if (!graphics || width <= 0 || height <= 0) {
        return;
    }

    const BYTE glowAlpha = static_cast<BYTE>(std::clamp(scene.glowAlpha, 0.0f, 255.0f));
    Gdiplus::SolidBrush glowBrush(Gdiplus::Color(
        glowAlpha,
        scene.glowColor.GetR(),
        scene.glowColor.GetG(),
        scene.glowColor.GetB()));
    Gdiplus::SolidBrush shadowBrush(scene.pedestalFill);
    graphics->FillEllipse(&glowBrush, scene.glowRect);
    graphics->FillEllipse(&shadowBrush, scene.shadowRect);
    DrawActionOverlay(graphics, scene.actionOverlay, scene.bodyStroke);
    FillRoundedRect(graphics, scene.pedestalRect, scene.pedestalFill, scene.pedestalFill, 0.0f);
    FillEllipse(graphics, scene.tailRect, scene.tailFill, scene.bodyStroke, 1.2f);

    if (scene.facingSign < 0.0f) {
        FillEar(graphics, scene.rightEar, scene.earFillRear, scene.earInner, scene.bodyStroke);
        FillEar(graphics, scene.leftEar, scene.earFill, scene.earInner, scene.bodyStroke);
    } else {
        FillEar(graphics, scene.leftEar, scene.earFillRear, scene.earInner, scene.bodyStroke);
        FillEar(graphics, scene.rightEar, scene.earFill, scene.earInner, scene.bodyStroke);
    }

    FillRoundedRect(graphics, scene.leftLegRect, scene.bodyFillRear, scene.bodyStroke, 1.2f);
    FillRoundedRect(graphics, scene.rightLegRect, scene.bodyFillRear, scene.bodyStroke, 1.2f);

    const Gdiplus::GraphicsState saved = graphics->Save();
    const float cx = scene.bodyRect.X + scene.bodyRect.Width * 0.5f;
    const float cy = scene.bodyRect.Y + scene.bodyRect.Height * 0.5f;
    graphics->TranslateTransform(cx, cy);
    graphics->RotateTransform(scene.bodyTiltDeg);
    graphics->TranslateTransform(-cx, -cy);
    FillEllipse(graphics, scene.bodyRect, scene.bodyFill, scene.bodyStroke, 1.8f);
    FillEllipse(graphics, scene.chestRect, scene.headFill, scene.bodyStroke, 1.1f);
    graphics->Restore(saved);

    FillRoundedRect(graphics, scene.leftHandRect, scene.headFillRear, scene.bodyStroke, 1.2f);
    FillRoundedRect(graphics, scene.rightHandRect, scene.headFillRear, scene.bodyStroke, 1.2f);
    FillEllipse(graphics, scene.headRect, scene.headFill, scene.bodyStroke, 1.8f);

    FillEllipse(graphics, scene.leftEyeRect, scene.eyeFill, scene.eyeFill, 0.0f);
    FillEllipse(graphics, scene.rightEyeRect, scene.eyeFill, scene.eyeFill, 0.0f);
    FillEllipse(graphics, scene.noseRect, scene.mouthFill, scene.mouthFill, 0.0f);
    FillEllipse(graphics, scene.leftBlushRect, scene.blushFill, scene.blushFill, 0.0f);
    FillEllipse(graphics, scene.rightBlushRect, scene.blushFill, scene.blushFill, 0.0f);

    {
        Gdiplus::Pen browPen(scene.eyeFill, 1.5f);
        browPen.SetStartCap(Gdiplus::LineCapRound);
        browPen.SetEndCap(Gdiplus::LineCapRound);
        graphics->DrawLine(&browPen, scene.leftBrowStart, scene.leftBrowEnd);
        graphics->DrawLine(&browPen, scene.rightBrowStart, scene.rightBrowEnd);
    }

    {
        Gdiplus::Pen mouthPen(scene.mouthFill, scene.mouthStrokeWidth);
        mouthPen.SetStartCap(Gdiplus::LineCapRound);
        mouthPen.SetEndCap(Gdiplus::LineCapRound);
        graphics->DrawArc(
            &mouthPen,
            scene.mouthRect,
            scene.mouthStartDeg,
            scene.mouthSweepDeg);
    }

    for (size_t i = 0; i < scene.laneBadgeRects.size(); ++i) {
        const auto fill = scene.laneReady[i] ? scene.badgeReadyFill : scene.badgePendingFill;
        FillRoundedRect(graphics, scene.laneBadgeRects[i], fill, scene.bodyStroke, 0.8f);
    }

    if (scene.poseBadgeVisible) {
        FillEllipse(graphics, scene.poseBadgeRect, scene.accentFill, scene.headFill, 1.0f);
    }
    if (scene.accessoryVisible) {
        DrawStar(graphics, scene.accessoryStar, scene.accessoryFill, scene.accessoryStroke);
    }
}

} // namespace mousefx::windows

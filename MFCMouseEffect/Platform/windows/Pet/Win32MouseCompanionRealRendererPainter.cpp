#include "pch.h"

#include "Platform/windows/Pet/Win32MouseCompanionRealRendererPainter.h"

#include <algorithm>

namespace mousefx::windows {
namespace {

Gdiplus::Color WithAlpha(const Gdiplus::Color& color, float alpha) {
    return Gdiplus::Color(
        static_cast<BYTE>(std::clamp(alpha, 0.0f, 255.0f)),
        color.GetR(),
        color.GetG(),
        color.GetB());
}

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
    const Gdiplus::Color& stroke,
    float strokeWidth,
    float innerBaseInsetPx,
    float innerMidInsetPx,
    float innerTipInsetPx) {
    if (!graphics) {
        return;
    }
    Gdiplus::GraphicsPath path;
    path.AddPolygon(points.data(), static_cast<INT>(points.size()));
    Gdiplus::SolidBrush fillBrush(fill);
    Gdiplus::Pen pen(stroke, strokeWidth);
    graphics->FillPath(&fillBrush, &path);
    graphics->DrawPath(&pen, &path);

    Gdiplus::GraphicsPath innerPath;
    const Gdiplus::PointF innerPoints[4] = {
        Gdiplus::PointF(points[0].X, points[0].Y + innerBaseInsetPx),
        Gdiplus::PointF(
            (points[0].X + points[1].X) * 0.5f,
            (points[0].Y + points[1].Y) * 0.5f + innerMidInsetPx),
        Gdiplus::PointF(points[2].X, points[2].Y + innerTipInsetPx),
        Gdiplus::PointF(
            (points[0].X + points[3].X) * 0.5f,
            (points[0].Y + points[3].Y) * 0.5f + innerMidInsetPx),
    };
    innerPath.AddPolygon(innerPoints, 4);
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

template <size_t N>
void DrawPolygonAdornment(
    Gdiplus::Graphics* graphics,
    const std::array<Gdiplus::PointF, N>& points,
    const Gdiplus::Color& fill,
    const Gdiplus::Color& stroke,
    float strokeWidth) {
    if (!graphics) {
        return;
    }
    Gdiplus::GraphicsPath path;
    path.AddPolygon(points.data(), static_cast<INT>(points.size()));
    Gdiplus::SolidBrush brush(fill);
    Gdiplus::Pen pen(stroke, strokeWidth);
    pen.SetLineJoin(Gdiplus::LineJoinRound);
    graphics->FillPath(&brush, &path);
    graphics->DrawPath(&pen, &path);
}

void DrawRibbonAdornment(
    Gdiplus::Graphics* graphics,
    const std::array<Gdiplus::PointF, 4>& leftWing,
    const std::array<Gdiplus::PointF, 4>& rightWing,
    const Gdiplus::RectF& centerRect,
    const Gdiplus::Color& fill,
    const Gdiplus::Color& stroke) {
    DrawPolygonAdornment(graphics, leftWing, fill, stroke, 1.0f);
    DrawPolygonAdornment(graphics, rightWing, fill, stroke, 1.0f);
    FillEllipse(graphics, centerRect, fill, stroke, 1.0f);
}

void DrawAdornmentLine(
    Gdiplus::Graphics* graphics,
    const Gdiplus::PointF& start,
    const Gdiplus::PointF& end,
    const Gdiplus::Color& stroke,
    float alpha,
    float width) {
    if (!graphics) {
        return;
    }
    Gdiplus::Pen pen(WithAlpha(stroke, alpha), width);
    pen.SetStartCap(Gdiplus::LineCapRound);
    pen.SetEndCap(Gdiplus::LineCapRound);
    graphics->DrawLine(&pen, start, end);
}

void DrawModelSceneGraph(
    Gdiplus::Graphics* graphics,
    const Win32MouseCompanionRealRendererScene& scene) {
    if (!graphics || !scene.modelSceneGraphVisible) {
        return;
    }

    Gdiplus::Pen edgePen(Gdiplus::Color(96, 132, 176, 255), 1.0f);
    edgePen.SetStartCap(Gdiplus::LineCapRound);
    edgePen.SetEndCap(Gdiplus::LineCapRound);

    for (const auto& edge : scene.modelSceneGraphEdges) {
        edgePen.SetColor(Gdiplus::Color(
            static_cast<BYTE>(std::clamp(edge.alpha, 0.0f, 255.0f)),
            132,
            176,
            255));
        graphics->DrawLine(&edgePen, edge.start, edge.end);
    }

    Gdiplus::Pen linkPen(Gdiplus::Color(160, 255, 255, 255), 1.3f);
    linkPen.SetStartCap(Gdiplus::LineCapRound);
    linkPen.SetEndCap(Gdiplus::LineCapRound);
    const Gdiplus::REAL dashPattern[2] = {3.0f, 3.0f};
    linkPen.SetDashPattern(dashPattern, 2);
    for (const auto& link : scene.modelSceneGraphLinks) {
        linkPen.SetColor(Gdiplus::Color(
            static_cast<BYTE>(std::clamp(link.alpha, 0.0f, 255.0f)),
            link.color.GetR(),
            link.color.GetG(),
            link.color.GetB()));
        graphics->DrawLine(&linkPen, link.start, link.end);
    }

    Gdiplus::FontFamily fontFamily(L"Segoe UI");
    Gdiplus::Font font(&fontFamily, 8.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush textBrush(Gdiplus::Color(196, 235, 242, 255));

    for (const auto& node : scene.modelSceneGraphNodes) {
        FillEllipse(
            graphics,
            node.bounds,
            node.fill,
            Gdiplus::Color(0, 0, 0, 0),
            0.0f);
        if (!node.highlighted) {
            continue;
        }
        const Gdiplus::PointF labelOrigin(
            node.bounds.GetRight() + 4.0f,
            node.bounds.Y - 2.0f);
        const std::wstring label(node.nodeName.begin(), node.nodeName.end());
        graphics->DrawString(
            label.c_str(),
            -1,
            &font,
            labelOrigin,
            &textBrush);
    }
}

void DrawModelProxyLayer(
    Gdiplus::Graphics* graphics,
    const Win32MouseCompanionRealRendererScene& scene) {
    if (!graphics || !scene.modelProxyVisible) {
        return;
    }

    Gdiplus::Pen linkPen(Gdiplus::Color(144, 146, 214, 255), 2.0f);
    linkPen.SetStartCap(Gdiplus::LineCapRound);
    linkPen.SetEndCap(Gdiplus::LineCapRound);
    for (const auto& link : scene.modelProxyLinks) {
        linkPen.SetColor(Gdiplus::Color(
            static_cast<BYTE>(std::clamp(link.alpha, 0.0f, 255.0f)),
            link.color.GetR(),
            link.color.GetG(),
            link.color.GetB()));
        graphics->DrawLine(&linkPen, link.start, link.end);
    }

    for (const auto& node : scene.modelProxyNodes) {
        FillEllipse(
            graphics,
            node.bounds,
            WithAlpha(node.fill, node.alpha),
            WithAlpha(node.fill, std::min(255.0f, node.alpha + 36.0f)),
            1.1f);
    }
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
            const float alpha = std::max(48.0f, overlay.followTrailBaseAlpha - static_cast<float>(i) * 34.0f);
            FillEllipse(
                graphics,
                overlay.followTrailRects[i],
                WithAlpha(overlay.accentColor, alpha),
                Gdiplus::Color(0, 0, 0, 0),
                0.0f);
        }
    }

    if (overlay.scrollArcVisible) {
        Gdiplus::Pen arcPen(WithAlpha(overlay.accentColor, overlay.scrollArcAlpha), overlay.scrollArcStrokeWidth);
        arcPen.SetStartCap(Gdiplus::LineCapRound);
        arcPen.SetEndCap(Gdiplus::LineCapRound);
        graphics->DrawArc(
            &arcPen,
            overlay.scrollArcRect,
            overlay.scrollArcStartDeg,
            overlay.scrollArcSweepDeg);
    }

    if (overlay.dragLineVisible) {
        Gdiplus::Pen dashPen(WithAlpha(overlay.accentColor, overlay.dragLineAlpha), overlay.dragLineStrokeWidth);
        dashPen.SetStartCap(Gdiplus::LineCapRound);
        dashPen.SetEndCap(Gdiplus::LineCapRound);
        const Gdiplus::REAL dashPattern[2] = {4.0f, 3.0f};
        dashPen.SetDashPattern(dashPattern, 2);
        graphics->DrawLine(&dashPen, overlay.dragLineStart, overlay.dragLineEnd);
    }

    if (overlay.clickRingVisible) {
        Gdiplus::Pen ringPen(WithAlpha(overlay.accentColor, overlay.clickRingAlpha), overlay.clickRingStrokeWidth);
        ringPen.SetAlignment(Gdiplus::PenAlignmentCenter);
        graphics->DrawEllipse(&ringPen, overlay.clickRingRect);
    }

    if (overlay.holdBandVisible) {
        FillRoundedRect(
            graphics,
            overlay.holdBandRect,
            WithAlpha(overlay.accentColor, overlay.holdBandAlpha),
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
    Gdiplus::SolidBrush shadowBrush(WithAlpha(scene.shadowFill, scene.shadowFill.GetA() * scene.shadowAlphaScale));
    graphics->FillEllipse(&glowBrush, scene.glowRect);
    graphics->FillEllipse(&shadowBrush, scene.shadowRect);
    DrawModelSceneGraph(graphics, scene);
    DrawModelProxyLayer(graphics, scene);
    DrawActionOverlay(graphics, scene.actionOverlay, scene.bodyStroke);
    FillRoundedRect(
        graphics,
        scene.pedestalRect,
        WithAlpha(scene.pedestalFill, scene.pedestalFill.GetA() * scene.pedestalAlphaScale),
        WithAlpha(scene.pedestalFill, scene.pedestalFill.GetA() * scene.pedestalAlphaScale),
        0.0f);
    FillEllipse(graphics, scene.tailRect, scene.tailFill, scene.bodyStroke, scene.tailStrokeWidth);
    FillEllipse(
        graphics,
        scene.tailRootCuffRect,
        scene.tailFillRear,
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.tailBridgeRect,
        scene.tailMidFill,
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.tailMidContourRect,
        scene.tailMidFill,
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.tailTipBridgeRect,
        scene.tailTipFill,
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.tailTipRect,
        scene.tailTipFill,
        scene.tailStroke,
        std::max(0.9f, scene.tailStrokeWidth - 0.2f));
    FillEllipse(
        graphics,
        scene.leftEarRootCuffRect,
        scene.facingSign < 0.0f ? scene.earRootCuffFill : scene.earRootCuffFillRear,
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightEarRootCuffRect,
        scene.facingSign < 0.0f ? scene.earRootCuffFillRear : scene.earRootCuffFill,
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);

    if (scene.facingSign < 0.0f) {
        FillEar(
            graphics,
            scene.rightEar,
            scene.earFillRear,
            scene.earInnerRear,
            scene.earStrokeRear,
            scene.earStrokeWidthRear,
            scene.earInnerBaseInsetPxRear,
            scene.earInnerMidInsetPxRear,
            scene.earInnerTipInsetPxRear);
        FillEllipse(
            graphics,
            scene.rightEarOcclusionCapRect,
            WithAlpha(scene.headFill, scene.earOcclusionCapAlpha),
            Gdiplus::Color(0, 0, 0, 0),
            0.0f);
        FillEar(
            graphics,
            scene.leftEar,
            scene.earFill,
            scene.earInner,
            scene.earStroke,
            scene.earStrokeWidth,
            scene.earInnerBaseInsetPx,
            scene.earInnerMidInsetPx,
            scene.earInnerTipInsetPx);
    } else {
        FillEar(
            graphics,
            scene.leftEar,
            scene.earFillRear,
            scene.earInnerRear,
            scene.earStrokeRear,
            scene.earStrokeWidthRear,
            scene.earInnerBaseInsetPxRear,
            scene.earInnerMidInsetPxRear,
            scene.earInnerTipInsetPxRear);
        FillEllipse(
            graphics,
            scene.leftEarOcclusionCapRect,
            WithAlpha(scene.headFill, scene.earOcclusionCapAlpha),
            Gdiplus::Color(0, 0, 0, 0),
            0.0f);
        FillEar(
            graphics,
            scene.rightEar,
            scene.earFill,
            scene.earInner,
            scene.earStroke,
            scene.earStrokeWidth,
            scene.earInnerBaseInsetPx,
            scene.earInnerMidInsetPx,
            scene.earInnerTipInsetPx);
    }

    FillRoundedRect(graphics, scene.leftLegRect, scene.bodyFillRear, scene.bodyStroke, scene.limbStrokeWidth);
    FillRoundedRect(graphics, scene.rightLegRect, scene.bodyFillRear, scene.bodyStroke, scene.limbStrokeWidth);
    FillEllipse(
        graphics,
        scene.leftLegSilhouetteBridgeRect,
        WithAlpha(scene.bodyFillRear, 144.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightLegSilhouetteBridgeRect,
        WithAlpha(scene.bodyFillRear, 144.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftLegCadenceBridgeRect,
        WithAlpha(scene.bodyFillRear, 150.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightLegCadenceBridgeRect,
        WithAlpha(scene.bodyFillRear, 150.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftLegRootCuffRect,
        WithAlpha(scene.bodyFillRear, 156.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightLegRootCuffRect,
        WithAlpha(scene.bodyFillRear, 156.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftLegPadRect,
        WithAlpha(scene.blushFill, 196.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightLegPadRect,
        WithAlpha(scene.blushFill, 196.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);

    const Gdiplus::GraphicsState saved = graphics->Save();
    const float cx = scene.bodyRect.X + scene.bodyRect.Width * 0.5f;
    const float cy = scene.bodyRect.Y + scene.bodyRect.Height * 0.5f;
    graphics->TranslateTransform(cx, cy);
    graphics->RotateTransform(scene.bodyTiltDeg);
    graphics->TranslateTransform(-cx, -cy);
    FillEllipse(graphics, scene.bodyRect, scene.bodyFill, scene.bodyStroke, scene.bodyStrokeWidth);
    FillEllipse(
        graphics,
        scene.leftShoulderPatchRect,
        WithAlpha(scene.bodyFillRear, 176.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightShoulderPatchRect,
        WithAlpha(scene.bodyFillRear, 176.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillRoundedRect(
        graphics,
        scene.neckBridgeRect,
        WithAlpha(scene.headFillRear, 210.0f),
        scene.bodyStroke,
        std::max(1.0f, scene.bodyStrokeWidth - 0.2f));
    FillEllipse(
        graphics,
        scene.leftHeadShoulderBridgeRect,
        WithAlpha(scene.headFillRear, 174.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightHeadShoulderBridgeRect,
        WithAlpha(scene.headFillRear, 174.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.chestRect,
        WithAlpha(scene.headFill, scene.chestFillAlpha),
        scene.bodyStroke,
        scene.chestStrokeWidth);
    FillEllipse(
        graphics,
        scene.leftHipPatchRect,
        WithAlpha(scene.bodyFillRear, 166.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightHipPatchRect,
        WithAlpha(scene.bodyFillRear, 166.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.bellyContourRect,
        WithAlpha(scene.headFillRear, 138.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.sternumContourRect,
        WithAlpha(scene.headFillRear, 122.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.upperTorsoContourRect,
        WithAlpha(scene.headFillRear, 132.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftTorsoCadenceBridgeRect,
        WithAlpha(scene.bodyFillRear, 142.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightTorsoCadenceBridgeRect,
        WithAlpha(scene.bodyFillRear, 142.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftBackContourRect,
        WithAlpha(scene.bodyFillRear, 146.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightBackContourRect,
        WithAlpha(scene.bodyFillRear, 146.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftFlankContourRect,
        WithAlpha(scene.bodyFillRear, 150.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightFlankContourRect,
        WithAlpha(scene.bodyFillRear, 150.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftTailHaunchBridgeRect,
        WithAlpha(scene.bodyFillRear, 154.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightTailHaunchBridgeRect,
        WithAlpha(scene.bodyFillRear, 154.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    graphics->Restore(saved);

    FillRoundedRect(graphics, scene.leftHandRect, scene.headFillRear, scene.bodyStroke, scene.limbStrokeWidth);
    FillRoundedRect(graphics, scene.rightHandRect, scene.headFillRear, scene.bodyStroke, scene.limbStrokeWidth);
    FillEllipse(
        graphics,
        scene.leftHandSilhouetteBridgeRect,
        WithAlpha(scene.headFillRear, 150.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightHandSilhouetteBridgeRect,
        WithAlpha(scene.headFillRear, 150.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftHandCadenceBridgeRect,
        WithAlpha(scene.headFillRear, 158.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightHandCadenceBridgeRect,
        WithAlpha(scene.headFillRear, 158.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftHandRootCuffRect,
        WithAlpha(scene.headFillRear, 166.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightHandRootCuffRect,
        WithAlpha(scene.headFillRear, 166.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftHandPadRect,
        WithAlpha(scene.blushFill, 206.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightHandPadRect,
        WithAlpha(scene.blushFill, 206.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(graphics, scene.headRect, scene.headFill, scene.bodyStroke, scene.headStrokeWidth);
    FillEllipse(
        graphics,
        scene.leftCheekContourRect,
        WithAlpha(scene.headFillRear, 168.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightCheekContourRect,
        WithAlpha(scene.headFillRear, 168.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.jawContourRect,
        WithAlpha(scene.headFillRear, 154.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.muzzlePadRect,
        WithAlpha(scene.headFillRear, 188.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.foreheadPadRect,
        WithAlpha(scene.headFillRear, 148.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.crownPadRect,
        WithAlpha(scene.headFillRear, 138.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftParietalBridgeRect,
        WithAlpha(scene.headFillRear, 132.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightParietalBridgeRect,
        WithAlpha(scene.headFillRear, 132.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftEarSkullBridgeRect,
        WithAlpha(scene.headFillRear, 138.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightEarSkullBridgeRect,
        WithAlpha(scene.headFillRear, 138.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftOccipitalContourRect,
        WithAlpha(scene.headFillRear, 126.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightOccipitalContourRect,
        WithAlpha(scene.headFillRear, 126.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftTempleContourRect,
        WithAlpha(scene.headFillRear, 132.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightTempleContourRect,
        WithAlpha(scene.headFillRear, 132.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftUnderEyeContourRect,
        WithAlpha(scene.headFillRear, 126.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightUnderEyeContourRect,
        WithAlpha(scene.headFillRear, 126.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.noseBridgeRect,
        WithAlpha(scene.headFillRear, 118.0f),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);

    FillEllipse(graphics, scene.leftEyeRect, scene.eyeFill, scene.eyeFill, 0.0f);
    FillEllipse(graphics, scene.rightEyeRect, scene.eyeFill, scene.eyeFill, 0.0f);
    FillEllipse(graphics, scene.leftPupilRect, scene.mouthFill, scene.mouthFill, 0.0f);
    FillEllipse(graphics, scene.rightPupilRect, scene.mouthFill, scene.mouthFill, 0.0f);
    FillEllipse(
        graphics,
        scene.leftEyeHighlightRect,
        WithAlpha(Gdiplus::Color(255, 255, 255, 255), scene.eyeHighlightAlpha),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightEyeHighlightRect,
        WithAlpha(Gdiplus::Color(255, 255, 255, 255), scene.eyeHighlightAlpha),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
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
        Gdiplus::Pen whiskerPen(WithAlpha(scene.mouthFill, 210.0f), scene.whiskerStrokeWidth);
        whiskerPen.SetStartCap(Gdiplus::LineCapRound);
        whiskerPen.SetEndCap(Gdiplus::LineCapRound);
        for (size_t i = 0; i < scene.leftWhiskerStart.size(); ++i) {
            graphics->DrawLine(&whiskerPen, scene.leftWhiskerStart[i], scene.leftWhiskerEnd[i]);
            graphics->DrawLine(&whiskerPen, scene.rightWhiskerStart[i], scene.rightWhiskerEnd[i]);
        }
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
        FillEllipse(
            graphics,
            scene.poseBadgeRect,
            WithAlpha(scene.accentFill, scene.poseBadgeAlpha),
            WithAlpha(scene.headFill, scene.poseBadgeAlpha * 0.85f),
            1.0f);
    }
    if (scene.accessoryVisible) {
        const auto accessoryFill = WithAlpha(scene.accessoryFill, scene.accessoryFill.GetA() * scene.accessoryAlphaScale);
        const auto accessoryStroke = WithAlpha(
            scene.accessoryStroke,
            scene.accessoryStroke.GetA() * std::min(1.0f, scene.accessoryAlphaScale * 0.96f));
        switch (scene.accessoryShape) {
        case Win32MouseCompanionRealRendererAccessoryShape::Moon:
            DrawPolygonAdornment(graphics, scene.accessoryMoon, accessoryFill, accessoryStroke, scene.accessoryStrokeWidth);
            FillEllipse(
                graphics,
                scene.accessoryMoonInsetRect,
                WithAlpha(accessoryStroke, 54.0f * scene.accessoryAlphaScale),
                Gdiplus::Color(0, 0, 0, 0),
                0.0f);
            break;
        case Win32MouseCompanionRealRendererAccessoryShape::Leaf:
            DrawPolygonAdornment(graphics, scene.accessoryLeaf, accessoryFill, accessoryStroke, scene.accessoryStrokeWidth);
            DrawAdornmentLine(
                graphics,
                scene.accessoryLeafVeinStart,
                scene.accessoryLeafVeinEnd,
                accessoryStroke,
                164.0f * scene.accessoryAlphaScale,
                scene.accessoryStrokeWidth);
            break;
        case Win32MouseCompanionRealRendererAccessoryShape::RibbonBow:
            DrawRibbonAdornment(
                graphics,
                scene.accessoryRibbonLeft,
                scene.accessoryRibbonRight,
                scene.accessoryRibbonCenter,
                accessoryFill,
                accessoryStroke);
            DrawAdornmentLine(
                graphics,
                scene.accessoryRibbonLeftFoldStart,
                scene.accessoryRibbonLeftFoldEnd,
                accessoryStroke,
                162.0f * scene.accessoryAlphaScale,
                scene.accessoryStrokeWidth);
            DrawAdornmentLine(
                graphics,
                scene.accessoryRibbonRightFoldStart,
                scene.accessoryRibbonRightFoldEnd,
                accessoryStroke,
                162.0f * scene.accessoryAlphaScale,
                scene.accessoryStrokeWidth);
            break;
        case Win32MouseCompanionRealRendererAccessoryShape::Star:
            DrawStar(graphics, scene.accessoryStar, accessoryFill, accessoryStroke);
            break;
        case Win32MouseCompanionRealRendererAccessoryShape::None:
        default:
            break;
        }
    }
}

} // namespace mousefx::windows

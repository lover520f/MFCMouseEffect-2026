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

float ScaleAlpha(float alpha, float scale) {
    return std::clamp(alpha * scale, 0.0f, 255.0f);
}

Gdiplus::Color WithScaledAlpha(
    const Gdiplus::Color& color,
    float alpha,
    float scale) {
    return WithAlpha(color, ScaleAlpha(alpha, scale));
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

    for (const auto& silhouette : scene.modelProxySilhouettes) {
        FillEllipse(
            graphics,
            silhouette.bounds,
            WithAlpha(silhouette.fill, silhouette.alpha),
            Gdiplus::Color(0, 0, 0, 0),
            0.0f);
    }

    for (const auto& surface : scene.modelProxySurfaces) {
        if (surface.polygon.size() < 3) {
            continue;
        }
        Gdiplus::SolidBrush surfaceBrush(WithAlpha(surface.fill, surface.alpha));
        Gdiplus::Pen surfacePen(
            WithAlpha(surface.fill, std::min(255.0f, surface.alpha + 26.0f)),
            1.0f);
        surfacePen.SetLineJoin(Gdiplus::LineJoinRound);
        graphics->FillPolygon(
            &surfaceBrush,
            surface.polygon.data(),
            static_cast<INT>(surface.polygon.size()));
        graphics->DrawPolygon(
            &surfacePen,
            surface.polygon.data(),
            static_cast<INT>(surface.polygon.size()));
    }

    if (scene.modelProxyHull.size() >= 3) {
        Gdiplus::GraphicsPath hullPath;
        hullPath.AddClosedCurve(
            scene.modelProxyHull.data(),
            static_cast<INT>(scene.modelProxyHull.size()),
            0.45f);
        Gdiplus::SolidBrush hullBrush(Gdiplus::Color(44, 170, 214, 255));
        Gdiplus::Pen hullPen(Gdiplus::Color(92, 170, 214, 255), 1.0f);
        hullPen.SetLineJoin(Gdiplus::LineJoinRound);
        graphics->FillPath(&hullBrush, &hullPath);
        graphics->DrawPath(&hullPen, &hullPath);
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

Gdiplus::Color ResolveImprintColor(
    const Win32MouseCompanionRealRendererScene& scene,
    const std::string& logicalNode) {
    if (logicalNode == "body") {
        return scene.bodyFillRear;
    }
    if (logicalNode == "head") {
        return scene.headFillRear;
    }
    if (logicalNode == "appendage") {
        return scene.tailMidFill;
    }
    if (logicalNode == "overlay") {
        return scene.accentFill;
    }
    if (logicalNode == "grounding") {
        return scene.pedestalFill;
    }
    return scene.headFillRear;
}

void DrawModelProxyImprint(
    Gdiplus::Graphics* graphics,
    const Win32MouseCompanionRealRendererScene& scene) {
    if (!graphics || !scene.modelProxyVisible) {
        return;
    }

    for (const auto& silhouette : scene.modelProxySilhouettes) {
        FillEllipse(
            graphics,
            silhouette.bounds,
            WithAlpha(ResolveImprintColor(scene, silhouette.logicalNode), silhouette.alpha * 0.46f),
            Gdiplus::Color(0, 0, 0, 0),
            0.0f);
    }

    for (const auto& surface : scene.modelProxySurfaces) {
        if (surface.polygon.size() < 3) {
            continue;
        }
        const Gdiplus::Color imprintColor =
            surface.surfaceKey == "core_shell" ? scene.headFillRear : scene.accentFill;
        Gdiplus::SolidBrush brush(WithAlpha(imprintColor, surface.alpha * 0.34f));
        graphics->FillPolygon(
            &brush,
            surface.polygon.data(),
            static_cast<INT>(surface.polygon.size()));
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

void DrawModelProxyActionLayer(
    Gdiplus::Graphics* graphics,
    const Win32MouseCompanionRealRendererModelProxyActionLayer& layer) {
    if (!graphics) {
        return;
    }

    if (layer.followShellVisible) {
        for (size_t i = 0; i < layer.followShellRects.size(); ++i) {
            const float alpha =
                std::max(40.0f, layer.followShellBaseAlpha - static_cast<float>(i) * 30.0f);
            FillEllipse(
                graphics,
                layer.followShellRects[i],
                WithAlpha(layer.accentColor, alpha),
                Gdiplus::Color(0, 0, 0, 0),
                0.0f);
        }
    }

    if (layer.scrollShellVisible) {
        Gdiplus::Pen arcPen(
            WithAlpha(layer.accentColor, layer.scrollShellAlpha),
            layer.scrollShellStrokeWidth);
        arcPen.SetStartCap(Gdiplus::LineCapRound);
        arcPen.SetEndCap(Gdiplus::LineCapRound);
        graphics->DrawArc(
            &arcPen,
            layer.scrollShellRect,
            layer.scrollShellStartDeg,
            layer.scrollShellSweepDeg);
    }

    if (layer.dragShellVisible) {
        Gdiplus::Pen dashPen(
            WithAlpha(layer.accentColor, layer.dragShellAlpha),
            layer.dragShellStrokeWidth);
        dashPen.SetStartCap(Gdiplus::LineCapRound);
        dashPen.SetEndCap(Gdiplus::LineCapRound);
        const Gdiplus::REAL dashPattern[2] = {5.0f, 4.0f};
        dashPen.SetDashPattern(dashPattern, 2);
        graphics->DrawLine(&dashPen, layer.dragShellStart, layer.dragShellEnd);
    }

    if (layer.clickShellVisible) {
        Gdiplus::Pen ringPen(
            WithAlpha(layer.accentColor, layer.clickShellAlpha),
            layer.clickShellStrokeWidth);
        ringPen.SetAlignment(Gdiplus::PenAlignmentCenter);
        graphics->DrawEllipse(&ringPen, layer.clickShellRect);
    }

    if (layer.holdShellVisible) {
        FillRoundedRect(
            graphics,
            layer.holdShellRect,
            WithAlpha(layer.accentColor, layer.holdShellAlpha),
            Gdiplus::Color(0, 0, 0, 0),
            0.0f);
    }
}

void DrawModelProxyAdornmentLayer(
    Gdiplus::Graphics* graphics,
    const Win32MouseCompanionRealRendererScene& scene) {
    if (!graphics || !scene.modelProxyAdornmentLayer.visible) {
        return;
    }

    const auto& layer = scene.modelProxyAdornmentLayer;
    for (size_t i = 0; i < layer.laneBadgeRects.size(); ++i) {
        const auto fill = layer.laneReady[i] ? scene.badgeReadyFill : scene.badgePendingFill;
        FillRoundedRect(
            graphics,
            layer.laneBadgeRects[i],
            WithScaledAlpha(fill, fill.GetA(), layer.laneAlphaScale),
            WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), layer.laneAlphaScale),
            0.9f);
    }

    if (layer.poseBadgeVisible) {
        FillEllipse(
            graphics,
            layer.poseBadgeRect,
            WithScaledAlpha(scene.accentFill, scene.poseBadgeAlpha, layer.poseBadgeAlphaScale),
            WithScaledAlpha(scene.headFill, scene.poseBadgeAlpha * 0.85f, layer.poseBadgeAlphaScale),
            1.0f);
    }

    if (!layer.accessoryVisible) {
        return;
    }

    const auto accessoryFill = WithScaledAlpha(
        scene.accessoryFill,
        scene.accessoryFill.GetA() * layer.accessoryAlphaScale,
        1.0f);
    const auto accessoryStroke = WithScaledAlpha(
        scene.accessoryStroke,
        scene.accessoryStroke.GetA() * std::min(1.0f, layer.accessoryAlphaScale * 0.96f),
        1.0f);
    const float strokeWidth = scene.accessoryStrokeWidth * layer.accessoryStrokeScale;

    switch (layer.accessoryShape) {
    case Win32MouseCompanionRealRendererAccessoryShape::Moon:
        DrawPolygonAdornment(graphics, layer.accessoryMoon, accessoryFill, accessoryStroke, strokeWidth);
        FillEllipse(
            graphics,
            layer.accessoryMoonInsetRect,
            WithAlpha(accessoryStroke, 54.0f * layer.accessoryAlphaScale),
            Gdiplus::Color(0, 0, 0, 0),
            0.0f);
        break;
    case Win32MouseCompanionRealRendererAccessoryShape::Leaf:
        DrawPolygonAdornment(graphics, layer.accessoryLeaf, accessoryFill, accessoryStroke, strokeWidth);
        DrawAdornmentLine(
            graphics,
            layer.accessoryLeafVeinStart,
            layer.accessoryLeafVeinEnd,
            accessoryStroke,
            164.0f * layer.accessoryAlphaScale,
            strokeWidth);
        break;
    case Win32MouseCompanionRealRendererAccessoryShape::RibbonBow:
        DrawRibbonAdornment(
            graphics,
            layer.accessoryRibbonLeft,
            layer.accessoryRibbonRight,
            layer.accessoryRibbonCenter,
            accessoryFill,
            accessoryStroke);
        DrawAdornmentLine(
            graphics,
            layer.accessoryRibbonLeftFoldStart,
            layer.accessoryRibbonLeftFoldEnd,
            accessoryStroke,
            162.0f * layer.accessoryAlphaScale,
            strokeWidth);
        DrawAdornmentLine(
            graphics,
            layer.accessoryRibbonRightFoldStart,
            layer.accessoryRibbonRightFoldEnd,
            accessoryStroke,
            162.0f * layer.accessoryAlphaScale,
            strokeWidth);
        break;
    case Win32MouseCompanionRealRendererAccessoryShape::Star:
        DrawStar(graphics, layer.accessoryStar, accessoryFill, accessoryStroke);
        break;
    case Win32MouseCompanionRealRendererAccessoryShape::None:
    default:
        break;
    }
}

void DrawModelProxyDetailLayer(
    Gdiplus::Graphics* graphics,
    const Win32MouseCompanionRealRendererScene& scene) {
    if (!graphics || !scene.modelProxyDetailLayer.visible) {
        return;
    }

    const auto& layer = scene.modelProxyDetailLayer;
    FillEllipse(
        graphics,
        layer.leftEyeRect,
        WithScaledAlpha(scene.eyeFill, scene.eyeFill.GetA(), layer.eyeAlphaScale),
        WithScaledAlpha(scene.eyeFill, scene.eyeFill.GetA(), layer.eyeAlphaScale),
        0.0f);
    FillEllipse(
        graphics,
        layer.rightEyeRect,
        WithScaledAlpha(scene.eyeFill, scene.eyeFill.GetA(), layer.eyeAlphaScale),
        WithScaledAlpha(scene.eyeFill, scene.eyeFill.GetA(), layer.eyeAlphaScale),
        0.0f);
    FillEllipse(
        graphics,
        layer.leftPupilRect,
        WithScaledAlpha(scene.mouthFill, scene.mouthFill.GetA(), layer.eyeAlphaScale),
        WithScaledAlpha(scene.mouthFill, scene.mouthFill.GetA(), layer.eyeAlphaScale),
        0.0f);
    FillEllipse(
        graphics,
        layer.rightPupilRect,
        WithScaledAlpha(scene.mouthFill, scene.mouthFill.GetA(), layer.eyeAlphaScale),
        WithScaledAlpha(scene.mouthFill, scene.mouthFill.GetA(), layer.eyeAlphaScale),
        0.0f);
    FillEllipse(
        graphics,
        layer.leftHighlightRect,
        WithScaledAlpha(scene.eyeFill, scene.eyeHighlightAlpha, layer.highlightAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.rightHighlightRect,
        WithScaledAlpha(scene.eyeFill, scene.eyeHighlightAlpha, layer.highlightAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.noseRect,
        WithScaledAlpha(scene.mouthFill, scene.mouthFill.GetA(), layer.mouthAlphaScale),
        WithScaledAlpha(scene.mouthFill, scene.mouthFill.GetA(), layer.mouthAlphaScale),
        0.0f);
    FillEllipse(
        graphics,
        layer.leftBlushRect,
        WithScaledAlpha(scene.blushFill, scene.blushFill.GetA(), layer.blushAlphaScale),
        WithScaledAlpha(scene.blushFill, scene.blushFill.GetA(), layer.blushAlphaScale),
        0.0f);
    FillEllipse(
        graphics,
        layer.rightBlushRect,
        WithScaledAlpha(scene.blushFill, scene.blushFill.GetA(), layer.blushAlphaScale),
        WithScaledAlpha(scene.blushFill, scene.blushFill.GetA(), layer.blushAlphaScale),
        0.0f);
    {
        Gdiplus::Pen whiskerPen(
            WithScaledAlpha(scene.mouthFill, 210.0f, layer.mouthAlphaScale),
            scene.whiskerStrokeWidth * layer.whiskerStrokeScale);
        whiskerPen.SetStartCap(Gdiplus::LineCapRound);
        whiskerPen.SetEndCap(Gdiplus::LineCapRound);
        for (size_t i = 0; i < layer.leftWhiskerStart.size(); ++i) {
            graphics->DrawLine(&whiskerPen, layer.leftWhiskerStart[i], layer.leftWhiskerEnd[i]);
            graphics->DrawLine(&whiskerPen, layer.rightWhiskerStart[i], layer.rightWhiskerEnd[i]);
        }
    }
    {
        Gdiplus::Pen mouthPen(
            WithScaledAlpha(scene.mouthFill, scene.mouthFill.GetA(), layer.mouthAlphaScale),
            scene.mouthStrokeWidth);
        mouthPen.SetStartCap(Gdiplus::LineCapRound);
        mouthPen.SetEndCap(Gdiplus::LineCapRound);
        graphics->DrawArc(
            &mouthPen,
            layer.mouthRect,
            layer.mouthStartDeg,
            layer.mouthSweepDeg);
    }
}

void DrawModelProxyFrameLayer(
    Gdiplus::Graphics* graphics,
    const Win32MouseCompanionRealRendererScene& scene) {
    if (!graphics || !scene.modelProxyFrameLayer.visible) {
        return;
    }

    const auto& layer = scene.modelProxyFrameLayer;
    FillRoundedRect(
        graphics,
        layer.bodyRect,
        WithScaledAlpha(scene.bodyFill, scene.bodyFill.GetA(), layer.fillAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), layer.strokeAlphaScale),
        scene.bodyStrokeWidth * layer.strokeWidthScale);
    FillEllipse(
        graphics,
        layer.chestRect,
        WithScaledAlpha(scene.headFill, scene.chestFillAlpha, layer.fillAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), layer.strokeAlphaScale),
        scene.chestStrokeWidth * layer.strokeWidthScale);
    FillEllipse(
        graphics,
        layer.neckBridgeRect,
        WithScaledAlpha(scene.headFillRear, 178.0f, layer.fillAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.headRect,
        WithScaledAlpha(scene.headFill, scene.headFill.GetA(), layer.fillAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), layer.strokeAlphaScale),
        scene.headStrokeWidth * layer.strokeWidthScale);
    FillEllipse(
        graphics,
        layer.tailRect,
        WithScaledAlpha(scene.tailFill, scene.tailFill.GetA(), layer.fillAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), layer.strokeAlphaScale),
        scene.tailStrokeWidth * layer.strokeWidthScale);
    FillRoundedRect(
        graphics,
        layer.leftHandRect,
        WithScaledAlpha(scene.bodyFillRear, scene.bodyFillRear.GetA(), layer.appendageAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), layer.strokeAlphaScale),
        scene.limbStrokeWidth * layer.strokeWidthScale);
    FillRoundedRect(
        graphics,
        layer.rightHandRect,
        WithScaledAlpha(scene.bodyFillRear, scene.bodyFillRear.GetA(), layer.appendageAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), layer.strokeAlphaScale),
        scene.limbStrokeWidth * layer.strokeWidthScale);
    FillRoundedRect(
        graphics,
        layer.leftLegRect,
        WithScaledAlpha(scene.bodyFillRear, scene.bodyFillRear.GetA(), layer.appendageAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), layer.strokeAlphaScale),
        scene.limbStrokeWidth * layer.strokeWidthScale);
    FillRoundedRect(
        graphics,
        layer.rightLegRect,
        WithScaledAlpha(scene.bodyFillRear, scene.bodyFillRear.GetA(), layer.appendageAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), layer.strokeAlphaScale),
        scene.limbStrokeWidth * layer.strokeWidthScale);
}

void DrawModelProxyContourLayer(
    Gdiplus::Graphics* graphics,
    const Win32MouseCompanionRealRendererScene& scene) {
    if (!graphics || !scene.modelProxyContourLayer.visible) {
        return;
    }

    const auto& layer = scene.modelProxyContourLayer;
    FillEllipse(
        graphics,
        layer.leftEarRootCuffRect,
        WithScaledAlpha(scene.earRootCuffFillRear, scene.earRootCuffFillRear.GetA(), layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.rightEarRootCuffRect,
        WithScaledAlpha(scene.earRootCuffFill, scene.earRootCuffFill.GetA(), layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEar(
        graphics,
        layer.leftEar,
        scene.earFill,
        scene.earInner,
        WithScaledAlpha(scene.earStroke, scene.earStroke.GetA(), layer.strokeAlphaScale),
        scene.earStrokeWidth,
        scene.earInnerBaseInsetPx,
        scene.earInnerMidInsetPx,
        scene.earInnerTipInsetPx);
    FillEar(
        graphics,
        layer.rightEar,
        scene.earFillRear,
        scene.earInnerRear,
        WithScaledAlpha(scene.earStrokeRear, scene.earStrokeRear.GetA(), layer.strokeAlphaScale),
        scene.earStrokeWidthRear,
        scene.earInnerBaseInsetPxRear,
        scene.earInnerMidInsetPxRear,
        scene.earInnerTipInsetPxRear);
    FillEllipse(
        graphics,
        layer.leftEarOcclusionCapRect,
        WithScaledAlpha(scene.headFill, scene.earOcclusionCapAlpha, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.rightEarOcclusionCapRect,
        WithScaledAlpha(scene.headFill, scene.earOcclusionCapAlpha, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.leftHeadShoulderBridgeRect,
        WithScaledAlpha(scene.headFillRear, 174.0f, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.rightHeadShoulderBridgeRect,
        WithScaledAlpha(scene.headFillRear, 174.0f, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.leftShoulderPatchRect,
        WithScaledAlpha(scene.bodyFillRear, 176.0f, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.rightShoulderPatchRect,
        WithScaledAlpha(scene.bodyFillRear, 176.0f, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.leftHipPatchRect,
        WithScaledAlpha(scene.bodyFillRear, 166.0f, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.rightHipPatchRect,
        WithScaledAlpha(scene.bodyFillRear, 166.0f, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.bellyContourRect,
        WithScaledAlpha(scene.headFillRear, 138.0f, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.sternumContourRect,
        WithScaledAlpha(scene.headFillRear, 122.0f, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.upperTorsoContourRect,
        WithScaledAlpha(scene.headFillRear, 132.0f, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.leftTorsoCadenceBridgeRect,
        WithScaledAlpha(scene.headFillRear, 126.0f, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.rightTorsoCadenceBridgeRect,
        WithScaledAlpha(scene.headFillRear, 126.0f, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.leftBackContourRect,
        WithScaledAlpha(scene.headFillRear, 122.0f, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.rightBackContourRect,
        WithScaledAlpha(scene.headFillRear, 122.0f, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.leftFlankContourRect,
        WithScaledAlpha(scene.headFillRear, 118.0f, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.rightFlankContourRect,
        WithScaledAlpha(scene.headFillRear, 118.0f, layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
}

void DrawModelProxyAppendageLayer(
    Gdiplus::Graphics* graphics,
    const Win32MouseCompanionRealRendererScene& scene) {
    if (!graphics || !scene.modelProxyAppendageLayer.visible) {
        return;
    }

    const auto& layer = scene.modelProxyAppendageLayer;
    FillEllipse(
        graphics,
        layer.tailRootCuffRect,
        WithScaledAlpha(scene.tailFillRear, scene.tailFillRear.GetA(), layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.tailBridgeRect,
        WithScaledAlpha(scene.tailMidFill, scene.tailMidFill.GetA(), layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.tailMidContourRect,
        WithScaledAlpha(scene.tailMidFill, scene.tailMidFill.GetA(), layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.tailTipBridgeRect,
        WithScaledAlpha(scene.tailTipFill, scene.tailTipFill.GetA(), layer.rearAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        layer.tailTipRect,
        WithScaledAlpha(scene.tailTipFill, scene.tailTipFill.GetA(), layer.accentAlphaScale),
        WithScaledAlpha(scene.tailStroke, scene.tailStroke.GetA(), layer.strokeAlphaScale),
        std::max(0.9f, scene.tailStrokeWidth - 0.2f));

    const auto rearFill = WithScaledAlpha(scene.bodyFillRear, scene.bodyFillRear.GetA(), layer.rearAlphaScale);
    const auto rearAccent = WithScaledAlpha(scene.blushFill, scene.blushFill.GetA(), layer.accentAlphaScale);
    FillEllipse(graphics, layer.leftLegSilhouetteBridgeRect, rearFill, Gdiplus::Color(0, 0, 0, 0), 0.0f);
    FillEllipse(graphics, layer.rightLegSilhouetteBridgeRect, rearFill, Gdiplus::Color(0, 0, 0, 0), 0.0f);
    FillEllipse(graphics, layer.leftLegCadenceBridgeRect, rearFill, Gdiplus::Color(0, 0, 0, 0), 0.0f);
    FillEllipse(graphics, layer.rightLegCadenceBridgeRect, rearFill, Gdiplus::Color(0, 0, 0, 0), 0.0f);
    FillEllipse(graphics, layer.leftLegRootCuffRect, rearFill, Gdiplus::Color(0, 0, 0, 0), 0.0f);
    FillEllipse(graphics, layer.rightLegRootCuffRect, rearFill, Gdiplus::Color(0, 0, 0, 0), 0.0f);
    FillEllipse(graphics, layer.leftLegPadRect, rearAccent, Gdiplus::Color(0, 0, 0, 0), 0.0f);
    FillEllipse(graphics, layer.rightLegPadRect, rearAccent, Gdiplus::Color(0, 0, 0, 0), 0.0f);
    FillEllipse(graphics, layer.leftHandSilhouetteBridgeRect, rearFill, Gdiplus::Color(0, 0, 0, 0), 0.0f);
    FillEllipse(graphics, layer.rightHandSilhouetteBridgeRect, rearFill, Gdiplus::Color(0, 0, 0, 0), 0.0f);
    FillEllipse(graphics, layer.leftHandCadenceBridgeRect, rearFill, Gdiplus::Color(0, 0, 0, 0), 0.0f);
    FillEllipse(graphics, layer.rightHandCadenceBridgeRect, rearFill, Gdiplus::Color(0, 0, 0, 0), 0.0f);
    FillEllipse(graphics, layer.leftHandRootCuffRect, rearFill, Gdiplus::Color(0, 0, 0, 0), 0.0f);
    FillEllipse(graphics, layer.rightHandRootCuffRect, rearFill, Gdiplus::Color(0, 0, 0, 0), 0.0f);
    FillEllipse(graphics, layer.leftHandPadRect, rearAccent, Gdiplus::Color(0, 0, 0, 0), 0.0f);
    FillEllipse(graphics, layer.rightHandPadRect, rearAccent, Gdiplus::Color(0, 0, 0, 0), 0.0f);
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
    DrawModelProxyFrameLayer(graphics, scene);
    DrawModelProxyContourLayer(graphics, scene);
    DrawModelProxyAppendageLayer(graphics, scene);
    DrawModelProxyDetailLayer(graphics, scene);
    DrawModelProxyAdornmentLayer(graphics, scene);
    DrawModelProxyActionLayer(graphics, scene.modelProxyActionLayer);
    DrawActionOverlay(graphics, scene.actionOverlay, scene.bodyStroke);
    FillRoundedRect(
        graphics,
        scene.pedestalRect,
        WithAlpha(scene.pedestalFill, scene.pedestalFill.GetA() * scene.pedestalAlphaScale),
        WithAlpha(scene.pedestalFill, scene.pedestalFill.GetA() * scene.pedestalAlphaScale),
        0.0f);
    FillEllipse(
        graphics,
        scene.tailRect,
        WithScaledAlpha(scene.tailFill, scene.tailFill.GetA(), scene.previewTailAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), scene.previewTailAlphaScale),
        scene.tailStrokeWidth * scene.previewTailStrokeScale);
    FillEllipse(
        graphics,
        scene.tailRootCuffRect,
        WithScaledAlpha(scene.tailFillRear, scene.tailFillRear.GetA(), scene.previewTailAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.tailBridgeRect,
        WithScaledAlpha(scene.tailMidFill, scene.tailMidFill.GetA(), scene.previewTailAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.tailMidContourRect,
        WithScaledAlpha(scene.tailMidFill, scene.tailMidFill.GetA(), scene.previewTailAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.tailTipBridgeRect,
        WithScaledAlpha(scene.tailTipFill, scene.tailTipFill.GetA(), scene.previewTailAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.tailTipRect,
        WithScaledAlpha(scene.tailTipFill, scene.tailTipFill.GetA(), scene.previewTailAlphaScale),
        WithScaledAlpha(scene.tailStroke, scene.tailStroke.GetA(), scene.previewTailAlphaScale),
        std::max(0.72f, (scene.tailStrokeWidth - 0.2f) * scene.previewTailStrokeScale));
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

    FillRoundedRect(
        graphics,
        scene.leftLegRect,
        WithScaledAlpha(scene.bodyFillRear, scene.bodyFillRear.GetA(), scene.previewLegAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), scene.previewLegAlphaScale),
        scene.limbStrokeWidth * scene.previewLegStrokeScale);
    FillRoundedRect(
        graphics,
        scene.rightLegRect,
        WithScaledAlpha(scene.bodyFillRear, scene.bodyFillRear.GetA(), scene.previewLegAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), scene.previewLegAlphaScale),
        scene.limbStrokeWidth * scene.previewLegStrokeScale);
    FillEllipse(
        graphics,
        scene.leftLegSilhouetteBridgeRect,
        WithScaledAlpha(scene.bodyFillRear, 144.0f, scene.previewLegAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightLegSilhouetteBridgeRect,
        WithScaledAlpha(scene.bodyFillRear, 144.0f, scene.previewLegAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftLegCadenceBridgeRect,
        WithScaledAlpha(scene.bodyFillRear, 150.0f, scene.previewLegAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightLegCadenceBridgeRect,
        WithScaledAlpha(scene.bodyFillRear, 150.0f, scene.previewLegAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftLegRootCuffRect,
        WithScaledAlpha(scene.bodyFillRear, 156.0f, scene.previewLegAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightLegRootCuffRect,
        WithScaledAlpha(scene.bodyFillRear, 156.0f, scene.previewLegAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftLegPadRect,
        WithScaledAlpha(scene.blushFill, 196.0f, scene.previewLegAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightLegPadRect,
        WithScaledAlpha(scene.blushFill, 196.0f, scene.previewLegAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);

    const Gdiplus::GraphicsState saved = graphics->Save();
    const float cx = scene.bodyRect.X + scene.bodyRect.Width * 0.5f;
    const float cy = scene.bodyRect.Y + scene.bodyRect.Height * 0.5f;
    graphics->TranslateTransform(cx, cy);
    graphics->RotateTransform(scene.bodyTiltDeg);
    graphics->TranslateTransform(-cx, -cy);
    FillEllipse(
        graphics,
        scene.bodyRect,
        WithScaledAlpha(scene.bodyFill, scene.bodyFill.GetA(), scene.previewBodyAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), scene.previewBodyAlphaScale),
        scene.bodyStrokeWidth);
    FillEllipse(
        graphics,
        scene.leftShoulderPatchRect,
        WithScaledAlpha(scene.bodyFillRear, 176.0f, scene.previewBodyAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightShoulderPatchRect,
        WithScaledAlpha(scene.bodyFillRear, 176.0f, scene.previewBodyAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillRoundedRect(
        graphics,
        scene.neckBridgeRect,
        WithScaledAlpha(scene.headFillRear, 210.0f, scene.previewBodyAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), scene.previewBodyAlphaScale),
        std::max(1.0f, scene.bodyStrokeWidth - 0.2f));
    FillEllipse(
        graphics,
        scene.leftHeadShoulderBridgeRect,
        WithScaledAlpha(scene.headFillRear, 174.0f, scene.previewBodyAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightHeadShoulderBridgeRect,
        WithScaledAlpha(scene.headFillRear, 174.0f, scene.previewBodyAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.chestRect,
        WithScaledAlpha(scene.headFill, scene.chestFillAlpha, scene.previewBodyAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), scene.previewBodyAlphaScale),
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

    FillRoundedRect(
        graphics,
        scene.leftHandRect,
        WithScaledAlpha(scene.headFillRear, scene.headFillRear.GetA(), scene.previewHandAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), scene.previewHandAlphaScale),
        scene.limbStrokeWidth * scene.previewHandStrokeScale);
    FillRoundedRect(
        graphics,
        scene.rightHandRect,
        WithScaledAlpha(scene.headFillRear, scene.headFillRear.GetA(), scene.previewHandAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), scene.previewHandAlphaScale),
        scene.limbStrokeWidth * scene.previewHandStrokeScale);
    FillEllipse(
        graphics,
        scene.leftHandSilhouetteBridgeRect,
        WithScaledAlpha(scene.headFillRear, 150.0f, scene.previewHandAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightHandSilhouetteBridgeRect,
        WithScaledAlpha(scene.headFillRear, 150.0f, scene.previewHandAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftHandCadenceBridgeRect,
        WithScaledAlpha(scene.headFillRear, 158.0f, scene.previewHandAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightHandCadenceBridgeRect,
        WithScaledAlpha(scene.headFillRear, 158.0f, scene.previewHandAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftHandRootCuffRect,
        WithScaledAlpha(scene.headFillRear, 166.0f, scene.previewHandAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightHandRootCuffRect,
        WithScaledAlpha(scene.headFillRear, 166.0f, scene.previewHandAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftHandPadRect,
        WithScaledAlpha(scene.blushFill, 206.0f, scene.previewHandAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightHandPadRect,
        WithScaledAlpha(scene.blushFill, 206.0f, scene.previewHandAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.headRect,
        WithScaledAlpha(scene.headFill, scene.headFill.GetA(), scene.previewHeadAlphaScale),
        WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), scene.previewHeadAlphaScale),
        scene.headStrokeWidth);
    FillEllipse(
        graphics,
        scene.leftCheekContourRect,
        WithScaledAlpha(scene.headFillRear, 168.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightCheekContourRect,
        WithScaledAlpha(scene.headFillRear, 168.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.jawContourRect,
        WithScaledAlpha(scene.headFillRear, 154.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.muzzlePadRect,
        WithScaledAlpha(scene.headFillRear, 188.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.foreheadPadRect,
        WithScaledAlpha(scene.headFillRear, 148.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.crownPadRect,
        WithScaledAlpha(scene.headFillRear, 138.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftParietalBridgeRect,
        WithScaledAlpha(scene.headFillRear, 132.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightParietalBridgeRect,
        WithScaledAlpha(scene.headFillRear, 132.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftEarSkullBridgeRect,
        WithScaledAlpha(scene.headFillRear, 138.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightEarSkullBridgeRect,
        WithScaledAlpha(scene.headFillRear, 138.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftOccipitalContourRect,
        WithScaledAlpha(scene.headFillRear, 126.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightOccipitalContourRect,
        WithScaledAlpha(scene.headFillRear, 126.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftTempleContourRect,
        WithScaledAlpha(scene.headFillRear, 132.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightTempleContourRect,
        WithScaledAlpha(scene.headFillRear, 132.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftUnderEyeContourRect,
        WithScaledAlpha(scene.headFillRear, 126.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightUnderEyeContourRect,
        WithScaledAlpha(scene.headFillRear, 126.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.noseBridgeRect,
        WithScaledAlpha(scene.headFillRear, 118.0f, scene.previewHeadAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);

    FillEllipse(
        graphics,
        scene.leftEyeRect,
        WithScaledAlpha(scene.eyeFill, scene.eyeFill.GetA(), scene.previewDetailAlphaScale),
        WithScaledAlpha(scene.eyeFill, scene.eyeFill.GetA(), scene.previewDetailAlphaScale),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightEyeRect,
        WithScaledAlpha(scene.eyeFill, scene.eyeFill.GetA(), scene.previewDetailAlphaScale),
        WithScaledAlpha(scene.eyeFill, scene.eyeFill.GetA(), scene.previewDetailAlphaScale),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftPupilRect,
        WithScaledAlpha(scene.mouthFill, scene.mouthFill.GetA(), scene.previewDetailAlphaScale),
        WithScaledAlpha(scene.mouthFill, scene.mouthFill.GetA(), scene.previewDetailAlphaScale),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightPupilRect,
        WithScaledAlpha(scene.mouthFill, scene.mouthFill.GetA(), scene.previewDetailAlphaScale),
        WithScaledAlpha(scene.mouthFill, scene.mouthFill.GetA(), scene.previewDetailAlphaScale),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftEyeHighlightRect,
        WithScaledAlpha(
            Gdiplus::Color(255, 255, 255, 255),
            scene.eyeHighlightAlpha,
            scene.previewDetailAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightEyeHighlightRect,
        WithScaledAlpha(
            Gdiplus::Color(255, 255, 255, 255),
            scene.eyeHighlightAlpha,
            scene.previewDetailAlphaScale),
        Gdiplus::Color(0, 0, 0, 0),
        0.0f);
    FillEllipse(
        graphics,
        scene.noseRect,
        WithScaledAlpha(scene.mouthFill, scene.mouthFill.GetA(), scene.previewDetailAlphaScale),
        WithScaledAlpha(scene.mouthFill, scene.mouthFill.GetA(), scene.previewDetailAlphaScale),
        0.0f);
    FillEllipse(
        graphics,
        scene.leftBlushRect,
        WithScaledAlpha(scene.blushFill, scene.blushFill.GetA(), scene.previewDetailAlphaScale),
        WithScaledAlpha(scene.blushFill, scene.blushFill.GetA(), scene.previewDetailAlphaScale),
        0.0f);
    FillEllipse(
        graphics,
        scene.rightBlushRect,
        WithScaledAlpha(scene.blushFill, scene.blushFill.GetA(), scene.previewDetailAlphaScale),
        WithScaledAlpha(scene.blushFill, scene.blushFill.GetA(), scene.previewDetailAlphaScale),
        0.0f);

    {
        Gdiplus::Pen browPen(
            WithScaledAlpha(scene.eyeFill, scene.eyeFill.GetA(), scene.previewDetailAlphaScale),
            1.5f);
        browPen.SetStartCap(Gdiplus::LineCapRound);
        browPen.SetEndCap(Gdiplus::LineCapRound);
        graphics->DrawLine(&browPen, scene.leftBrowStart, scene.leftBrowEnd);
        graphics->DrawLine(&browPen, scene.rightBrowStart, scene.rightBrowEnd);
    }

    {
        Gdiplus::Pen whiskerPen(
            WithScaledAlpha(scene.mouthFill, 210.0f, scene.previewDetailAlphaScale),
            scene.whiskerStrokeWidth);
        whiskerPen.SetStartCap(Gdiplus::LineCapRound);
        whiskerPen.SetEndCap(Gdiplus::LineCapRound);
        for (size_t i = 0; i < scene.leftWhiskerStart.size(); ++i) {
            graphics->DrawLine(&whiskerPen, scene.leftWhiskerStart[i], scene.leftWhiskerEnd[i]);
            graphics->DrawLine(&whiskerPen, scene.rightWhiskerStart[i], scene.rightWhiskerEnd[i]);
        }
    }

    {
        Gdiplus::Pen mouthPen(
            WithScaledAlpha(scene.mouthFill, scene.mouthFill.GetA(), scene.previewDetailAlphaScale),
            scene.mouthStrokeWidth);
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
        FillRoundedRect(
            graphics,
            scene.laneBadgeRects[i],
            WithScaledAlpha(fill, fill.GetA(), scene.previewAdornmentAlphaScale),
            WithScaledAlpha(scene.bodyStroke, scene.bodyStroke.GetA(), scene.previewAdornmentAlphaScale),
            0.8f);
    }

    if (scene.poseBadgeVisible) {
        FillEllipse(
            graphics,
            scene.poseBadgeRect,
            WithScaledAlpha(scene.accentFill, scene.poseBadgeAlpha, scene.previewAdornmentAlphaScale),
            WithScaledAlpha(scene.headFill, scene.poseBadgeAlpha * 0.85f, scene.previewAdornmentAlphaScale),
            1.0f);
    }
    if (scene.accessoryVisible) {
        const auto accessoryFill = WithScaledAlpha(
            scene.accessoryFill,
            scene.accessoryFill.GetA() * scene.accessoryAlphaScale,
            scene.previewAdornmentAlphaScale);
        const auto accessoryStroke = WithScaledAlpha(
            scene.accessoryStroke,
            scene.accessoryStroke.GetA() * std::min(1.0f, scene.accessoryAlphaScale * 0.96f),
            scene.previewAdornmentAlphaScale);
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

    DrawModelProxyImprint(graphics, scene);
}

} // namespace mousefx::windows

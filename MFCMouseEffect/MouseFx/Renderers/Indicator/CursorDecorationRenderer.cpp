#include "pch.h"

#include "MouseFx/Renderers/Indicator/CursorDecorationRenderer.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

namespace mousefx {
namespace {

int HexNibble(char ch) {
    if (ch >= '0' && ch <= '9') {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f') {
        return 10 + (ch - 'a');
    }
    if (ch >= 'A' && ch <= 'F') {
        return 10 + (ch - 'A');
    }
    return 0;
}

uint8_t HexByte(char high, char low) {
    return static_cast<uint8_t>((HexNibble(high) << 4) | HexNibble(low));
}

} // namespace

CursorDecorationLayout CursorDecorationRenderer::ResolveLayout(
    const InputIndicatorConfig::CursorDecorationConfig& config) const {
    const int sizePx = std::clamp(config.sizePx, 12, 72);
    CursorDecorationLayout layout{};
    layout.widthPx = std::max(36, sizePx * 4);
    layout.heightPx = std::max(36, sizePx * 4);
    layout.anchorOffsetXPx = layout.widthPx / 2;
    layout.anchorOffsetYPx = layout.heightPx / 2;
    return layout;
}

void CursorDecorationRenderer::Render(
    Gdiplus::Graphics& graphics,
    const InputIndicatorConfig::CursorDecorationConfig& config) const {
    const CursorDecorationLayout layout = ResolveLayout(config);
    const Gdiplus::Color accent = ParseHexColor(config.colorHex, config.alphaPercent);
    if (config.pluginId == "orb") {
        RenderOrb(graphics, config, layout, accent);
        return;
    }
    RenderRing(graphics, config, layout, accent);
}

Gdiplus::Color CursorDecorationRenderer::ParseHexColor(const std::string& colorHex, int alphaPercent) {
    const std::string safeHex = (colorHex.size() == 7 && colorHex[0] == '#')
        ? colorHex
        : std::string("#ff5a5a");
    const uint8_t alpha = static_cast<uint8_t>(
        std::clamp(alphaPercent, 15, 100) * 255 / 100);
    return Gdiplus::Color(
        alpha,
        HexByte(safeHex[1], safeHex[2]),
        HexByte(safeHex[3], safeHex[4]),
        HexByte(safeHex[5], safeHex[6]));
}

Gdiplus::Color CursorDecorationRenderer::WithScaledAlpha(
    const Gdiplus::Color& color,
    float scale) {
    const int scaledAlpha = static_cast<int>(std::lround(color.GetA() * std::clamp(scale, 0.0f, 1.0f)));
    return Gdiplus::Color(
        static_cast<BYTE>(std::clamp(scaledAlpha, 0, 255)),
        color.GetR(),
        color.GetG(),
        color.GetB());
}

void CursorDecorationRenderer::FillSoftEllipse(
    Gdiplus::Graphics& graphics,
    const Gdiplus::RectF& ellipse,
    const Gdiplus::Color& inner,
    const Gdiplus::Color& outer) {
    Gdiplus::GraphicsPath path;
    path.AddEllipse(ellipse);
    Gdiplus::PathGradientBrush brush(&path);
    brush.SetCenterColor(inner);
    std::array<Gdiplus::Color, 1> surround = {outer};
    int count = static_cast<int>(surround.size());
    brush.SetSurroundColors(surround.data(), &count);
    graphics.FillEllipse(&brush, ellipse);
}

void CursorDecorationRenderer::RenderRing(
    Gdiplus::Graphics& graphics,
    const InputIndicatorConfig::CursorDecorationConfig& config,
    const CursorDecorationLayout& layout,
    const Gdiplus::Color& accent) const {
    const float centerX = static_cast<float>(layout.anchorOffsetXPx);
    const float centerY = static_cast<float>(layout.anchorOffsetYPx);
    const float outerRadius = static_cast<float>(std::clamp(config.sizePx, 12, 72));
    const float innerRadius = outerRadius * 0.45f;

    FillSoftEllipse(
        graphics,
        Gdiplus::RectF(centerX - outerRadius * 1.45f, centerY - outerRadius * 1.45f, outerRadius * 2.9f, outerRadius * 2.9f),
        WithScaledAlpha(accent, 0.16f),
        Gdiplus::Color(0, accent.GetR(), accent.GetG(), accent.GetB()));

    Gdiplus::Pen outerRing(WithScaledAlpha(accent, 0.82f), std::max(1.6f, outerRadius * 0.16f));
    graphics.DrawEllipse(
        &outerRing,
        centerX - outerRadius,
        centerY - outerRadius,
        outerRadius * 2.0f,
        outerRadius * 2.0f);

    Gdiplus::Pen innerRing(WithScaledAlpha(accent, 0.36f), std::max(1.0f, outerRadius * 0.07f));
    graphics.DrawEllipse(
        &innerRing,
        centerX - innerRadius,
        centerY - innerRadius,
        innerRadius * 2.0f,
        innerRadius * 2.0f);

    Gdiplus::SolidBrush coreBrush(WithScaledAlpha(accent, 0.22f));
    graphics.FillEllipse(
        &coreBrush,
        centerX - innerRadius * 0.58f,
        centerY - innerRadius * 0.58f,
        innerRadius * 1.16f,
        innerRadius * 1.16f);
}

void CursorDecorationRenderer::RenderOrb(
    Gdiplus::Graphics& graphics,
    const InputIndicatorConfig::CursorDecorationConfig& config,
    const CursorDecorationLayout& layout,
    const Gdiplus::Color& accent) const {
    const float centerX = static_cast<float>(layout.anchorOffsetXPx);
    const float centerY = static_cast<float>(layout.anchorOffsetYPx);
    const float radius = static_cast<float>(std::clamp(config.sizePx, 12, 72));

    FillSoftEllipse(
        graphics,
        Gdiplus::RectF(centerX - radius * 1.8f, centerY - radius * 1.8f, radius * 3.6f, radius * 3.6f),
        WithScaledAlpha(accent, 0.18f),
        Gdiplus::Color(0, accent.GetR(), accent.GetG(), accent.GetB()));

    FillSoftEllipse(
        graphics,
        Gdiplus::RectF(centerX - radius, centerY - radius, radius * 2.0f, radius * 2.0f),
        WithScaledAlpha(accent, 0.86f),
        WithScaledAlpha(accent, 0.22f));

    Gdiplus::SolidBrush coreBrush(Gdiplus::Color(
        static_cast<BYTE>(std::min<int>(255, accent.GetA() + 26)),
        255,
        255,
        255));
    graphics.FillEllipse(
        &coreBrush,
        centerX - radius * 0.34f,
        centerY - radius * 0.34f,
        radius * 0.68f,
        radius * 0.68f);
}

} // namespace mousefx

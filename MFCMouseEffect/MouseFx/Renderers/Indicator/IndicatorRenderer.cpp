#include "pch.h"

#include "IndicatorRenderer.h"

#include <algorithm>
#include <cctype>
#include <cmath>

namespace mousefx {
namespace {

std::string NormalizeKeyLayoutMode(std::string mode) {
    std::transform(mode.begin(), mode.end(), mode.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    if (mode == "fixed_area") {
        return "fixed_area";
    }
    return "fixed_font";
}

Gdiplus::RectF BuildKeyPanelRect(float widthPx, float heightPx) {
    return Gdiplus::RectF(widthPx * 0.08f, heightPx * 0.31f, widthPx * 0.84f, heightPx * 0.38f);
}

float MeasureKeyLabelWidth(const std::wstring& text, float fontSizePx) {
    Gdiplus::Bitmap surface(1, 1, PixelFormat32bppPARGB);
    Gdiplus::Graphics graphics(&surface);
    Gdiplus::FontFamily family(L"Segoe UI");
    Gdiplus::Font font(&family, fontSizePx, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::StringFormat format;
    format.SetFormatFlags(format.GetFormatFlags() | Gdiplus::StringFormatFlagsNoWrap);
    format.SetTrimming(Gdiplus::StringTrimmingNone);
    Gdiplus::RectF measured;
    graphics.MeasureString(
        text.c_str(),
        static_cast<INT>(text.size()),
        &font,
        Gdiplus::PointF(0.0f, 0.0f),
        &format,
        &measured);
    return measured.Width;
}

float ResolveFittedKeyFontSize(
    Gdiplus::Graphics& g,
    const std::wstring& text,
    const Gdiplus::RectF& panel,
    const Gdiplus::FontFamily& family,
    Gdiplus::StringFormat* format,
    float initialSize) {
    const float minSize = std::max(6.0f, initialSize * 0.62f);
    const float widthBudget = std::max(10.0f, panel.Width - 10.0f);
    const float heightBudget = std::max(8.0f, panel.Height - 6.0f);
    float size = std::max(minSize, initialSize);

    for (int i = 0; i < 12; ++i) {
        Gdiplus::Font font(&family, size, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
        Gdiplus::RectF measured;
        g.MeasureString(
            text.c_str(),
            static_cast<INT>(text.size()),
            &font,
            Gdiplus::PointF(0.0f, 0.0f),
            format,
            &measured);

        if ((measured.Width <= widthBudget && measured.Height <= heightBudget) || size <= minSize) {
            return size;
        }
        size = std::max(minSize, size - 0.8f);
    }
    return size;
}

} // namespace

// ============================================================================
// Static helpers
// ============================================================================

Gdiplus::Color IndicatorRenderer::C(uint8_t a, uint8_t r, uint8_t g, uint8_t b) {
    return Gdiplus::Color(a, r, g, b);
}

float IndicatorRenderer::EaseOutCubic(float t) {
    const float x = std::clamp(t, 0.0f, 1.0f);
    const float inv = 1.0f - x;
    return 1.0f - inv * inv * inv;
}

void IndicatorRenderer::AddRoundedRectPath(Gdiplus::GraphicsPath& path,
                                           const Gdiplus::RectF& rect,
                                           float radius) {
    const float r = std::max(1.0f, std::min(radius, std::min(rect.Width, rect.Height) * 0.5f));
    const float d = r * 2.0f;
    path.Reset();
    path.AddArc(rect.X, rect.Y, d, d, 180.0f, 90.0f);
    path.AddArc(rect.GetRight() - d, rect.Y, d, d, 270.0f, 90.0f);
    path.AddArc(rect.GetRight() - d, rect.GetBottom() - d, d, d, 0.0f, 90.0f);
    path.AddArc(rect.X, rect.GetBottom() - d, d, d, 90.0f, 90.0f);
    path.CloseFigure();
}

IndicatorAnimParams IndicatorRenderer::ComputeAnimParams(float t) {
    IndicatorAnimParams p{};
    p.t = std::clamp(t, 0.0f, 1.0f);
    p.life = 1.0f - p.t;
    p.pulse = 0.90f + 0.10f * std::sin(p.t * 3.1415926f * 4.0f);
    p.eased = EaseOutCubic(p.t);
    p.alpha = static_cast<int>(230.0f * p.life);
    p.hotAlpha = static_cast<int>(std::min(255.0f, 170.0f * p.life * p.pulse));
    return p;
}

// ============================================================================
// Sub-element painters
// ============================================================================

void IndicatorRenderer::DrawMouseBody(Gdiplus::Graphics& g, int sizePx,
                                      const IndicatorAnimParams& anim) const {
    const float size = static_cast<float>(sizePx);
    const Gdiplus::RectF body(size * 0.12f, size * 0.07f, size * 0.76f, size * 0.86f);
    const float radius = size * 0.19f;

    // Drop shadow
    Gdiplus::GraphicsPath shadowPath;
    AddRoundedRectPath(shadowPath,
        Gdiplus::RectF(body.X + 2.0f, body.Y + 3.0f, body.Width, body.Height), radius);
    Gdiplus::SolidBrush shadowBrush(C(static_cast<uint8_t>(std::min(255, anim.alpha / 3)), 0, 0, 0));
    g.FillPath(&shadowBrush, &shadowPath);

    // Body fill + border
    Gdiplus::GraphicsPath bodyPath;
    AddRoundedRectPath(bodyPath, body, radius);
    Gdiplus::LinearGradientBrush bodyBrush(
        Gdiplus::PointF(body.X, body.Y),
        Gdiplus::PointF(body.X, body.GetBottom()),
        C(static_cast<uint8_t>(anim.alpha), 28, 44, 72),
        C(static_cast<uint8_t>(anim.alpha), 8, 15, 31));
    Gdiplus::Pen bodyPen(C(static_cast<uint8_t>(std::min(255, anim.alpha + 18)), 182, 205, 235), 1.8f);
    g.FillPath(&bodyBrush, &bodyPath);
    g.DrawPath(&bodyPen, &bodyPath);
}

void IndicatorRenderer::DrawButtonHighlight(Gdiplus::Graphics& g,
                                            const Gdiplus::GraphicsPath& clipPath,
                                            const Gdiplus::RectF& region,
                                            const Gdiplus::Color& top,
                                            const Gdiplus::Color& bottom,
                                            int /*hotAlpha*/) const {
    const Gdiplus::GraphicsState st = g.Save();
    g.SetClip(&clipPath, Gdiplus::CombineModeReplace);
    Gdiplus::LinearGradientBrush brush(
        Gdiplus::PointF(region.X, region.Y),
        Gdiplus::PointF(region.X, region.GetBottom()),
        top, bottom);
    g.FillRectangle(&brush, region);
    g.Restore(st);
}

void IndicatorRenderer::DrawWheel(Gdiplus::Graphics& g,
                                  const Gdiplus::RectF& wheelRect,
                                  bool active,
                                  const IndicatorAnimParams& anim) const {
    Gdiplus::SolidBrush base(C(static_cast<uint8_t>(anim.alpha), 142, 166, 200));
    g.FillEllipse(&base, wheelRect);

    if (active) {
        const int wheelA = static_cast<int>(std::min(255.0f, 210.0f * anim.life * anim.pulse));
        Gdiplus::SolidBrush glow(C(static_cast<uint8_t>(wheelA), 108, 236, 255));
        g.FillEllipse(&glow, wheelRect);
        Gdiplus::Pen ring(C(static_cast<uint8_t>(std::min(255, wheelA + 20)), 154, 244, 255), 1.0f);
        g.DrawEllipse(&ring, wheelRect);
    }
}

void IndicatorRenderer::DrawWheelArrow(Gdiplus::Graphics& g, int sizePx,
                                       const Gdiplus::RectF& body, bool up,
                                       const IndicatorAnimParams& anim) const {
    const float size = static_cast<float>(sizePx);
    const float cx = body.X + body.Width * 0.5f;
    // Move arrow to the middle-ish to avoid overlapping the bottom text (W+/W-)
    const float cy = body.Y + body.Height * 0.52f;
    const float shift = size * (0.016f + 0.028f * anim.eased);

    // Main arrow
    Gdiplus::PointF pts[3];
    if (up) {
        pts[0] = Gdiplus::PointF(cx, cy - shift - size * 0.06f);
        pts[1] = Gdiplus::PointF(cx - size * 0.045f, cy - shift + size * 0.02f);
        pts[2] = Gdiplus::PointF(cx + size * 0.045f, cy - shift + size * 0.02f);
    } else {
        pts[0] = Gdiplus::PointF(cx, cy + shift + size * 0.06f);
        pts[1] = Gdiplus::PointF(cx - size * 0.045f, cy + shift - size * 0.02f);
        pts[2] = Gdiplus::PointF(cx + size * 0.045f, cy + shift - size * 0.02f);
    }
    Gdiplus::SolidBrush arrBrush(C(static_cast<uint8_t>(std::min(255, anim.alpha + 10)), 124, 236, 255));
    g.FillPolygon(&arrBrush, pts, 3);

    // Trailing ghost arrow (fade effect)
    const float trail = size * 0.035f * anim.eased;
    const int trailAlpha = static_cast<int>(anim.alpha * 0.35f);
    Gdiplus::PointF pts2[3];
    if (up) {
        pts2[0] = Gdiplus::PointF(cx, pts[0].Y + trail);
        pts2[1] = Gdiplus::PointF(cx - size * 0.03f, pts[1].Y + trail);
        pts2[2] = Gdiplus::PointF(cx + size * 0.03f, pts[2].Y + trail);
    } else {
        pts2[0] = Gdiplus::PointF(cx, pts[0].Y - trail);
        pts2[1] = Gdiplus::PointF(cx - size * 0.03f, pts[1].Y - trail);
        pts2[2] = Gdiplus::PointF(cx + size * 0.03f, pts[2].Y - trail);
    }
    Gdiplus::SolidBrush trailBrush(C(static_cast<uint8_t>(trailAlpha), 124, 236, 255));
    g.FillPolygon(&trailBrush, pts2, 3);
}

void IndicatorRenderer::DrawClickRipple(Gdiplus::Graphics& g,
                                        const Gdiplus::RectF& region,
                                        const IndicatorAnimParams& anim) const {
    // Expanding ring effect on click
    const float cx = region.X + region.Width * 0.5f;
    const float cy = region.Y + region.Height * 0.5f;
    const float maxR = std::min(region.Width, region.Height) * 0.45f;
    const float r = maxR * anim.eased;
    const int rippleAlpha = static_cast<int>(120.0f * anim.life * anim.life);
    if (r > 1.0f && rippleAlpha > 2) {
        Gdiplus::Pen ripplePen(C(static_cast<uint8_t>(rippleAlpha), 200, 230, 255), 1.5f);
        g.DrawEllipse(&ripplePen, cx - r, cy - r, r * 2.0f, r * 2.0f);
    }
}

void IndicatorRenderer::DrawLabel(Gdiplus::Graphics& g, int sizePx,
                                  const Gdiplus::RectF& rect,
                                  const std::wstring& text,
                                  const IndicatorAnimParams& anim) const {
    if (text.empty()) return;
    const float size = static_cast<float>(sizePx);
    Gdiplus::FontFamily ff(L"Segoe UI");
    Gdiplus::Font font(&ff, size * 0.17f, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush textBrush(C(static_cast<uint8_t>(std::min(255, anim.alpha + 15)), 244, 250, 255));
    Gdiplus::StringFormat sf;
    sf.SetAlignment(Gdiplus::StringAlignmentCenter);
    sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    g.DrawString(text.c_str(), static_cast<INT>(text.size()), &font, rect, &sf, &textBrush);
}

// ============================================================================
// Public entry points
// ============================================================================

void IndicatorRenderer::RenderPointerAction(Gdiplus::Graphics& g, int sizePx,
                                          IndicatorEventKind kind,
                                          const IndicatorAnimParams& anim,
                                          const std::wstring& labelOverride) const {
    const float size = static_cast<float>(sizePx);
    const bool leftEvt  = (kind == IndicatorEventKind::Left1  || kind == IndicatorEventKind::Left2  || kind == IndicatorEventKind::Left3);
    const bool rightEvt = (kind == IndicatorEventKind::Right1 || kind == IndicatorEventKind::Right2 || kind == IndicatorEventKind::Right3);
    const bool middleEvt = (kind == IndicatorEventKind::Middle1 || kind == IndicatorEventKind::Middle2 || kind == IndicatorEventKind::Middle3);
    const bool wheelEvt = (kind == IndicatorEventKind::WheelUp || kind == IndicatorEventKind::WheelDown);

    // 1) Mouse body (shadow + fill + border)
    DrawMouseBody(g, sizePx, anim);

    // Re-derive body rect for child elements
    const Gdiplus::RectF body(size * 0.12f, size * 0.07f, size * 0.76f, size * 0.86f);
    const float radius = size * 0.19f;
    Gdiplus::GraphicsPath bodyPath;
    AddRoundedRectPath(bodyPath, body, radius);

    // 2) Button regions
    const float splitX = body.X + body.Width * 0.5f;
    const float topY = body.Y + body.Height * 0.34f;
    const Gdiplus::RectF leftTop(body.X, body.Y, body.Width * 0.5f, topY - body.Y);
    const Gdiplus::RectF rightTop(splitX, body.Y, body.Width * 0.5f, topY - body.Y);

    if (leftEvt) {
        DrawButtonHighlight(g, bodyPath, leftTop,
            C(static_cast<uint8_t>(anim.hotAlpha), 74, 160, 255),
            C(static_cast<uint8_t>(anim.hotAlpha / 2), 52, 98, 178),
            anim.hotAlpha);
        DrawClickRipple(g, leftTop, anim);
    }
    if (rightEvt) {
        DrawButtonHighlight(g, bodyPath, rightTop,
            C(static_cast<uint8_t>(anim.hotAlpha), 255, 150, 92),
            C(static_cast<uint8_t>(anim.hotAlpha / 2), 182, 94, 54),
            anim.hotAlpha);
        DrawClickRipple(g, rightTop, anim);
    }

    // 3) Separator lines
    Gdiplus::Pen sepPen(C(static_cast<uint8_t>(anim.alpha), 114, 140, 172), 1.1f);
    g.DrawLine(&sepPen, splitX, body.Y + 2.0f, splitX, topY);
    g.DrawLine(&sepPen, body.X + 2.0f, topY, body.GetRight() - 2.0f, topY);

    // 4) Scroll wheel
    Gdiplus::RectF wheelRect(splitX - body.Width * 0.078f, body.Y + body.Height * 0.086f,
                             body.Width * 0.156f, body.Height * 0.205f);
    DrawWheel(g, wheelRect, middleEvt || wheelEvt, anim);

    // 5) Event label text (L / R / M / W+ etc.)
    std::wstring labelText = labelOverride;
    if (labelText.empty()) {
        switch (kind) {
        case IndicatorEventKind::Left1:    labelText = L"L";  break;
        case IndicatorEventKind::Left2:    labelText = L"L2"; break;
        case IndicatorEventKind::Left3:    labelText = L"L3"; break;
        case IndicatorEventKind::Right1:   labelText = L"R";  break;
        case IndicatorEventKind::Right2:   labelText = L"R2"; break;
        case IndicatorEventKind::Right3:   labelText = L"R3"; break;
        case IndicatorEventKind::Middle1:  labelText = L"M";  break;
        case IndicatorEventKind::Middle2:  labelText = L"M2"; break;
        case IndicatorEventKind::Middle3:  labelText = L"M3"; break;
        case IndicatorEventKind::WheelUp:  labelText = L"W+"; break;
        case IndicatorEventKind::WheelDown:labelText = L"W-"; break;
        default: break;
        }
    }
    if (!labelText.empty()) {
        Gdiplus::RectF textRect(body.X, body.GetBottom() - size * 0.29f, body.Width, size * 0.23f);
        DrawLabel(g, sizePx, textRect, labelText, anim);
    }

    // 6) Wheel arrow animation
    if (wheelEvt) {
        DrawWheelArrow(g, sizePx, body, kind == IndicatorEventKind::WheelUp, anim);
    }

    // 7) Middle-click ripple
    if (middleEvt) {
        DrawClickRipple(g, wheelRect, anim);
    }
}

int IndicatorRenderer::ResolveKeyWindowWidthPx(
    int baseSizePx,
    const std::wstring& label,
    const std::string& layoutMode) const {
    const int base = std::max(40, baseSizePx);
    if (NormalizeKeyLayoutMode(layoutMode) != "fixed_font") {
        return base;
    }

    const std::wstring displayText = label.empty() ? L"Key" : label;
    const float baseHeight = static_cast<float>(base);
    const Gdiplus::RectF panel = BuildKeyPanelRect(baseHeight, baseHeight);
    const float defaultFontSize = baseHeight * 0.14f;
    const float measuredWidth = MeasureKeyLabelWidth(displayText, defaultFontSize);
    const float widthBudget = std::max(10.0f, panel.Width - 10.0f);
    if (measuredWidth <= widthBudget) {
        return base;
    }

    const float requiredPanelWidth = measuredWidth + 10.0f;
    const float requiredWindowWidth = requiredPanelWidth / 0.84f;
    const int expanded = static_cast<int>(std::ceil(requiredWindowWidth));
    const int maxWidth = std::max(base, base * 12);
    return std::clamp(expanded, base, maxWidth);
}

void IndicatorRenderer::RenderKeyAction(Gdiplus::Graphics& g, int widthPx, int heightPx,
                                        const std::wstring& label,
                                        const IndicatorAnimParams& anim,
                                        const std::string& layoutMode) const {
    const float width = static_cast<float>(std::max(1, widthPx));
    const float height = static_cast<float>(std::max(1, heightPx));
    const float unit = std::min(width, height);
    const Gdiplus::RectF panel = BuildKeyPanelRect(width, height);

    // Drop shadow
    Gdiplus::GraphicsPath shadowPath;
    AddRoundedRectPath(shadowPath,
        Gdiplus::RectF(panel.X + 2.0f, panel.Y + 3.0f, panel.Width, panel.Height),
        unit * 0.11f);
    Gdiplus::SolidBrush shadowBrush(C(static_cast<uint8_t>(std::min(255, anim.alpha / 3)), 0, 0, 0));
    g.FillPath(&shadowBrush, &shadowPath);

    // Panel fill + border
    Gdiplus::GraphicsPath panelPath;
    AddRoundedRectPath(panelPath, panel, unit * 0.11f);
    Gdiplus::LinearGradientBrush panelBrush(
        Gdiplus::PointF(panel.X, panel.Y),
        Gdiplus::PointF(panel.X, panel.GetBottom()),
        C(static_cast<uint8_t>(anim.alpha), 43, 66, 103),
        C(static_cast<uint8_t>(anim.alpha), 17, 30, 52));
    g.FillPath(&panelBrush, &panelPath);
    Gdiplus::Pen panelPen(C(static_cast<uint8_t>(std::min(255, anim.alpha + 18)), 190, 222, 255), 1.6f);
    g.DrawPath(&panelPen, &panelPath);

    // Key label text
    std::wstring displayText = label.empty() ? L"Key" : label;
    Gdiplus::FontFamily ff(L"Segoe UI");
    Gdiplus::StringFormat sf;
    sf.SetAlignment(Gdiplus::StringAlignmentCenter);
    sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    sf.SetFormatFlags(sf.GetFormatFlags() | Gdiplus::StringFormatFlagsNoWrap);
    sf.SetTrimming(Gdiplus::StringTrimmingNone);
    const float defaultFontSize = height * 0.14f;
    float fontSize = defaultFontSize;
    if (NormalizeKeyLayoutMode(layoutMode) == "fixed_area") {
        fontSize = ResolveFittedKeyFontSize(g, displayText, panel, ff, &sf, defaultFontSize);
    }
    Gdiplus::Font font(&ff, fontSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush textBrush(C(static_cast<uint8_t>(std::min(255, anim.alpha + 15)), 241, 247, 255));
    g.DrawString(displayText.c_str(), static_cast<INT>(displayText.size()),
                 &font, panel, &sf, &textBrush);
}

} // namespace mousefx

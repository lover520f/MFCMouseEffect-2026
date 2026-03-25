#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"

#include <gdiplus.h>

namespace mousefx {

struct CursorDecorationLayout {
    int widthPx = 72;
    int heightPx = 72;
    int anchorOffsetXPx = 36;
    int anchorOffsetYPx = 36;
};

class CursorDecorationRenderer final {
public:
    CursorDecorationLayout ResolveLayout(
        const InputIndicatorConfig::CursorDecorationConfig& config) const;

    void Render(
        Gdiplus::Graphics& graphics,
        const InputIndicatorConfig::CursorDecorationConfig& config) const;

private:
    static Gdiplus::Color ParseHexColor(const std::string& colorHex, int alphaPercent);
    static Gdiplus::Color WithScaledAlpha(const Gdiplus::Color& color, float scale);
    static void FillSoftEllipse(
        Gdiplus::Graphics& graphics,
        const Gdiplus::RectF& ellipse,
        const Gdiplus::Color& inner,
        const Gdiplus::Color& outer);
    void RenderRing(
        Gdiplus::Graphics& graphics,
        const InputIndicatorConfig::CursorDecorationConfig& config,
        const CursorDecorationLayout& layout,
        const Gdiplus::Color& accent) const;
    void RenderOrb(
        Gdiplus::Graphics& graphics,
        const InputIndicatorConfig::CursorDecorationConfig& config,
        const CursorDecorationLayout& layout,
        const Gdiplus::Color& accent) const;
};

} // namespace mousefx

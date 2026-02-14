#pragma once

#include <cstdint>
#include <string>
#include <gdiplus.h>

namespace mousefx {

// Describes which mouse/keyboard event triggered the indicator.
enum class IndicatorEventKind : uint8_t {
    None = 0,
    Left1, Left2, Left3,
    Right1, Right2, Right3,
    Middle1, Middle2, Middle3,
    WheelUp, WheelDown,
    KeyInput
};

// Pre-computed animation parameters passed to the renderer each frame.
struct IndicatorAnimParams {
    float t         = 0.0f;   // Normalised progress [0..1]
    float life      = 1.0f;   // 1-t, remaining life
    float pulse     = 1.0f;   // Subtle pulsing factor
    float eased     = 0.0f;   // EaseOutCubic(t)
    int   alpha     = 230;    // Overall opacity [0..255]
    int   hotAlpha  = 170;    // Highlight opacity
};

// Pure rendering helper – no window / timer / state management.
// Draws the indicator graphic onto a caller-supplied GDI+ Graphics.
class IndicatorRenderer final {
public:
    IndicatorRenderer() = default;

    // Draw the mouse action indicator (body + buttons + wheel + label).
    void RenderMouseAction(Gdiplus::Graphics& g, int sizePx,
                           IndicatorEventKind kind,
                           const IndicatorAnimParams& anim,
                           const std::wstring& labelOverride = {}) const;

    // Draw the keyboard key indicator (rounded panel + text label).
    void RenderKeyAction(Gdiplus::Graphics& g, int sizePx,
                         const std::wstring& label,
                         const IndicatorAnimParams& anim) const;

    // Compute animation parameters for the given progress.
    static IndicatorAnimParams ComputeAnimParams(float t);

private:
    // ---- drawing primitives ----
    static void AddRoundedRectPath(Gdiplus::GraphicsPath& path,
                                   const Gdiplus::RectF& rect, float radius);
    static Gdiplus::Color C(uint8_t a, uint8_t r, uint8_t g, uint8_t b);
    static float EaseOutCubic(float t);
    static std::wstring TruncateLabel(const std::wstring& src, size_t maxChars);

    // ---- sub-element painters ----
    void DrawMouseBody(Gdiplus::Graphics& g, int sizePx,
                       const IndicatorAnimParams& anim) const;
    void DrawButtonHighlight(Gdiplus::Graphics& g,
                             const Gdiplus::GraphicsPath& clipPath,
                             const Gdiplus::RectF& region,
                             const Gdiplus::Color& top,
                             const Gdiplus::Color& bottom,
                             int hotAlpha) const;
    void DrawWheel(Gdiplus::Graphics& g, const Gdiplus::RectF& wheelRect,
                   bool active, const IndicatorAnimParams& anim) const;
    void DrawWheelArrow(Gdiplus::Graphics& g, int sizePx,
                        const Gdiplus::RectF& body, bool up,
                        const IndicatorAnimParams& anim) const;
    void DrawClickRipple(Gdiplus::Graphics& g,
                         const Gdiplus::RectF& region,
                         const IndicatorAnimParams& anim) const;
    void DrawLabel(Gdiplus::Graphics& g, int sizePx,
                   const Gdiplus::RectF& rect,
                   const std::wstring& text,
                   const IndicatorAnimParams& anim) const;
};

} // namespace mousefx

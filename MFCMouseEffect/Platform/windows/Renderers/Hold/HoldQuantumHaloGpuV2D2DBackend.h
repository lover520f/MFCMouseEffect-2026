#pragma once

#include "MouseFx/Styles/RippleStyle.h"

#include <gdiplus.h>
#include <d2d1.h>
#include <dxgiformat.h>
#include <wrl/client.h>

#include <atomic>
#include <cstdint>

namespace mousefx {

class HoldQuantumHaloGpuV2D2DBackend final {
public:
    void ResetSession();
    bool IsAvailable() const;

    bool Render(
        Gdiplus::Graphics& g,
        float t,
        uint64_t elapsedMs,
        int sizePx,
        const RippleStyle& style,
        uint32_t holdMs,
        bool hasCursorState,
        int cursorScreenX,
        int cursorScreenY);

private:
    bool EnsureResources();

    void DrawLine(float x0, float y0, float x1, float y1, float width, float r, float g, float b, float a);
    void DrawFilledEllipse(float cx, float cy, float rx, float ry, float r, float g, float b, float a);
    void DrawEllipseOutline(float cx, float cy, float rx, float ry, float width, float r, float g, float b, float a);
    void DrawArcPolyline(float cx, float cy, float radius, float startRad, float sweepRad, float width, float r, float g, float b, float a, int segments);

    void MarkFailure();
    void MarkComExceptionFailure();

    Microsoft::WRL::ComPtr<ID2D1Factory> factory_{};
    Microsoft::WRL::ComPtr<ID2D1DCRenderTarget> target_{};
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush_{};
    int failureCount_ = 0;
    bool disabled_ = false;
    inline static std::atomic<bool> globalBlocked_{false};
};

} // namespace mousefx

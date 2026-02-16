#include "pch.h"
#include "HoldQuantumHaloGpuV2D2DBackend.h"
#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Utils/MathUtils.h"

#include <algorithm>
#include <cmath>
#include <comdef.h>

#pragma comment(lib, "d2d1.lib")

namespace mousefx {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static float ComputeProgress(float t01, uint64_t elapsedMs, uint32_t thresholdMs, uint32_t holdMs) {
    const float clampedT = ClampFloat(t01, 0.0f, 1.0f);
    const uint32_t threshold = (thresholdMs == 0) ? 1u : thresholdMs;
    if (holdMs == 0) return clampedT;
    const float holdT = ClampFloat(static_cast<float>(holdMs) / static_cast<float>(threshold), 0.0f, 1.0f);
    const float elapsedT = ClampFloat(static_cast<float>(elapsedMs) / static_cast<float>(threshold), 0.0f, 1.0f);
    return (holdT > elapsedT) ? holdT : elapsedT;
}

// ---------------------------------------------------------------------------
// Session
// ---------------------------------------------------------------------------

void HoldQuantumHaloGpuV2D2DBackend::ResetSession() {
    if (globalBlocked_.load()) {
        disabled_ = true;
        return;
    }
    disabled_ = false;
    failureCount_ = 0;
}

bool HoldQuantumHaloGpuV2D2DBackend::IsAvailable() const {
    return !disabled_ && !globalBlocked_.load();
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------

bool HoldQuantumHaloGpuV2D2DBackend::Render(
    Gdiplus::Graphics& g,
    float t,
    uint64_t elapsedMs,
    int sizePx,
    const RippleStyle& style,
    uint32_t holdMs,
    bool hasCursorState,
    int cursorScreenX,
    int cursorScreenY) {
    if (disabled_ || globalBlocked_.load()) return false;
    if (sizePx <= 0) return false;

    HDC hdc = nullptr;
    try {
        if (!EnsureResources()) {
            MarkFailure();
            return false;
        }

        Gdiplus::Matrix world;
        if (g.GetTransform(&world) != Gdiplus::Ok) {
            MarkFailure();
            return false;
        }
        Gdiplus::REAL m[6] = {};
        if (world.GetElements(m) != Gdiplus::Ok) {
            MarkFailure();
            return false;
        }

        hdc = g.GetHDC();
        if (!hdc) {
            MarkFailure();
            return false;
        }

        bool ok = false;
        do {
            LONG left = static_cast<LONG>(std::lround(m[4]));
            LONG top = static_cast<LONG>(std::lround(m[5]));
            const int dcW = GetDeviceCaps(hdc, HORZRES);
            const int dcH = GetDeviceCaps(hdc, VERTRES);
            if (hasCursorState) {
                POINT screenPt{};
                screenPt.x = cursorScreenX;
                screenPt.y = cursorScreenY;
                const POINT localPt = ScreenToOverlayPoint(screenPt);
                if (dcW > 0 && dcH > 0) {
                    if (localPt.x < -sizePx || localPt.x > dcW + sizePx ||
                        localPt.y < -sizePx || localPt.y > dcH + sizePx) {
                        ok = true;
                        break;
                    }
                }
                left = static_cast<LONG>(localPt.x - sizePx / 2);
                top = static_cast<LONG>(localPt.y - sizePx / 2);
            }

            LONG rcLeft = left;
            LONG rcTop = top;
            LONG rcRight = left + sizePx;
            LONG rcBottom = top + sizePx;
            if (dcW > 0 && dcH > 0) {
                rcLeft = (std::max)(0L, rcLeft);
                rcTop = (std::max)(0L, rcTop);
                rcRight = (std::min)(rcRight, static_cast<LONG>(dcW));
                rcBottom = (std::min)(rcBottom, static_cast<LONG>(dcH));
            }
            if (rcRight <= rcLeft || rcBottom <= rcTop) {
                ok = true;
                break;
            }
            RECT rc = { rcLeft, rcTop, rcRight, rcBottom };
            if (FAILED(target_->BindDC(hdc, &rc))) break;

            target_->BeginDraw();
            target_->SetTransform(D2D1::Matrix3x2F::Identity());
            target_->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

            const float progress = ComputeProgress(t, elapsedMs, style.durationMs, holdMs);
            const float timeSec = static_cast<float>(elapsedMs) / 1000.0f;
            const float cx = static_cast<float>(left) + sizePx * 0.5f;
            const float cy = static_cast<float>(top) + sizePx * 0.5f;
            const float radius = style.startRadius + (style.endRadius - style.startRadius) * progress;
            const float baseR = (std::min)(radius + 8.0f, sizePx * 0.43f);

            const uint32_t stroke = style.stroke.value;
            const float baseA = ClampFloat(static_cast<float>((stroke >> 24) & 0xFFu) / 255.0f, 0.06f, 1.0f);
            const float sr = static_cast<float>((stroke >> 16) & 0xFFu) / 255.0f;
            const float sg = static_cast<float>((stroke >> 8) & 0xFFu) / 255.0f;
            const float sb = static_cast<float>(stroke & 0xFFu) / 255.0f;

            const float pr = ClampFloat(sr * 0.72f + 0.28f, 0.0f, 1.0f);
            const float pg = ClampFloat(sg * 0.18f + 0.10f, 0.0f, 1.0f);
            const float pb = ClampFloat(sb * 0.95f + 0.05f, 0.0f, 1.0f);

            const float cr = ClampFloat(sr * 0.35f + 0.00f, 0.0f, 1.0f);
            const float cg = ClampFloat(sg * 0.90f + 0.20f, 0.0f, 1.0f);
            const float cb = ClampFloat(sb * 0.95f + 0.30f, 0.0f, 1.0f);

            // Outer glow shell.
            for (int i = 0; i < 4; ++i) {
                const float frac = (i + 1) / 4.0f;
                const float ringR = baseR + frac * 9.0f;
                const float pulse = 0.52f + 0.48f * std::sinf(timeSec * (1.3f + frac * 0.8f) + frac * 3.7f);
                const float alpha = baseA * (0.08f + 0.11f * (1.0f - frac)) * pulse;
                DrawEllipseOutline(cx, cy, ringR, ringR, 2.0f + frac * 2.0f, cr, cg, cb, alpha);
            }

            // Neon spokes.
            const int rayCount = 28;
            for (int i = 0; i < rayCount; ++i) {
                const float rayF = i / static_cast<float>(rayCount);
                const float ang = rayF * 6.2831853f + timeSec * 0.65f + std::sinf(timeSec * 1.8f + rayF * 7.0f) * 0.12f;
                const float inner = baseR * (0.36f + 0.06f * std::sinf(timeSec * 2.1f + i * 0.37f));
                const float outer = baseR * (0.76f + 0.10f * std::fabs(std::sinf(timeSec * 1.2f + i * 0.29f)));
                const float x0 = cx + std::cos(ang) * inner;
                const float y0 = cy + std::sin(ang) * inner;
                const float x1 = cx + std::cos(ang) * outer;
                const float y1 = cy + std::sin(ang) * outer;
                const float alpha = baseA * (0.05f + 0.15f * std::fabs(std::sinf(timeSec * 2.2f + i * 0.19f)));
                DrawLine(x0, y0, x1, y1, 1.0f, cr, cg, cb, alpha);
            }

            // Static track ring.
            DrawEllipseOutline(cx, cy, baseR, baseR, 2.8f, cr, cg, cb, baseA * 0.33f);

            // Progress arc from 12 o'clock.
            const float sweep = ClampFloat(progress, 0.0f, 1.0f) * 6.2831853f;
            DrawArcPolyline(cx, cy, baseR, -1.5707963f, sweep, 3.6f, pr, pg, pb, baseA * 0.82f, 84);

            // Weave arc fragments.
            for (int seg = 0; seg < 20; ++seg) {
                const float f = seg / 20.0f;
                const float a0 = -1.5707963f + f * 6.2831853f + timeSec * ((seg & 1) ? 0.25f : -0.22f);
                const float span = 0.08f + 0.11f * std::fabs(std::sinf(timeSec * 1.7f + seg * 0.27f));
                const float rr = baseR - 6.0f + std::sinf(timeSec * 1.3f + seg * 0.32f) * 1.6f;
                const float alpha = baseA * (0.09f + 0.16f * std::fabs(std::sinf(timeSec * 1.6f + seg * 0.31f)));
                DrawArcPolyline(cx, cy, rr, a0, span, 1.7f, cr, cg, cb, alpha, 8);
            }

            // Head capsule.
            const float head = -1.5707963f + sweep + timeSec * 0.28f;
            const float hx = cx + std::cos(head) * baseR;
            const float hy = cy + std::sin(head) * baseR;
            DrawFilledEllipse(hx, hy, 4.2f, 4.2f, pr, pg, pb, baseA * 0.94f);
            DrawFilledEllipse(cx, cy, (std::max)(2.2f, style.strokeWidth * 1.1f), (std::max)(2.2f, style.strokeWidth * 1.1f),
                1.0f, 1.0f, 1.0f, baseA * 0.45f);

            const HRESULT endHr = target_->EndDraw();
            if (FAILED(endHr)) break;
            ok = true;
        } while (false);

        g.ReleaseHDC(hdc);
        hdc = nullptr;

        if (!ok) {
            MarkFailure();
            return false;
        }
        return true;
    } catch (const _com_error&) {
        if (hdc) {
            g.ReleaseHDC(hdc);
            hdc = nullptr;
        }
        MarkComExceptionFailure();
        return false;
    } catch (...) {
        if (hdc) {
            g.ReleaseHDC(hdc);
            hdc = nullptr;
        }
        MarkComExceptionFailure();
        return false;
    }
}

// ---------------------------------------------------------------------------
// Resources
// ---------------------------------------------------------------------------

bool HoldQuantumHaloGpuV2D2DBackend::EnsureResources() {
    if (!factory_) {
        const HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, factory_.GetAddressOf());
        if (FAILED(hr)) return false;
    }
    if (!target_) {
        const D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            0.0f,
            0.0f,
            D2D1_RENDER_TARGET_USAGE_NONE,
            D2D1_FEATURE_LEVEL_DEFAULT);
        if (FAILED(factory_->CreateDCRenderTarget(&props, target_.GetAddressOf()))) return false;
    }
    if (!brush_) {
        if (FAILED(target_->CreateSolidColorBrush(D2D1::ColorF(1, 1, 1, 1), brush_.GetAddressOf()))) return false;
    }
    return true;
}

// ---------------------------------------------------------------------------
// Drawing primitives
// ---------------------------------------------------------------------------

void HoldQuantumHaloGpuV2D2DBackend::DrawLine(float x0, float y0, float x1, float y1, float width, float r, float g, float b, float a) {
    brush_->SetColor(D2D1::ColorF(r, g, b, a));
    target_->DrawLine(D2D1::Point2F(x0, y0), D2D1::Point2F(x1, y1), brush_.Get(), width);
}

void HoldQuantumHaloGpuV2D2DBackend::DrawFilledEllipse(float cx, float cy, float rx, float ry, float r, float g, float b, float a) {
    brush_->SetColor(D2D1::ColorF(r, g, b, a));
    target_->FillEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), rx, ry), brush_.Get());
}

void HoldQuantumHaloGpuV2D2DBackend::DrawEllipseOutline(float cx, float cy, float rx, float ry, float width, float r, float g, float b, float a) {
    brush_->SetColor(D2D1::ColorF(r, g, b, a));
    target_->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), rx, ry), brush_.Get(), width);
}

void HoldQuantumHaloGpuV2D2DBackend::DrawArcPolyline(
    float cx,
    float cy,
    float radius,
    float startRad,
    float sweepRad,
    float width,
    float r,
    float g,
    float b,
    float a,
    int segments) {
    if (segments < 1) segments = 1;
    if (radius <= 0.0f) return;
    if (std::fabs(sweepRad) < 0.0001f) return;
    brush_->SetColor(D2D1::ColorF(r, g, b, a));
    const float step = sweepRad / static_cast<float>(segments);
    float prevA = startRad;
    D2D1_POINT_2F prev = D2D1::Point2F(cx + std::cos(prevA) * radius, cy + std::sin(prevA) * radius);
    for (int i = 1; i <= segments; ++i) {
        const float aa = startRad + step * static_cast<float>(i);
        const D2D1_POINT_2F cur = D2D1::Point2F(cx + std::cos(aa) * radius, cy + std::sin(aa) * radius);
        target_->DrawLine(prev, cur, brush_.Get(), width);
        prev = cur;
    }
}

void HoldQuantumHaloGpuV2D2DBackend::MarkFailure() {
    ++failureCount_;
    if (failureCount_ >= 3) {
        disabled_ = true;
    }
}

void HoldQuantumHaloGpuV2D2DBackend::MarkComExceptionFailure() {
    disabled_ = true;
    globalBlocked_.store(true);
}

} // namespace mousefx

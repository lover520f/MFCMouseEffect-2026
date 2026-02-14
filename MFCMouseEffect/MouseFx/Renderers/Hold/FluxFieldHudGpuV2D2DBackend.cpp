#include "pch.h"
#include "FluxFieldHudGpuV2D2DBackend.h"
#include "MouseFx/Core/OverlayCoordSpace.h"
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

void FluxFieldHudGpuV2D2DBackend::ResetSession() {
    if (globalBlocked_.load()) {
        disabled_ = true;
        return;
    }
    disabled_ = false;
    failureCount_ = 0;
}

bool FluxFieldHudGpuV2D2DBackend::IsAvailable() const {
    return !disabled_ && !globalBlocked_.load();
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------

bool FluxFieldHudGpuV2D2DBackend::Render(
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
                        // keep matrix-derived left/top as fallback anchor
                    } else {
                        left = static_cast<LONG>(localPt.x - sizePx / 2);
                        top = static_cast<LONG>(localPt.y - sizePx / 2);
                    }
                } else {
                    left = static_cast<LONG>(localPt.x - sizePx / 2);
                    top = static_cast<LONG>(localPt.y - sizePx / 2);
                }
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
            const uint32_t stroke = style.stroke.value;
            const float baseAlpha = ClampFloat(static_cast<float>((stroke >> 24) & 0xFFu) / 255.0f, 0.05f, 1.0f);

            const float r = static_cast<float>((stroke >> 16) & 0xFFu) / 255.0f;
            const float gg = static_cast<float>((stroke >> 8) & 0xFFu) / 255.0f;
            const float b = static_cast<float>(stroke & 0xFFu) / 255.0f;

            for (int i = 0; i < 5; ++i) {
                const float frac = (i + 1) / 5.0f;
                const float ringR = radius * (0.35f + frac * 0.65f);
                const float pulse = 0.55f + 0.45f * std::sinf(timeSec * (1.2f + frac) + frac * 3.7f);
                const float alpha = baseAlpha * (0.12f + 0.18f * frac) * pulse;
                brush_->SetColor(D2D1::ColorF(r, gg, b, alpha));
                target_->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), ringR, ringR), brush_.Get(), 1.5f + frac * 2.2f);
            }

            const float mainR = radius * 0.92f;
            brush_->SetColor(D2D1::ColorF(r, gg, b, baseAlpha * 0.70f));
            target_->DrawEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), mainR, mainR), brush_.Get(), 3.0f);

            const float headAngle = progress * 6.2831853f + timeSec * 0.9f;
            const float hx = cx + std::cos(headAngle) * mainR;
            const float hy = cy + std::sin(headAngle) * mainR;
            brush_->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f, baseAlpha * 0.88f));
            target_->FillEllipse(D2D1::Ellipse(D2D1::Point2F(hx, hy), 2.8f, 2.8f), brush_.Get());

            // Fallback-safe center marker (cheap)
            brush_->SetColor(D2D1::ColorF(1.0f, 1.0f, 1.0f, baseAlpha * 0.42f));
            const float core = std::max(2.0f, style.strokeWidth * 1.4f);
            target_->FillEllipse(D2D1::Ellipse(D2D1::Point2F(cx, cy), core, core), brush_.Get());

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

bool FluxFieldHudGpuV2D2DBackend::EnsureResources() {
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

void FluxFieldHudGpuV2D2DBackend::MarkFailure() {
    ++failureCount_;
    if (failureCount_ >= 3) {
        disabled_ = true;
    }
}

void FluxFieldHudGpuV2D2DBackend::MarkComExceptionFailure() {
    disabled_ = true;
    globalBlocked_.store(true);
}

} // namespace mousefx

#pragma once

#include "MouseFx/Styles/RippleStyle.h"

#include <gdiplus.h>
#include <d2d1.h>
#include <dxgiformat.h>
#include <wrl/client.h>
#include <windows.h>

#include <cmath>
#include <cstdint>

namespace mousefx {

class FluxFieldHudGpuV2D2DBackend final {
public:
    void ResetSession() {
        disabled_ = false;
        failureCount_ = 0;
    }

    bool IsAvailable() const {
        return !disabled_;
    }

    bool Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style, uint32_t holdMs) {
        if (disabled_) return false;
        if (sizePx <= 0) return false;

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

        HDC hdc = g.GetHDC();
        if (!hdc) {
            MarkFailure();
            return false;
        }

        bool ok = false;
        do {
            RECT rc = { 0, 0, sizePx, sizePx };
            if (FAILED(target_->BindDC(hdc, &rc))) break;

            target_->BeginDraw();
            target_->SetTransform(D2D1::Matrix3x2F::Translation(m[4], m[5]));
            target_->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

            const float progress = ComputeProgress(t, elapsedMs, style.durationMs, holdMs);
            const float timeSec = static_cast<float>(elapsedMs) / 1000.0f;
            const float cx = sizePx * 0.5f;
            const float cy = sizePx * 0.5f;
            const float radius = style.startRadius + (style.endRadius - style.startRadius) * progress;
            const uint32_t stroke = style.stroke.value;
            const float baseAlpha = ClampF(static_cast<float>((stroke >> 24) & 0xFFu) / 255.0f, 0.05f, 1.0f);

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

        if (!ok) {
            MarkFailure();
            return false;
        }
        return true;
    }

private:
    bool EnsureResources() {
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

    static float ComputeProgress(float t01, uint64_t elapsedMs, uint32_t thresholdMs, uint32_t holdMs) {
        const float clampedT = ClampF(t01, 0.0f, 1.0f);
        const uint32_t threshold = (thresholdMs == 0) ? 1u : thresholdMs;
        if (holdMs == 0) return clampedT;
        const float holdT = ClampF(static_cast<float>(holdMs) / static_cast<float>(threshold), 0.0f, 1.0f);
        const float elapsedT = ClampF(static_cast<float>(elapsedMs) / static_cast<float>(threshold), 0.0f, 1.0f);
        return (holdT > elapsedT) ? holdT : elapsedT;
    }

    static float ClampF(float x, float lo, float hi) {
        if (x < lo) return lo;
        if (x > hi) return hi;
        return x;
    }

    void MarkFailure() {
        ++failureCount_;
        if (failureCount_ >= 3) {
            disabled_ = true;
        }
    }

    Microsoft::WRL::ComPtr<ID2D1Factory> factory_{};
    Microsoft::WRL::ComPtr<ID2D1DCRenderTarget> target_{};
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush_{};
    int failureCount_ = 0;
    bool disabled_ = false;
};

} // namespace mousefx

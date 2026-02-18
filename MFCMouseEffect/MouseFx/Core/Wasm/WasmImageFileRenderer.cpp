#include "pch.h"

#include "WasmImageFileRenderer.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

namespace mousefx::wasm {

namespace {

constexpr PROPID kPropertyTagFrameDelay = 0x5100;

float Clamp01(float value) {
    if (value < 0.0f) return 0.0f;
    if (value > 1.0f) return 1.0f;
    return value;
}

} // namespace

WasmImageFileRenderer::WasmImageFileRenderer(
    const std::wstring& imagePath,
    uint32_t tintArgb,
    bool applyTint,
    float alphaScale)
    : imagePath_(imagePath),
      tintArgb_(tintArgb),
      applyTint_(applyTint),
      alphaScale_(Clamp01(alphaScale)) {}

void WasmImageFileRenderer::Start(const RippleStyle& /*style*/) {
    if (!EnsureLoaded()) {
        return;
    }
    if (hasAnimatedFrames_) {
        image_->SelectActiveFrame(&frameDimension_, 0u);
    }
}

void WasmImageFileRenderer::SetParams(const RenderParams& params) {
    params_ = params;
}

void WasmImageFileRenderer::Render(
    Gdiplus::Graphics& g,
    float /*t*/,
    uint64_t elapsedMs,
    int sizePx,
    const RippleStyle& /*style*/) {
    if (!EnsureLoaded()) {
        return;
    }

    const UINT frameIndex = ResolveFrameIndex(elapsedMs);
    if (hasAnimatedFrames_) {
        image_->SelectActiveFrame(&frameDimension_, frameIndex);
    }

    const float imageW = static_cast<float>(image_->GetWidth());
    const float imageH = static_cast<float>(image_->GetHeight());
    if (imageW <= 0.0f || imageH <= 0.0f) {
        return;
    }

    const float targetMax = std::max(32.0f, static_cast<float>(sizePx) * 0.84f);
    const float scale = std::min(targetMax / imageW, targetMax / imageH);
    const float drawW = std::max(8.0f, imageW * scale);
    const float drawH = std::max(8.0f, imageH * scale);
    const float drawX = (static_cast<float>(sizePx) - drawW) * 0.5f;
    const float drawY = (static_cast<float>(sizePx) - drawH) * 0.5f;

    float matrixData[5][5] = {};
    ApplyTintColorMatrix(matrixData);
    Gdiplus::ColorMatrix matrix;
    std::memcpy(&matrix, matrixData, sizeof(matrix));

    Gdiplus::ImageAttributes attrs;
    attrs.SetColorMatrix(&matrix, Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);

    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
    g.DrawImage(
        image_.get(),
        Gdiplus::RectF(drawX, drawY, drawW, drawH),
        0.0f,
        0.0f,
        imageW,
        imageH,
        Gdiplus::UnitPixel,
        &attrs);
}

bool WasmImageFileRenderer::EnsureLoaded() {
    if (image_) {
        return true;
    }
    if (loadFailed_) {
        return false;
    }
    if (imagePath_.empty()) {
        loadFailed_ = true;
        return false;
    }

    std::unique_ptr<Gdiplus::Image> image(Gdiplus::Image::FromFile(imagePath_.c_str(), FALSE));
    if (!image || image->GetLastStatus() != Gdiplus::Ok) {
        loadFailed_ = true;
        return false;
    }

    image_ = std::move(image);
    BuildFrameTimeline();
    return true;
}

void WasmImageFileRenderer::BuildFrameTimeline() {
    frameDelaysMs_.clear();
    totalFrameMs_ = 0;
    hasAnimatedFrames_ = false;
    frameDimension_ = GUID{};

    if (!image_) {
        return;
    }
    const UINT dimCount = image_->GetFrameDimensionsCount();
    if (dimCount == 0) {
        return;
    }

    std::vector<GUID> dimensions(dimCount);
    if (image_->GetFrameDimensionsList(dimensions.data(), dimCount) != Gdiplus::Ok) {
        return;
    }

    frameDimension_ = dimensions[0];
    const UINT frameCount = image_->GetFrameCount(&frameDimension_);
    if (frameCount <= 1) {
        return;
    }

    hasAnimatedFrames_ = true;
    frameDelaysMs_.assign(frameCount, 100u);

    const UINT propSize = image_->GetPropertyItemSize(kPropertyTagFrameDelay);
    if (propSize == 0) {
        totalFrameMs_ = static_cast<uint64_t>(frameCount) * 100u;
        return;
    }

    std::vector<BYTE> propData(propSize);
    auto* item = reinterpret_cast<Gdiplus::PropertyItem*>(propData.data());
    if (image_->GetPropertyItem(kPropertyTagFrameDelay, propSize, item) != Gdiplus::Ok) {
        totalFrameMs_ = static_cast<uint64_t>(frameCount) * 100u;
        return;
    }

    const UINT delayCount = std::min<UINT>(frameCount, item->length / sizeof(uint32_t));
    const auto* delayValues = reinterpret_cast<const uint32_t*>(item->value);
    for (UINT i = 0; i < delayCount; ++i) {
        // GIF uses 1/100 sec; clamp to avoid zero-delay frames.
        const uint32_t ms = std::max<uint32_t>(20u, delayValues[i] * 10u);
        frameDelaysMs_[i] = ms;
    }

    for (uint32_t delay : frameDelaysMs_) {
        totalFrameMs_ += delay;
    }
}

UINT WasmImageFileRenderer::ResolveFrameIndex(uint64_t elapsedMs) const {
    if (!hasAnimatedFrames_ || frameDelaysMs_.empty()) {
        return 0u;
    }
    if (totalFrameMs_ == 0) {
        return 0u;
    }

    uint64_t cursorMs = elapsedMs;
    if (params_.loop) {
        cursorMs = cursorMs % totalFrameMs_;
    } else if (cursorMs >= totalFrameMs_) {
        return static_cast<UINT>(frameDelaysMs_.size() - 1u);
    }

    uint64_t acc = 0;
    for (UINT i = 0; i < frameDelaysMs_.size(); ++i) {
        acc += frameDelaysMs_[i];
        if (cursorMs < acc) {
            return i;
        }
    }
    return static_cast<UINT>(frameDelaysMs_.size() - 1u);
}

void WasmImageFileRenderer::ApplyTintColorMatrix(float outMatrix[5][5]) const {
    for (int r = 0; r < 5; ++r) {
        for (int c = 0; c < 5; ++c) {
            outMatrix[r][c] = (r == c) ? 1.0f : 0.0f;
        }
    }

    const float effectiveAlpha = Clamp01(alphaScale_ * std::max(0.0f, params_.intensity));
    outMatrix[3][3] = effectiveAlpha;

    if (!applyTint_) {
        return;
    }

    const float tintA = static_cast<float>((tintArgb_ >> 24) & 0xFFu) / 255.0f;
    const float tintR = static_cast<float>((tintArgb_ >> 16) & 0xFFu) / 255.0f;
    const float tintG = static_cast<float>((tintArgb_ >> 8) & 0xFFu) / 255.0f;
    const float tintB = static_cast<float>(tintArgb_ & 0xFFu) / 255.0f;

    outMatrix[0][0] = tintR;
    outMatrix[1][1] = tintG;
    outMatrix[2][2] = tintB;
    outMatrix[3][3] = Clamp01(effectiveAlpha * tintA);
}

} // namespace mousefx::wasm

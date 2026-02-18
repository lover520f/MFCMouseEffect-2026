#pragma once

#include <gdiplus.h>

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "MouseFx/Interfaces/IRippleRenderer.h"

namespace mousefx::wasm {

class WasmImageFileRenderer final : public IRippleRenderer {
public:
    WasmImageFileRenderer(
        const std::wstring& imagePath,
        uint32_t tintArgb,
        bool applyTint,
        float alphaScale);

    void Start(const RippleStyle& style) override;
    void SetParams(const RenderParams& params) override;
    void Render(
        Gdiplus::Graphics& g,
        float t,
        uint64_t elapsedMs,
        int sizePx,
        const RippleStyle& style) override;

private:
    bool EnsureLoaded();
    void BuildFrameTimeline();
    UINT ResolveFrameIndex(uint64_t elapsedMs) const;
    void ApplyTintColorMatrix(float outMatrix[5][5]) const;

    std::wstring imagePath_{};
    uint32_t tintArgb_ = 0;
    bool applyTint_ = false;
    float alphaScale_ = 1.0f;
    RenderParams params_{};
    std::unique_ptr<Gdiplus::Image> image_{};
    std::vector<uint32_t> frameDelaysMs_{};
    uint64_t totalFrameMs_ = 0;
    GUID frameDimension_{};
    bool hasAnimatedFrames_ = false;
    bool loadFailed_ = false;
};

} // namespace mousefx::wasm


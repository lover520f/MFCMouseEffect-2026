#pragma once

#include "HoldNeon3DRenderer.h"

namespace mousefx {

// Stage-1 Dawn-native route placeholder:
// keep a dedicated effect id so we can evolve renderer internals
// without changing legacy hold_neon3d behavior.
class HoldNeon3DGpuV2Renderer final : public IRippleRenderer {
public:
    void Start(const RippleStyle& style) override {
        impl_.Start(style);
    }

    void OnCommand(const std::string& cmd, const std::string& args) override {
        impl_.OnCommand(cmd, args);
    }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        impl_.Render(g, t, elapsedMs, sizePx, style);
    }

private:
    HoldNeon3DRenderer impl_{};
};

REGISTER_RENDERER("hold_neon3d_gpu_v2", HoldNeon3DGpuV2Renderer)

} // namespace mousefx

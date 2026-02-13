#pragma once

#include "FluxFieldHudCpuRenderer.h"

namespace mousefx {

// Stability-first fallback implementation.
// Keep dedicated GPU-v2 route id for pipeline wiring/diagnostics while
// rendering uses proven CPU path until GPU backend is fully stabilized.
class FluxFieldHudGpuV2Renderer final : public IRippleRenderer {
public:
    void Start(const RippleStyle& style) override { cpuImpl_.Start(style); }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        cpuImpl_.Render(g, t, elapsedMs, sizePx, style);
    }

    void OnCommand(const std::string& cmd, const std::string& args) override { cpuImpl_.OnCommand(cmd, args); }

private:
    FluxFieldHudCpuRenderer cpuImpl_{};
};

REGISTER_RENDERER("hold_fluxfield_gpu_v2", FluxFieldHudGpuV2Renderer)

} // namespace mousefx


#pragma once

#include "FluxFieldHudCpuRenderer.h"

namespace mousefx {

// GPU route placeholder for FluxField HUD.
// Stage-1 keeps visual parity with CPU renderer while runtime/path gating is validated.
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

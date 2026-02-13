#pragma once

#include "FluxFieldHudCpuRenderer.h"
#include <cstdio>

namespace mousefx {

// Stability-first fallback implementation.
// Keep dedicated GPU-v2 route id for pipeline wiring/diagnostics while
// rendering uses proven CPU path until GPU backend is fully stabilized.
class FluxFieldHudGpuV2Renderer final : public IRippleRenderer {
public:
    void Start(const RippleStyle& style) override {
        state_ = {};
        cpuImpl_.Start(style);
    }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        cpuImpl_.Render(g, t, elapsedMs, sizePx, style);
    }

    void OnCommand(const std::string& cmd, const std::string& args) override {
        if (cmd == "hold_state") {
            uint32_t ms = 0;
            int x = 0;
            int y = 0;
            if (sscanf_s(args.c_str(), "%u,%d,%d", &ms, &x, &y) >= 1) {
                state_.active = true;
                state_.holdMs = ms;
                state_.cursorX = x;
                state_.cursorY = y;
            }
        }
        cpuImpl_.OnCommand(cmd, args);
    }

private:
    struct HoldState {
        bool active = false;
        uint32_t holdMs = 0;
        int cursorX = 0;
        int cursorY = 0;
    };

    FluxFieldHudCpuRenderer cpuImpl_{};
    HoldState state_{};
};

REGISTER_RENDERER("hold_fluxfield_gpu_v2", FluxFieldHudGpuV2Renderer)

} // namespace mousefx

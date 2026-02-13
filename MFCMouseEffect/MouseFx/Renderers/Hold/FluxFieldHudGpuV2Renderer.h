#pragma once

#include "FluxFieldHudCpuRenderer.h"
#include "FluxFieldHudGpuV2D2DBackend.h"
#include <cstdio>
#include <filesystem>
#include <string>

namespace mousefx {

// Stability-first fallback implementation.
// Keep dedicated GPU-v2 route id for pipeline wiring/diagnostics while
// rendering uses proven CPU path until GPU backend is fully stabilized.
class FluxFieldHudGpuV2Renderer final : public IRippleRenderer {
public:
    void Start(const RippleStyle& style) override {
        state_ = {};
        d2dExperimentalEnabled_ = IsD2dExperimentalEnabled();
        if (d2dExperimentalEnabled_) {
            d2dBackend_.ResetSession();
        }
        cpuImpl_.Start(style);
    }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        if (d2dExperimentalEnabled_ && d2dBackend_.IsAvailable()) {
            if (d2dBackend_.Render(g, t, elapsedMs, sizePx, style, state_.holdMs)) {
                return;
            }
        }
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
    static bool IsD2dExperimentalEnabled() {
        wchar_t envValue[16] = {};
        const DWORD envLen = GetEnvironmentVariableW(L"MFX_FLUX_GPU_V2_D2D", envValue, 16);
        if (envLen > 0 && envLen < 16) {
            std::wstring v(envValue, envValue + envLen);
            for (wchar_t& c : v) {
                if (c >= L'A' && c <= L'Z') c = (wchar_t)(c - L'A' + L'a');
            }
            if (v == L"1" || v == L"true" || v == L"on") return true;
        }

        wchar_t exePath[MAX_PATH] = {};
        GetModuleFileNameW(nullptr, exePath, MAX_PATH);
        std::filesystem::path p(exePath);
        p = p.parent_path() / L".local" / L"diag" / L"flux_gpu_v2_d2d.on";
        return std::filesystem::exists(p);
    }

    struct HoldState {
        bool active = false;
        uint32_t holdMs = 0;
        int cursorX = 0;
        int cursorY = 0;
    };

    FluxFieldHudCpuRenderer cpuImpl_{};
    FluxFieldHudGpuV2D2DBackend d2dBackend_{};
    HoldState state_{};
    bool d2dExperimentalEnabled_ = false;
};

REGISTER_RENDERER("hold_fluxfield_gpu_v2", FluxFieldHudGpuV2Renderer)

} // namespace mousefx

#pragma once

#include "FluxFieldGpuV2ComputeEngine.h"
#include "FluxFieldHudGpuV2VisualRenderer.h"
#include "MouseFx/Core/ConfigPathResolver.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace mousefx {

class FluxFieldHudGpuV2Renderer final : public IRippleRenderer {
public:
    void Start(const RippleStyle& style) override {
        state_ = {};
        experimentalFlag_ = false;
        visualImpl_.Start(style);
        gpuStarted_ = gpuCompute_.Start();
    }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        if (gpuStarted_ && gpuCompute_.IsActive()) {
            gpuCompute_.Tick(elapsedMs, state_.holdMs);
        }
        visualImpl_.Render(g, t, elapsedMs, sizePx, style);
    }

    void OnCommand(const std::string& cmd, const std::string& args) override {
        if (cmd == "gpu_v2_d2d_experimental") {
            std::string value = args;
            for (char& c : value) {
                if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
            }
            experimentalFlag_ = (value == "1" || value == "true" || value == "on");
        }
        if (cmd == "hold_state") {
            uint32_t ms = 0;
            int x = 0;
            int y = 0;
            if (sscanf_s(args.c_str(), "%u,%d,%d", &ms, &x, &y) >= 1) {
                state_.active = true;
                state_.holdMs = ms;
                state_.cursorX = x;
                state_.cursorY = y;
                if (ms == 0) {
                    state_.active = false;
                    WriteGpuSnapshot();
                }
            }
        }
        visualImpl_.OnCommand(cmd, args);
    }

private:
    void WriteGpuSnapshot() const {
        const std::wstring diagDir = ResolveLocalDiagDirectory();
        if (diagDir.empty()) return;
        std::error_code ec;
        std::filesystem::create_directories(diagDir, ec);
        if (ec) return;
        const std::filesystem::path outFile =
            std::filesystem::path(diagDir) / L"flux_gpu_v2_compute_status_auto.json";
        std::ofstream out(outFile, std::ios::binary | std::ios::trunc);
        if (!out.is_open()) return;
        const auto snap = gpuCompute_.GetSnapshot();
        std::ostringstream ss;
        ss << "{"
           << "\"gpu_started\":" << (gpuStarted_ ? "true" : "false") << ","
           << "\"gpu_active\":" << (snap.active ? "true" : "false") << ","
           << "\"gpu_reason\":\"" << snap.reason << "\","
           << "\"gpu_tick_count\":" << snap.tickCount << ","
           << "\"gpu_last_passes\":" << snap.lastPasses << ","
           << "\"experimental_flag\":" << (experimentalFlag_ ? "true" : "false")
           << "}";
        out << ss.str();
    }

    struct HoldState {
        bool active = false;
        uint32_t holdMs = 0;
        int cursorX = 0;
        int cursorY = 0;
    };

    FluxFieldHudGpuV2VisualRenderer visualImpl_{};
    FluxFieldGpuV2ComputeEngine gpuCompute_{};
    HoldState state_{};
    bool gpuStarted_ = false;
    bool experimentalFlag_ = false;
};

REGISTER_RENDERER("hold_fluxfield_gpu_v2", FluxFieldHudGpuV2Renderer)

} // namespace mousefx

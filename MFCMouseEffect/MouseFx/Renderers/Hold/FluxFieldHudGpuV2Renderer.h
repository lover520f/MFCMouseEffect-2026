#pragma once

#include "FluxFieldGpuV2ComputeEngine.h"
#include "FluxFieldHudGpuV2D2DBackend.h"
#include "MouseFx/Effects/HoldRouteCatalog.h"
#include "MouseFx/Core/Config/ConfigPathResolver.h"

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
        style_ = style;
        renderAttemptCount_ = 0;
        renderSuccessCount_ = 0;
        renderFailureCount_ = 0;
        gpuVisualBackend_.ResetSession();
        gpuStarted_ = gpuCompute_.Start();
        gpuVisualActive_ = gpuVisualBackend_.IsAvailable();
        cpuFallbackActive_ = false;
        if (!gpuStarted_) {
            routeReason_ = "gpu_compute_start_failed";
            gpuVisualActive_ = false;
            return;
        }
        routeReason_ = gpuVisualActive_ ? "gpu_d2d_visual_active" : "gpu_visual_unavailable";
    }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        if (!gpuStarted_ || !gpuCompute_.IsActive() || !gpuVisualActive_) {
            return;
        }
        ++renderAttemptCount_;
        gpuCompute_.Tick(elapsedMs, state_.holdMs);

        const bool ok = gpuVisualBackend_.Render(
            g,
            t,
            elapsedMs,
            sizePx,
            style_,
            state_.holdMs,
            state_.hasCursorState,
            state_.cursorX,
            state_.cursorY);
        if (!ok) {
            ++renderFailureCount_;
            gpuVisualActive_ = false;
            routeReason_ = "gpu_visual_runtime_failed";
            WriteGpuSnapshot();
            return;
        }
        ++renderSuccessCount_;
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
                state_.hasCursorState = true;
            }
            return;
        }
        if (cmd == "hold_end") {
            state_.active = false;
            state_.holdMs = 0;
            state_.hasCursorState = false;
            WriteGpuSnapshot();
            return;
        }
        (void)cmd;
        (void)args;
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
           << "\"gpu_visual_active\":" << (gpuVisualActive_ ? "true" : "false") << ","
           << "\"cpu_fallback_active\":" << (cpuFallbackActive_ ? "true" : "false") << ","
           << "\"render_attempt_count\":" << renderAttemptCount_ << ","
           << "\"render_success_count\":" << renderSuccessCount_ << ","
           << "\"render_failure_count\":" << renderFailureCount_ << ","
           << "\"route_reason\":\"" << routeReason_ << "\""
           << "}";
        out << ss.str();
    }

    struct HoldState {
        bool active = false;
        uint32_t holdMs = 0;
        bool hasCursorState = false;
        int cursorX = 0;
        int cursorY = 0;
    };

    RippleStyle style_{};
    FluxFieldHudGpuV2D2DBackend gpuVisualBackend_{};
    FluxFieldGpuV2ComputeEngine gpuCompute_{};
    HoldState state_{};
    bool gpuStarted_ = false;
    bool gpuVisualActive_ = false;
    bool cpuFallbackActive_ = false;
    uint64_t renderAttemptCount_ = 0;
    uint64_t renderSuccessCount_ = 0;
    uint64_t renderFailureCount_ = 0;
    std::string routeReason_ = "uninitialized";
};

REGISTER_RENDERER(mousefx::hold_route::kTypeFluxFieldGpuV2, FluxFieldHudGpuV2Renderer)

} // namespace mousefx

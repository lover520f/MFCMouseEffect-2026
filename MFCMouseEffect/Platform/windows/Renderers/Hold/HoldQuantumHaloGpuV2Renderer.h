#pragma once

#include "MouseFx/Renderers/RendererRegistry.h"
#include "Platform/windows/Renderers/Hold/QuantumHaloGpuV2ComputeEngine.h"
#include "MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterHost.h"
#include "MouseFx/Core/Config/ConfigPathResolver.h"
#include "MouseFx/Core/System/CursorPositionProvider.h"
#include "MouseFx/Effects/HoldRouteCatalog.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace mousefx {

class HoldQuantumHaloGpuV2Renderer final : public IRippleRenderer {
public:
    ~HoldQuantumHaloGpuV2Renderer() override {
        gpuPresenter_.Shutdown();
    }

    void Start(const RippleStyle& style) override {
        state_ = {};
        gpuVisualRenderedLastFrame_ = false;
        gpuVisualSubmitCount_ = 0;
        gpuVisualMissCount_ = 0;
        sawPositiveHold_ = false;
        gpuStarted_ = gpuCompute_.Start();
        presenterStarted_ = gpuPresenter_.Start();
    }

    void OnCommand(const std::string& cmd, const std::string& args) override {
        if (cmd == "hold_ms") {
            uint32_t ms = 0;
            if (sscanf_s(args.c_str(), "%u", &ms) == 1) {
                state_.holdMs = ms;
                if (ms > 0) sawPositiveHold_ = true;
            }
            return;
        }
        if (cmd == "hold_state") {
            uint32_t ms = 0;
            int x = 0;
            int y = 0;
            if (sscanf_s(args.c_str(), "%u,%d,%d", &ms, &x, &y) >= 1) {
                state_.active = true;
                state_.holdMs = ms;
                state_.hasCursorState = true;
                state_.cursorX = x;
                state_.cursorY = y;
                if (ms > 0) {
                    sawPositiveHold_ = true;
                }
                if (ms == 0 && sawPositiveHold_) {
                    state_.active = false;
                    WriteGpuSnapshot();
                    sawPositiveHold_ = false;
                }
            }
        }
    }

    void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) override {
        (void)g;
        if (gpuStarted_ && gpuCompute_.IsActive()) {
            gpuCompute_.Tick(elapsedMs, state_.holdMs);
        }

        if (!state_.hasCursorState) {
            ScreenPoint pt{};
            if (TryGetCursorScreenPoint(&pt)) {
                state_.hasCursorState = true;
                state_.cursorX = pt.x;
                state_.cursorY = pt.y;
            }
        }

        gpuVisualRenderedLastFrame_ = false;
        if (!presenterStarted_) {
            presenterStarted_ = gpuPresenter_.Start();
        }
        if (presenterStarted_ && gpuPresenter_.IsReady() && state_.hasCursorState) {
            gpuVisualRenderedLastFrame_ = gpuPresenter_.RenderFrame(
                state_.cursorX,
                state_.cursorY,
                sizePx,
                t,
                elapsedMs,
                state_.holdMs,
                style);
        }

        if (gpuVisualRenderedLastFrame_) {
            ++gpuVisualSubmitCount_;
        } else {
            ++gpuVisualMissCount_;
        }
    }

private:
    void WriteGpuSnapshot() const {
        const std::wstring diagDir = ResolveLocalDiagDirectory();
        if (diagDir.empty()) return;
        std::error_code ec;
        std::filesystem::create_directories(diagDir, ec);
        if (ec) return;
        const std::filesystem::path outFile =
            std::filesystem::path(diagDir) / L"quantum_halo_gpu_v2_compute_status_auto.json";
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
           << "\"visual_gpu_rendered_last_frame\":" << (gpuVisualRenderedLastFrame_ ? "true" : "false") << ","
           << "\"visual_gpu_available\":" << (gpuPresenter_.IsReady() ? "true" : "false") << ","
           << "\"presenter_backend_preference\":\"" << gpuPresenter_.PreferredBackendName() << "\","
           << "\"presenter_active_backend\":\"" << gpuPresenter_.ActiveBackendName() << "\","
           << "\"presenter_last_error\":\"" << gpuPresenter_.LastErrorReason() << "\","
           << "\"presenter_last_backend_failure\":\"" << gpuPresenter_.LastBackendFailureReason() << "\","
           << "\"presenter_backend_switch_count\":" << gpuPresenter_.BackendSwitchCount() << ","
           << "\"visual_submit_count\":" << gpuVisualSubmitCount_ << ","
           << "\"visual_miss_count\":" << gpuVisualMissCount_
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

    QuantumHaloGpuV2ComputeEngine gpuCompute_{};
    QuantumHaloPresenterHost gpuPresenter_{};
    HoldState state_{};
    bool gpuStarted_ = false;
    bool presenterStarted_ = false;
    bool sawPositiveHold_ = false;
    bool gpuVisualRenderedLastFrame_ = false;
    uint64_t gpuVisualSubmitCount_ = 0;
    uint64_t gpuVisualMissCount_ = 0;
};

REGISTER_RENDERER(mousefx::hold_route::kTypeQuantumHaloGpuV2, HoldQuantumHaloGpuV2Renderer)
static mousefx::RendererRegistrar<HoldQuantumHaloGpuV2Renderer> reg_HoldQuantumHaloGpuV2RendererCompat(mousefx::hold_route::kTypeQuantumHaloGpuV2Legacy);

} // namespace mousefx

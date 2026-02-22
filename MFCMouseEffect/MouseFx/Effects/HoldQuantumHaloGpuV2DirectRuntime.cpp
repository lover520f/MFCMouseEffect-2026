#include "pch.h"
#include "HoldQuantumHaloGpuV2DirectRuntime.h"

#include "MouseFx/Core/Config/ConfigPathResolver.h"
#include "MouseFx/Renderers/Hold/QuantumHaloGpuV2ComputeEngine.h"
#include "MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterHost.h"
#include "MouseFx/Utils/MathUtils.h"

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace mousefx {

// ---------------------------------------------------------------------------
// Helpers (file-scope)
// ---------------------------------------------------------------------------

static float ComputeProgress(uint64_t elapsedMs, uint32_t holdMs, uint32_t durationMs) {
    const float elapsedT = ClampFloat(static_cast<float>(elapsedMs) / static_cast<float>(durationMs), 0.0f, 1.0f);
    if (holdMs == 0u) return elapsedT;
    const float holdT = ClampFloat(static_cast<float>(holdMs) / static_cast<float>(durationMs), 0.0f, 1.0f);
    return (holdT > elapsedT) ? holdT : elapsedT;
}

static std::string HrToHex(HRESULT hr) {
    std::ostringstream ss;
    ss << "0x" << std::uppercase << std::hex << static_cast<unsigned long>(hr);
    return ss.str();
}

static void WriteSnapshot(
    const QuantumHaloGpuV2ComputeEngine::Snapshot& snap,
    bool presenterReady,
    bool renderedLastFrame,
    uint64_t submitCount,
    uint64_t missCount,
    const std::string& runtimeReason,
    const std::string& presenterPreference,
    const std::string& presenterBackend,
    const std::string& presenterLastError,
    const std::string& presenterLastBackendFailure,
    uint32_t presenterBackendSwitchCount) {
    const std::wstring diagDir = ResolveLocalDiagDirectory();
    if (diagDir.empty()) return;

    std::error_code ec;
    std::filesystem::create_directories(diagDir, ec);
    if (ec) return;

    const std::filesystem::path outFile =
        std::filesystem::path(diagDir) / L"quantum_halo_gpu_v2_compute_status_auto.json";
    std::ofstream out(outFile, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return;

    std::ostringstream ss;
    ss << "{"
       << "\"gpu_started\":" << (snap.active ? "true" : "false") << ","
       << "\"gpu_active\":" << (snap.active ? "true" : "false") << ","
       << "\"gpu_reason\":\"" << snap.reason << "\","
       << "\"gpu_tick_count\":" << snap.tickCount << ","
       << "\"gpu_last_passes\":" << snap.lastPasses << ","
       << "\"visual_gpu_rendered_last_frame\":" << (renderedLastFrame ? "true" : "false") << ","
       << "\"visual_gpu_available\":" << (presenterReady ? "true" : "false") << ","
       << "\"presenter_backend_preference\":\"" << presenterPreference << "\","
       << "\"presenter_active_backend\":\"" << presenterBackend << "\","
       << "\"presenter_last_error\":\"" << presenterLastError << "\","
       << "\"presenter_last_backend_failure\":\"" << presenterLastBackendFailure << "\","
       << "\"presenter_backend_switch_count\":" << presenterBackendSwitchCount << ","
       << "\"visual_submit_count\":" << submitCount << ","
       << "\"visual_miss_count\":" << missCount << ","
       << "\"runtime_reason\":\"" << runtimeReason << "\""
       << "}";
    out << ss.str();
}

// ---------------------------------------------------------------------------
// HoldQuantumHaloGpuV2DirectRuntime
// ---------------------------------------------------------------------------

HoldQuantumHaloGpuV2DirectRuntime::~HoldQuantumHaloGpuV2DirectRuntime() {
    Stop();
}

bool HoldQuantumHaloGpuV2DirectRuntime::Start(const RippleStyle& style, const ScreenPoint& startPt) {
    Stop();

    style_ = style;
    style_.windowSize = ClampInt(style_.windowSize, 96, 640);
    if (style_.durationMs == 0u) {
        style_.durationMs = 1u;
    }

    holdMs_.store(0u, std::memory_order_relaxed);
    cursorX_.store(startPt.x, std::memory_order_relaxed);
    cursorY_.store(startPt.y, std::memory_order_relaxed);
    submitCount_.store(0ull, std::memory_order_relaxed);
    missCount_.store(0ull, std::memory_order_relaxed);

    stopEvent_ = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (!stopEvent_) {
        return false;
    }

    stopRequested_.store(false, std::memory_order_release);
    running_.store(true, std::memory_order_release);
    worker_ = std::thread(&HoldQuantumHaloGpuV2DirectRuntime::WorkerMain, this);
    return true;
}

void HoldQuantumHaloGpuV2DirectRuntime::Update(uint32_t holdMs, const ScreenPoint& pt) {
    if (!running_.load(std::memory_order_acquire)) {
        return;
    }
    holdMs_.store(holdMs, std::memory_order_relaxed);
    cursorX_.store(pt.x, std::memory_order_relaxed);
    cursorY_.store(pt.y, std::memory_order_relaxed);
}

void HoldQuantumHaloGpuV2DirectRuntime::Stop() {
    stopRequested_.store(true, std::memory_order_release);
    if (stopEvent_) {
        SetEvent(stopEvent_);
    }
    if (worker_.joinable()) {
        worker_.join();
    }
    running_.store(false, std::memory_order_release);
    if (stopEvent_) {
        CloseHandle(stopEvent_);
        stopEvent_ = nullptr;
    }
}

bool HoldQuantumHaloGpuV2DirectRuntime::IsRunning() const {
    return running_.load(std::memory_order_acquire);
}

void HoldQuantumHaloGpuV2DirectRuntime::WorkerMain() {
    std::string runtimeReason = "ok";

    const HRESULT comHr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    const bool comInitialized = (comHr == S_OK) || (comHr == S_FALSE);
    if (!comInitialized && comHr != RPC_E_CHANGED_MODE) {
        runtimeReason = "coinit_failed_" + HrToHex(comHr);
    }

    QuantumHaloGpuV2ComputeEngine compute{};
    QuantumHaloPresenterHost presenter{};

    const bool computeStarted = (runtimeReason == "ok") ? compute.Start() : false;
    if (!computeStarted && runtimeReason == "ok") {
        runtimeReason = "compute_start_failed";
    }

    const bool presenterStarted = (runtimeReason == "ok") ? presenter.Start() : false;
    if (!presenterStarted && runtimeReason == "ok") {
        runtimeReason = "presenter_start_failed_" + presenter.LastErrorReason();
    }
    bool presenterReady = presenterStarted && presenter.IsReady();
    if (!presenterReady && runtimeReason == "ok") {
        runtimeReason = "presenter_not_ready_" + presenter.LastErrorReason();
    }

    const uint64_t startTick = GetTickCount64();
    uint64_t lastComputeTickMs = 0;
    bool renderedLastFrame = false;

    while (!stopRequested_.load(std::memory_order_acquire)) {
        MSG msg{};
        while (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        const uint64_t nowMs = GetTickCount64();
        const uint64_t elapsedMs = (nowMs >= startTick) ? (nowMs - startTick) : 0;
        const uint32_t holdMs = holdMs_.load(std::memory_order_relaxed);
        const int cursorX = cursorX_.load(std::memory_order_relaxed);
        const int cursorY = cursorY_.load(std::memory_order_relaxed);

        if (computeStarted && compute.IsActive()) {
            if ((nowMs - lastComputeTickMs) >= 33u) {
                compute.Tick(elapsedMs, holdMs);
                lastComputeTickMs = nowMs;
            }
        }

        renderedLastFrame = false;
        if (presenterReady) {
            const float t = ComputeProgress(elapsedMs, holdMs, style_.durationMs);
            renderedLastFrame = presenter.RenderFrame(
                cursorX,
                cursorY,
                style_.windowSize,
                t,
                elapsedMs,
                holdMs,
                style_);
            if (!renderedLastFrame && runtimeReason == "ok") {
                runtimeReason = "render_frame_false_" + presenter.LastErrorReason();
            }
        }

        if (renderedLastFrame) {
            submitCount_.fetch_add(1ull, std::memory_order_relaxed);
            runtimeReason = "ok";
        } else {
            missCount_.fetch_add(1ull, std::memory_order_relaxed);
        }

        presenterReady = presenter.IsReady();
        if (!presenterReady && runtimeReason == "ok") {
            runtimeReason = "presenter_became_not_ready_" + presenter.LastErrorReason();
        }

        if (stopEvent_) {
            const HANDLE handles[1] = { stopEvent_ };
            const DWORD waitResult = MsgWaitForMultipleObjects(1, handles, FALSE, 8u, QS_ALLINPUT);
            if (waitResult == WAIT_OBJECT_0) {
                break;
            }
        } else {
            Sleep(8u);
        }
    }

    const auto snap = compute.GetSnapshot();
    WriteSnapshot(
        snap,
        presenterReady,
        renderedLastFrame,
        submitCount_.load(std::memory_order_relaxed),
        missCount_.load(std::memory_order_relaxed),
        runtimeReason,
        presenter.PreferredBackendName(),
        presenter.ActiveBackendName(),
        presenter.LastErrorReason(),
        presenter.LastBackendFailureReason(),
        presenter.BackendSwitchCount());

    presenter.Shutdown();
    compute.Shutdown();
    if (comInitialized) {
        CoUninitialize();
    }
}

} // namespace mousefx

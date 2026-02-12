#pragma once

#include <windows.h>
#include <gdiplus.h>

#include <atomic>
#include <cstdint>
#include <mutex>
#include <memory>
#include <string>
#include <vector>

#include "MouseFx/Gpu/OverlayGpuCommandStream.h"
#include "MouseFx/Interfaces/IOverlayLayer.h"
#include "MouseFx/Windows/DawnGpuPresenter.h"
#include "MouseFx/Windows/OverlayLayeredCpuPresenter.h"
#include "MouseFx/Gpu/GpuFinalPresentPolicy.h"

namespace mousefx {

class OverlayHostWindow final {
public:
    OverlayHostWindow();
    ~OverlayHostWindow();

    bool Create();
    void Shutdown();

    IOverlayLayer* AddLayer(std::unique_ptr<IOverlayLayer> layer);
    void RemoveLayer(IOverlayLayer* layer);
    void ClearLayers();
    void SetGpuSubmitContext(const std::string& activeBackend, const std::string& pipelineMode);
    uint64_t GetLastGpuCommandFrameTickMs() const;
    uint32_t GetLastGpuCommandCount() const;
    uint32_t GetLastGpuTrailCommandCount() const;
    uint32_t GetLastGpuRippleCommandCount() const;
    uint32_t GetLastGpuParticleCommandCount() const;
    uint64_t GetGpuPresentAttemptCount() const;
    uint64_t GetGpuPresentSuccessCount() const;
    uint64_t GetGpuPresentFallbackCount() const;
    std::string GetGpuPresentLastDetail() const;
    bool IsGpuPresentActive() const;
    gpu::GpuFinalPresentPolicyDecision GetGpuFinalPresentPolicyDecision() const;

    struct HostSurface {
        HWND hwnd = nullptr;
        HDC memDc = nullptr;
        HBITMAP dib = nullptr;
        void* bits = nullptr;
        int x = 0;
        int y = 0;
        int width = 0;
        int height = 0;
        bool hadVisibleContent = false;
    };

private:
    static constexpr UINT_PTR kTimerId = 5;
    static constexpr UINT kMsgEnsureTopmost = WM_APP + 0x33;

    static void CALLBACK ForegroundEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD eventTime);

    static const wchar_t* ClassName();
    static bool EnsureClassRegistered();
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT OnMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void OnTick();
    void Render();
    void RenderSurface(HostSurface& surface);
    void CollectGpuCommandStream(uint64_t nowMs);
    void UpdateFrameLoopTimerInterval(UINT intervalMs);
    UINT ResolveNextTimerIntervalMs() const;
    gpu::GpuFinalPresentPolicyDecision EvaluateGpuFinalPresentPolicy() const;
    bool ShouldUseLayeredSurfaces() const;
    bool EnsureSurfaceMode(bool forceRebuild);
    bool RebuildSurfaces();
    void DestroySurfaces();
    void SyncBoundsWithVirtualScreen(bool forceMove);
    void StartFrameLoop();
    void StopFrameLoop();
    void EnsureTopmostZOrder(bool force = false);
    void RegisterForegroundHook();
    void UnregisterForegroundHook();

    std::vector<HostSurface> surfaces_{};
    HWND timerHwnd_ = nullptr;
    int virtualX_ = 0;
    int virtualY_ = 0;
    int virtualW_ = 0;
    int virtualH_ = 0;
    bool ticking_ = false;
    bool timerResolutionRaised_ = false;
    UINT currentTimerIntervalMs_ = 0;
    uint64_t lastTopmostEnsureMs_ = 0;
    HWINEVENTHOOK foregroundHook_ = nullptr;
    std::vector<std::unique_ptr<IOverlayLayer>> layers_{};
    gpu::OverlayGpuCommandStream gpuCommandStream_{};
    std::atomic<uint64_t> gpuCommandFrameTickMs_{0};
    std::atomic<uint32_t> gpuCommandCount_{0};
    std::atomic<uint32_t> gpuTrailCommandCount_{0};
    std::atomic<uint32_t> gpuRippleCommandCount_{0};
    std::atomic<uint32_t> gpuParticleCommandCount_{0};
    std::atomic<uint32_t> gpuRippleHoldCommandCount_{0};
    std::atomic<uint64_t> lastNonEmptyGpuCommandTickMs_{0};
    std::atomic<uint64_t> gpuPresentAttemptCount_{0};
    std::atomic<uint64_t> gpuPresentSuccessCount_{0};
    std::atomic<uint64_t> gpuPresentFallbackCount_{0};
    mutable std::mutex gpuPresentDetailMutex_{};
    std::string gpuPresentLastDetail_ = "cpu_present_path";
    DawnGpuPresenter dawnPresenter_{};
    OverlayLayeredCpuPresenter cpuPresenter_{};
    std::string gpuSubmitActiveBackend_ = "cpu";
    std::string gpuSubmitPipelineMode_ = "cpu_layered";
    bool useLayeredSurfaces_ = true;
    std::atomic<bool> forceLayeredCpuFallback_{false};
    std::atomic<bool> pendingLayeredRollback_{false};
    std::atomic<uint32_t> gpuPresentConsecutiveFailures_{0};
};

} // namespace mousefx

#pragma once

#include <windows.h>
#include <gdiplus.h>

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "MouseFx/Gpu/OverlayGpuCommandStream.h"
#include "MouseFx/Interfaces/IOverlayLayer.h"

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
    uint64_t lastTopmostEnsureMs_ = 0;
    HWINEVENTHOOK foregroundHook_ = nullptr;
    std::vector<std::unique_ptr<IOverlayLayer>> layers_{};
    gpu::OverlayGpuCommandStream gpuCommandStream_{};
    std::atomic<uint64_t> gpuCommandFrameTickMs_{0};
    std::atomic<uint32_t> gpuCommandCount_{0};
    std::atomic<uint32_t> gpuTrailCommandCount_{0};
    std::atomic<uint32_t> gpuRippleCommandCount_{0};
    std::atomic<uint32_t> gpuParticleCommandCount_{0};
    std::string gpuSubmitActiveBackend_ = "cpu";
    std::string gpuSubmitPipelineMode_ = "cpu_layered";
};

} // namespace mousefx

#include "pch.h"

#include "DawnGpuPresenter.h"

#include "MouseFx/Gpu/DawnOverlayBridge.h"
#include "MouseFx/Gpu/DawnRuntime.h"
#include "MouseFx/Gpu/DawnSurfaceCommandStats.h"
#include "MouseFx/Gpu/DawnSurfaceInterop.h"

namespace mousefx {

DawnGpuPresenter::~DawnGpuPresenter() {
    std::lock_guard<std::mutex> lock(surfaceMutex_);
    gpu::ReleaseDawnSurfaceInteropState(&surfaceState_);
}

bool DawnGpuPresenter::Present(const OverlayPresentFrame& frame) {
    attempts_.fetch_add(1, std::memory_order_relaxed);

    const gpu::DawnRuntimeStatus runtime = gpu::GetDawnRuntimeStatusFast();
    const gpu::DawnOverlayBridgeStatus bridge = gpu::GetDawnOverlayBridgeStatus();

    if (!runtime.queueReady) {
        std::lock_guard<std::mutex> lock(detailMutex_);
        lastDetail_ = "gpu_present_queue_not_ready";
        return false;
    }
    if (bridge.mode != "compositor" || !bridge.compositorApisReady) {
        std::lock_guard<std::mutex> lock(detailMutex_);
        lastDetail_ = "gpu_present_bridge_not_ready";
        return false;
    }

    // Layered overlay windows are driven by UpdateLayeredWindow alpha composition.
    // Direct Dawn surface present on this kind of HWND can turn into full-screen black
    // on some drivers/compositor paths, so keep CPU as the authoritative presenter here.
    const LONG_PTR exStyle = GetWindowLongPtrW(frame.hwnd, GWL_EXSTYLE);
    if ((exStyle & WS_EX_LAYERED) != 0) {
        std::lock_guard<std::mutex> lock(detailMutex_);
        lastDetail_ = "gpu_present_layered_hwnd_requires_cpu_fallback";
        return false;
    }

    const gpu::DawnSurfaceCommandStats stats = gpu::BuildDawnSurfaceCommandStats(frame.gpuCommandStream);

    std::string interopDetail;
    const gpu::DawnRuntimePresentContext presentContext = gpu::GetDawnRuntimePresentContext();
    bool hadGpuVisualContent = false;
    bool interopOk = false;
    {
        std::lock_guard<std::mutex> lock(surfaceMutex_);
        interopOk = gpu::PresentDawnSurfaceClearPass(
            &surfaceState_,
            presentContext,
            reinterpret_cast<void*>(frame.hwnd),
            frame.width,
            frame.height,
            frame.gpuCommandStream,
            &hadGpuVisualContent,
            &interopDetail);
    }
    if (!interopOk) {
        std::lock_guard<std::mutex> lock(detailMutex_);
        lastDetail_ = interopDetail.empty() ? "gpu_present_interop_failed" : interopDetail;
        return false;
    }

    std::lock_guard<std::mutex> lock(detailMutex_);
    lastDetail_ = interopDetail.empty() ? "gpu_present_surface_submit_ok" : interopDetail;
    if (!hadGpuVisualContent && stats.commandCount == 0) {
        lastDetail_ += "_clearpass_no_visible_commands";
    }
    return true;
}

std::string DawnGpuPresenter::LastDetail() const {
    std::lock_guard<std::mutex> lock(detailMutex_);
    return lastDetail_;
}

} // namespace mousefx

#pragma once

#include <string>

#include "DawnParticleSurfaceRenderer.h"
#include "DawnRippleSurfaceRenderer.h"
#include "DawnTrailSurfaceRenderer.h"

namespace mousefx::gpu {

struct DawnRuntimePresentContext;
class OverlayGpuCommandStream;

struct DawnSurfaceInteropState {
    void* surface = nullptr;
    void* hwnd = nullptr;
    int width = 0;
    int height = 0;
    void* procSurfaceUnconfigure = nullptr;
    void* procSurfaceRelease = nullptr;
    DawnTrailSurfaceRendererState trailRenderer{};
    DawnParticleSurfaceRendererState particleRenderer{};
    DawnRippleSurfaceRendererState rippleRenderer{};
};

void ReleaseDawnSurfaceInteropState(DawnSurfaceInteropState* state) noexcept;
bool PresentDawnSurfaceClearPass(
    DawnSurfaceInteropState* state,
    const DawnRuntimePresentContext& context,
    void* hwnd,
    int width,
    int height,
    const OverlayGpuCommandStream* commandStream,
    bool* hadGpuVisualContentOut,
    std::string* detailOut);

} // namespace mousefx::gpu

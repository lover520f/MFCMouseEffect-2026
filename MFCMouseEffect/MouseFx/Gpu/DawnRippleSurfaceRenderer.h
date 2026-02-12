#pragma once

#include <cstdint>
#include <string>

namespace mousefx::gpu {

struct DawnRuntimePresentContext;
class OverlayGpuCommandStream;

struct DawnRippleSurfaceRendererState {
    void* shaderModule = nullptr;
    void* renderPipeline = nullptr;
    void* vertexBuffer = nullptr;
    uint64_t vertexBufferBytes = 0;
    void* procShaderModuleRelease = nullptr;
    void* procRenderPipelineRelease = nullptr;
    void* procBufferRelease = nullptr;
};

void ReleaseDawnRippleSurfaceRendererState(DawnRippleSurfaceRendererState* state) noexcept;
bool EncodeDawnRippleSurfacePass(
    DawnRippleSurfaceRendererState* state,
    const DawnRuntimePresentContext& context,
    const OverlayGpuCommandStream* commandStream,
    int surfaceWidth,
    int surfaceHeight,
    void* renderPassEncoder,
    uint32_t* drawnPulseCountOut,
    std::string* detailOut);

} // namespace mousefx::gpu

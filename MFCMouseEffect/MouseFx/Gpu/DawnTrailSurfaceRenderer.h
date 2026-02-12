#pragma once

#include <cstdint>
#include <string>

namespace mousefx::gpu {

struct DawnRuntimePresentContext;
class OverlayGpuCommandStream;

struct DawnTrailSurfaceRendererState {
    void* shaderModule = nullptr;
    void* renderPipeline = nullptr;
    void* vertexBuffer = nullptr;
    uint64_t vertexBufferBytes = 0;
    void* procShaderModuleRelease = nullptr;
    void* procRenderPipelineRelease = nullptr;
    void* procBufferRelease = nullptr;
};

void ReleaseDawnTrailSurfaceRendererState(DawnTrailSurfaceRendererState* state) noexcept;
bool EncodeDawnTrailSurfacePass(
    DawnTrailSurfaceRendererState* state,
    const DawnRuntimePresentContext& context,
    const OverlayGpuCommandStream* commandStream,
    int surfaceWidth,
    int surfaceHeight,
    void* renderPassEncoder,
    uint32_t* drawnTrailTrianglesOut,
    std::string* detailOut);

} // namespace mousefx::gpu

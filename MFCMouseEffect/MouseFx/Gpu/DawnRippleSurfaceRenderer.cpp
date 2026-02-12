#include "pch.h"

#include "DawnRippleSurfaceRenderer.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "DawnRuntime.h"
#include "OverlayGpuCommandStream.h"

namespace mousefx::gpu {
namespace {

struct WGPUChainedStructRaw {
    const WGPUChainedStructRaw* next = nullptr;
    uint32_t sType = 0;
};

struct WGPUStringViewRaw {
    const char* data = nullptr;
    size_t length = static_cast<size_t>(-1);
};

struct WGPUShaderSourceWGSLRaw {
    WGPUChainedStructRaw chain{};
    WGPUStringViewRaw code{};
};

struct WGPUShaderModuleDescriptorRaw {
    WGPUChainedStructRaw* nextInChain = nullptr;
    WGPUStringViewRaw label{};
};

struct WGPUBufferDescriptorRaw {
    WGPUChainedStructRaw* nextInChain = nullptr;
    WGPUStringViewRaw label{};
    uint64_t usage = 0;
    uint64_t size = 0;
    uint32_t mappedAtCreation = 0;
};

struct WGPUVertexAttributeRaw {
    WGPUChainedStructRaw* nextInChain = nullptr;
    uint32_t format = 0;
    uint64_t offset = 0;
    uint32_t shaderLocation = 0;
};

struct WGPUVertexBufferLayoutRaw {
    WGPUChainedStructRaw* nextInChain = nullptr;
    uint32_t stepMode = 0;
    uint64_t arrayStride = 0;
    size_t attributeCount = 0;
    const WGPUVertexAttributeRaw* attributes = nullptr;
};

struct WGPUBlendComponentRaw {
    uint32_t operation = 0;
    uint32_t srcFactor = 0;
    uint32_t dstFactor = 0;
};

struct WGPUBlendStateRaw {
    WGPUBlendComponentRaw color{};
    WGPUBlendComponentRaw alpha{};
};

struct WGPUColorTargetStateRaw {
    WGPUChainedStructRaw* nextInChain = nullptr;
    uint32_t format = 0;
    const WGPUBlendStateRaw* blend = nullptr;
    uint64_t writeMask = 0;
};

struct WGPUVertexStateRaw {
    WGPUChainedStructRaw* nextInChain = nullptr;
    void* module = nullptr;
    WGPUStringViewRaw entryPoint{};
    size_t constantCount = 0;
    const void* constants = nullptr;
    size_t bufferCount = 0;
    const WGPUVertexBufferLayoutRaw* buffers = nullptr;
};

struct WGPUFragmentStateRaw {
    WGPUChainedStructRaw* nextInChain = nullptr;
    void* module = nullptr;
    WGPUStringViewRaw entryPoint{};
    size_t constantCount = 0;
    const void* constants = nullptr;
    size_t targetCount = 0;
    const WGPUColorTargetStateRaw* targets = nullptr;
};

struct WGPUPrimitiveStateRaw {
    WGPUChainedStructRaw* nextInChain = nullptr;
    uint32_t topology = 0;
    uint32_t stripIndexFormat = 0;
    uint32_t frontFace = 0;
    uint32_t cullMode = 0;
    uint32_t unclippedDepth = 0;
};

struct WGPUMultisampleStateRaw {
    WGPUChainedStructRaw* nextInChain = nullptr;
    uint32_t count = 1;
    uint32_t mask = 0xFFFFFFFFu;
    uint32_t alphaToCoverageEnabled = 0;
};

struct WGPURenderPipelineDescriptorRaw {
    WGPUChainedStructRaw* nextInChain = nullptr;
    WGPUStringViewRaw label{};
    void* layout = nullptr;
    WGPUVertexStateRaw vertex{};
    WGPUPrimitiveStateRaw primitive{};
    const void* depthStencil = nullptr;
    WGPUMultisampleStateRaw multisample{};
    const WGPUFragmentStateRaw* fragment = nullptr;
};

using PFN_wgpuDeviceCreateShaderModule = void* (*)(void*, const WGPUShaderModuleDescriptorRaw*);
using PFN_wgpuShaderModuleRelease = void (*)(void*);
using PFN_wgpuDeviceCreateRenderPipeline = void* (*)(void*, const WGPURenderPipelineDescriptorRaw*);
using PFN_wgpuRenderPipelineRelease = void (*)(void*);
using PFN_wgpuDeviceCreateBuffer = void* (*)(void*, const WGPUBufferDescriptorRaw*);
using PFN_wgpuBufferRelease = void (*)(void*);
using PFN_wgpuQueueWriteBuffer = void (*)(void*, void*, uint64_t, const void*, size_t);
using PFN_wgpuRenderPassEncoderSetPipeline = void (*)(void*, void*);
using PFN_wgpuRenderPassEncoderSetVertexBuffer = void (*)(void*, uint32_t, void*, uint64_t, uint64_t);
using PFN_wgpuRenderPassEncoderDraw = void (*)(void*, uint32_t, uint32_t, uint32_t, uint32_t);

constexpr uint32_t kWGPUSTypeShaderSourceWGSL = 0x00000002;
constexpr uint32_t kWGPUTextureFormatBGRA8Unorm = 0x0000001B;
constexpr uint32_t kWGPUVertexFormatFloat32x2 = 0x0000001D;
constexpr uint32_t kWGPUVertexFormatFloat32x4 = 0x0000001F;
constexpr uint32_t kWGPUVertexStepModeVertex = 0x00000001;
constexpr uint32_t kWGPUPrimitiveTopologyTriangleList = 0x00000004;
constexpr uint32_t kWGPUBlendOperationAdd = 0x00000001;
constexpr uint32_t kWGPUBlendFactorOne = 0x00000002;
constexpr uint32_t kWGPUBlendFactorOneMinusSrcAlpha = 0x00000006;
constexpr uint64_t kWGPUColorWriteMaskAll = 0x000000000000000FULL;
constexpr uint64_t kWGPUBufferUsageCopyDst = 0x0000000000000008ULL;
constexpr uint64_t kWGPUBufferUsageVertex = 0x0000000000000020ULL;

struct RippleVertexGpu {
    float posX = 0.0f;
    float posY = 0.0f;
    float uvX = 0.0f;
    float uvY = 0.0f;
    float colorR = 0.0f;
    float colorG = 0.0f;
    float colorB = 0.0f;
    float colorA = 0.0f;
};

constexpr const char* kRippleShaderWgsl = R"(
struct VSIn {
    @location(0) pos : vec2f,
    @location(1) uv : vec2f,
    @location(2) color : vec4f,
};

struct VSOut {
    @builtin(position) position : vec4f,
    @location(0) uv : vec2f,
    @location(1) color : vec4f,
};

@vertex
fn vs_main(input : VSIn) -> VSOut {
    var out : VSOut;
    out.position = vec4f(input.pos, 0.0, 1.0);
    out.uv = input.uv;
    out.color = input.color;
    return out;
}

@fragment
fn fs_main(input : VSOut) -> @location(0) vec4f {
    let d = length(input.uv);
    let edge = smoothstep(1.0, 0.72, d);
    let core = smoothstep(0.46, 0.22, d);
    let ring = max(edge - core, 0.0);
    let alpha = ring * input.color.a;
    return vec4f(input.color.rgb * alpha, alpha);
}
)";

inline float Clamp01(float v) {
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    return v;
}

void AppendRippleQuad(
    float centerX,
    float centerY,
    float radius,
    float r,
    float g,
    float b,
    float a,
    int surfaceWidth,
    int surfaceHeight,
    std::vector<RippleVertexGpu>* out) {
    if (!out || surfaceWidth <= 0 || surfaceHeight <= 0 || radius <= 1.0f || a <= 0.001f) return;

    const float left = centerX - radius;
    const float right = centerX + radius;
    const float top = centerY - radius;
    const float bottom = centerY + radius;

    auto toNdcX = [surfaceWidth](float px) -> float {
        return (px / (float)surfaceWidth) * 2.0f - 1.0f;
    };
    auto toNdcY = [surfaceHeight](float py) -> float {
        return 1.0f - (py / (float)surfaceHeight) * 2.0f;
    };

    const RippleVertexGpu v0{toNdcX(left), toNdcY(top), -1.0f, -1.0f, r, g, b, a};
    const RippleVertexGpu v1{toNdcX(right), toNdcY(top), 1.0f, -1.0f, r, g, b, a};
    const RippleVertexGpu v2{toNdcX(left), toNdcY(bottom), -1.0f, 1.0f, r, g, b, a};
    const RippleVertexGpu v3{toNdcX(right), toNdcY(bottom), 1.0f, 1.0f, r, g, b, a};

    out->push_back(v0);
    out->push_back(v1);
    out->push_back(v2);
    out->push_back(v2);
    out->push_back(v1);
    out->push_back(v3);
}

uint32_t BuildRippleVertices(
    const OverlayGpuCommandStream* commandStream,
    int surfaceWidth,
    int surfaceHeight,
    std::vector<RippleVertexGpu>* outVertices) {
    if (!commandStream || !outVertices) return 0;
    outVertices->clear();
    outVertices->reserve(commandStream->Commands().size() * 6u);

    uint32_t pulses = 0;
    for (const auto& cmd : commandStream->Commands()) {
        if (cmd.type != OverlayGpuCommandType::RipplePulse || cmd.vertices.empty()) continue;
        const auto& center = cmd.vertices.front();

        const float t = Clamp01(center.extra);
        const float baseRadius = (center.size > 1.0f) ? center.size : 28.0f;
        const float animatedRadius = baseRadius * (0.50f + 1.10f * t);

        const uint32_t argb = center.colorArgb;
        const float a = ((float)((argb >> 24) & 0xFFu) / 255.0f) * (1.0f - t);
        const float r = (float)((argb >> 16) & 0xFFu) / 255.0f;
        const float g = (float)((argb >> 8) & 0xFFu) / 255.0f;
        const float b = (float)(argb & 0xFFu) / 255.0f;

        AppendRippleQuad(
            center.x,
            center.y,
            (std::max)(animatedRadius, 12.0f),
            r,
            g,
            b,
            Clamp01(a),
            surfaceWidth,
            surfaceHeight,
            outVertices);
        ++pulses;
    }
    return pulses;
}

void ReleaseBuffer(DawnRippleSurfaceRendererState* state) noexcept {
    if (!state || !state->vertexBuffer) return;
    const auto bufferRelease = reinterpret_cast<PFN_wgpuBufferRelease>(state->procBufferRelease);
    if (bufferRelease) bufferRelease(state->vertexBuffer);
    state->vertexBuffer = nullptr;
    state->vertexBufferBytes = 0;
}

bool EnsureRipplePipeline(
    DawnRippleSurfaceRendererState* state,
    const DawnRuntimePresentContext& context,
    std::string* detailOut) {
    if (!state) {
        if (detailOut) *detailOut = "ripple_state_invalid";
        return false;
    }
    const auto createShaderModule =
        reinterpret_cast<PFN_wgpuDeviceCreateShaderModule>(context.procDeviceCreateShaderModule);
    const auto shaderModuleRelease =
        reinterpret_cast<PFN_wgpuShaderModuleRelease>(context.procShaderModuleRelease);
    const auto createRenderPipeline =
        reinterpret_cast<PFN_wgpuDeviceCreateRenderPipeline>(context.procDeviceCreateRenderPipeline);
    const auto renderPipelineRelease =
        reinterpret_cast<PFN_wgpuRenderPipelineRelease>(context.procRenderPipelineRelease);
    if (!createShaderModule || !shaderModuleRelease || !createRenderPipeline || !renderPipelineRelease) {
        if (detailOut) *detailOut = "ripple_pipeline_api_missing";
        return false;
    }

    state->procShaderModuleRelease = context.procShaderModuleRelease;
    state->procRenderPipelineRelease = context.procRenderPipelineRelease;

    if (state->renderPipeline && state->shaderModule) {
        return true;
    }

    WGPUShaderSourceWGSLRaw wgsl{};
    wgsl.chain.sType = kWGPUSTypeShaderSourceWGSL;
    wgsl.code.data = kRippleShaderWgsl;
    wgsl.code.length = static_cast<size_t>(-1);

    WGPUShaderModuleDescriptorRaw shaderDesc{};
    shaderDesc.nextInChain = &wgsl.chain;
    shaderDesc.label.data = "mfx_ripple_shader";
    shaderDesc.label.length = static_cast<size_t>(-1);
    state->shaderModule = createShaderModule(context.device, &shaderDesc);
    if (!state->shaderModule) {
        if (detailOut) *detailOut = "ripple_shader_create_failed";
        return false;
    }

    WGPUVertexAttributeRaw attrs[3]{};
    attrs[0].format = kWGPUVertexFormatFloat32x2;
    attrs[0].offset = 0;
    attrs[0].shaderLocation = 0;
    attrs[1].format = kWGPUVertexFormatFloat32x2;
    attrs[1].offset = sizeof(float) * 2;
    attrs[1].shaderLocation = 1;
    attrs[2].format = kWGPUVertexFormatFloat32x4;
    attrs[2].offset = sizeof(float) * 4;
    attrs[2].shaderLocation = 2;

    WGPUVertexBufferLayoutRaw vboLayout{};
    vboLayout.stepMode = kWGPUVertexStepModeVertex;
    vboLayout.arrayStride = sizeof(RippleVertexGpu);
    vboLayout.attributeCount = 3;
    vboLayout.attributes = attrs;

    WGPUBlendComponentRaw blendComp{};
    blendComp.operation = kWGPUBlendOperationAdd;
    blendComp.srcFactor = kWGPUBlendFactorOne;
    blendComp.dstFactor = kWGPUBlendFactorOneMinusSrcAlpha;

    WGPUBlendStateRaw blendState{};
    blendState.color = blendComp;
    blendState.alpha = blendComp;

    WGPUColorTargetStateRaw target{};
    target.format = kWGPUTextureFormatBGRA8Unorm;
    target.blend = &blendState;
    target.writeMask = kWGPUColorWriteMaskAll;

    WGPUFragmentStateRaw fragment{};
    fragment.module = state->shaderModule;
    fragment.entryPoint.data = "fs_main";
    fragment.entryPoint.length = static_cast<size_t>(-1);
    fragment.targetCount = 1;
    fragment.targets = &target;

    WGPUVertexStateRaw vertex{};
    vertex.module = state->shaderModule;
    vertex.entryPoint.data = "vs_main";
    vertex.entryPoint.length = static_cast<size_t>(-1);
    vertex.bufferCount = 1;
    vertex.buffers = &vboLayout;

    WGPUPrimitiveStateRaw primitive{};
    primitive.topology = kWGPUPrimitiveTopologyTriangleList;

    WGPUMultisampleStateRaw multisample{};
    multisample.count = 1;
    multisample.mask = 0xFFFFFFFFu;
    multisample.alphaToCoverageEnabled = 0;

    WGPURenderPipelineDescriptorRaw pipeDesc{};
    pipeDesc.label.data = "mfx_ripple_pipeline";
    pipeDesc.label.length = static_cast<size_t>(-1);
    pipeDesc.layout = nullptr;
    pipeDesc.vertex = vertex;
    pipeDesc.primitive = primitive;
    pipeDesc.depthStencil = nullptr;
    pipeDesc.multisample = multisample;
    pipeDesc.fragment = &fragment;

    state->renderPipeline = createRenderPipeline(context.device, &pipeDesc);
    if (!state->renderPipeline) {
        shaderModuleRelease(state->shaderModule);
        state->shaderModule = nullptr;
        if (detailOut) *detailOut = "ripple_pipeline_create_failed";
        return false;
    }
    return true;
}

bool EnsureVertexBuffer(
    DawnRippleSurfaceRendererState* state,
    const DawnRuntimePresentContext& context,
    uint64_t requiredBytes,
    std::string* detailOut) {
    if (!state) {
        if (detailOut) *detailOut = "ripple_buffer_state_invalid";
        return false;
    }
    const auto createBuffer = reinterpret_cast<PFN_wgpuDeviceCreateBuffer>(context.procDeviceCreateBuffer);
    const auto bufferRelease = reinterpret_cast<PFN_wgpuBufferRelease>(context.procBufferRelease);
    if (!createBuffer || !bufferRelease) {
        if (detailOut) *detailOut = "ripple_buffer_api_missing";
        return false;
    }
    state->procBufferRelease = context.procBufferRelease;

    if (state->vertexBuffer && state->vertexBufferBytes >= requiredBytes) {
        return true;
    }

    ReleaseBuffer(state);
    const uint64_t allocBytes = (std::max)(requiredBytes, 4096ull);
    WGPUBufferDescriptorRaw desc{};
    desc.label.data = "mfx_ripple_vertex_buffer";
    desc.label.length = static_cast<size_t>(-1);
    desc.usage = kWGPUBufferUsageVertex | kWGPUBufferUsageCopyDst;
    desc.size = allocBytes;
    desc.mappedAtCreation = 0;

    state->vertexBuffer = createBuffer(context.device, &desc);
    if (!state->vertexBuffer) {
        if (detailOut) *detailOut = "ripple_vertex_buffer_create_failed";
        return false;
    }
    state->vertexBufferBytes = allocBytes;
    return true;
}

} // namespace

void ReleaseDawnRippleSurfaceRendererState(DawnRippleSurfaceRendererState* state) noexcept {
    if (!state) return;

    ReleaseBuffer(state);

    if (state->renderPipeline) {
        const auto releasePipeline = reinterpret_cast<PFN_wgpuRenderPipelineRelease>(state->procRenderPipelineRelease);
        if (releasePipeline) releasePipeline(state->renderPipeline);
        state->renderPipeline = nullptr;
    }
    if (state->shaderModule) {
        const auto releaseShader = reinterpret_cast<PFN_wgpuShaderModuleRelease>(state->procShaderModuleRelease);
        if (releaseShader) releaseShader(state->shaderModule);
        state->shaderModule = nullptr;
    }

    state->vertexBufferBytes = 0;
    state->procShaderModuleRelease = nullptr;
    state->procRenderPipelineRelease = nullptr;
    state->procBufferRelease = nullptr;
}

bool EncodeDawnRippleSurfacePass(
    DawnRippleSurfaceRendererState* state,
    const DawnRuntimePresentContext& context,
    const OverlayGpuCommandStream* commandStream,
    int surfaceWidth,
    int surfaceHeight,
    void* renderPassEncoder,
    uint32_t* drawnPulseCountOut,
    std::string* detailOut) {
    if (drawnPulseCountOut) *drawnPulseCountOut = 0;
    if (!state || !renderPassEncoder || !context.device || !context.queue) {
        if (detailOut) *detailOut = "ripple_draw_context_invalid";
        return false;
    }

    const auto queueWriteBuffer = reinterpret_cast<PFN_wgpuQueueWriteBuffer>(context.procQueueWriteBuffer);
    const auto passSetPipeline = reinterpret_cast<PFN_wgpuRenderPassEncoderSetPipeline>(context.procRenderPassEncoderSetPipeline);
    const auto passSetVertexBuffer =
        reinterpret_cast<PFN_wgpuRenderPassEncoderSetVertexBuffer>(context.procRenderPassEncoderSetVertexBuffer);
    const auto passDraw = reinterpret_cast<PFN_wgpuRenderPassEncoderDraw>(context.procRenderPassEncoderDraw);
    if (!queueWriteBuffer || !passSetPipeline || !passSetVertexBuffer || !passDraw) {
        if (detailOut) *detailOut = "ripple_draw_api_missing";
        return false;
    }

    std::vector<RippleVertexGpu> vertices{};
    const uint32_t pulses = BuildRippleVertices(commandStream, surfaceWidth, surfaceHeight, &vertices);
    if (pulses == 0 || vertices.empty()) {
        if (detailOut) *detailOut = "ripple_draw_noop";
        return true;
    }

    if (!EnsureRipplePipeline(state, context, detailOut)) {
        return false;
    }

    const uint64_t requiredBytes = (uint64_t)vertices.size() * sizeof(RippleVertexGpu);
    if (!EnsureVertexBuffer(state, context, requiredBytes, detailOut)) {
        return false;
    }

    queueWriteBuffer(context.queue, state->vertexBuffer, 0, vertices.data(), (size_t)requiredBytes);
    passSetPipeline(renderPassEncoder, state->renderPipeline);
    passSetVertexBuffer(renderPassEncoder, 0, state->vertexBuffer, 0, requiredBytes);
    passDraw(renderPassEncoder, (uint32_t)vertices.size(), 1, 0, 0);

    if (drawnPulseCountOut) *drawnPulseCountOut = pulses;
    if (detailOut) {
        *detailOut = "ripple_draw_ok_p" + std::to_string(pulses) + "_v" + std::to_string(vertices.size());
    }
    return true;
}

} // namespace mousefx::gpu

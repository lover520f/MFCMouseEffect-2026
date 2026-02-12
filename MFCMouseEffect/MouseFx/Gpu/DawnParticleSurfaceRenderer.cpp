#include "pch.h"

#include "DawnParticleSurfaceRenderer.h"

#include <algorithm>
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

struct ParticleVertexGpu {
    float posX = 0.0f;
    float posY = 0.0f;
    float uvX = 0.0f;
    float uvY = 0.0f;
    float colorR = 0.0f;
    float colorG = 0.0f;
    float colorB = 0.0f;
    float colorA = 0.0f;
};

constexpr const char* kParticleShaderWgsl = R"(
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
    let alpha = smoothstep(1.0, 0.0, d) * input.color.a;
    return vec4f(input.color.rgb * alpha, alpha);
}
)";

void DecodeArgb(uint32_t argb, float* r, float* g, float* b, float* a) {
    if (!r || !g || !b || !a) return;
    *a = (float)((argb >> 24) & 0xFFu) / 255.0f;
    *r = (float)((argb >> 16) & 0xFFu) / 255.0f;
    *g = (float)((argb >> 8) & 0xFFu) / 255.0f;
    *b = (float)(argb & 0xFFu) / 255.0f;
}

void PushParticleVertex(
    float x,
    float y,
    float uvX,
    float uvY,
    uint32_t argb,
    int surfaceWidth,
    int surfaceHeight,
    std::vector<ParticleVertexGpu>* out) {
    if (!out || surfaceWidth <= 0 || surfaceHeight <= 0) return;
    float r = 0.0f;
    float g = 0.0f;
    float b = 0.0f;
    float a = 0.0f;
    DecodeArgb(argb, &r, &g, &b, &a);
    ParticleVertexGpu v{};
    v.posX = (x / (float)surfaceWidth) * 2.0f - 1.0f;
    v.posY = 1.0f - (y / (float)surfaceHeight) * 2.0f;
    v.uvX = uvX;
    v.uvY = uvY;
    v.colorR = r;
    v.colorG = g;
    v.colorB = b;
    v.colorA = a;
    out->push_back(v);
}

uint32_t BuildParticleVertices(
    const OverlayGpuCommandStream* commandStream,
    int surfaceWidth,
    int surfaceHeight,
    std::vector<ParticleVertexGpu>* outVertices) {
    if (!commandStream || !outVertices) return 0;
    outVertices->clear();
    uint32_t sprites = 0;
    for (const auto& cmd : commandStream->Commands()) {
        if (cmd.type != OverlayGpuCommandType::ParticleSprites || cmd.vertices.empty()) continue;
        for (const auto& v : cmd.vertices) {
            const float size = (v.size > 1.0f) ? v.size : 2.0f;
            const float half = size * 0.5f;
            const float l = v.x - half;
            const float r = v.x + half;
            const float t = v.y - half;
            const float b = v.y + half;
            PushParticleVertex(l, t, -1.0f, -1.0f, v.colorArgb, surfaceWidth, surfaceHeight, outVertices);
            PushParticleVertex(r, t, 1.0f, -1.0f, v.colorArgb, surfaceWidth, surfaceHeight, outVertices);
            PushParticleVertex(l, b, -1.0f, 1.0f, v.colorArgb, surfaceWidth, surfaceHeight, outVertices);
            PushParticleVertex(l, b, -1.0f, 1.0f, v.colorArgb, surfaceWidth, surfaceHeight, outVertices);
            PushParticleVertex(r, t, 1.0f, -1.0f, v.colorArgb, surfaceWidth, surfaceHeight, outVertices);
            PushParticleVertex(r, b, 1.0f, 1.0f, v.colorArgb, surfaceWidth, surfaceHeight, outVertices);
            ++sprites;
        }
    }
    return sprites;
}

void ReleaseBuffer(DawnParticleSurfaceRendererState* state) noexcept {
    if (!state || !state->vertexBuffer) return;
    const auto release = reinterpret_cast<PFN_wgpuBufferRelease>(state->procBufferRelease);
    if (release) release(state->vertexBuffer);
    state->vertexBuffer = nullptr;
    state->vertexBufferBytes = 0;
}

bool EnsurePipeline(DawnParticleSurfaceRendererState* state, const DawnRuntimePresentContext& context, std::string* detailOut) {
    const auto createShader = reinterpret_cast<PFN_wgpuDeviceCreateShaderModule>(context.procDeviceCreateShaderModule);
    const auto releaseShader = reinterpret_cast<PFN_wgpuShaderModuleRelease>(context.procShaderModuleRelease);
    const auto createPipeline = reinterpret_cast<PFN_wgpuDeviceCreateRenderPipeline>(context.procDeviceCreateRenderPipeline);
    const auto releasePipeline = reinterpret_cast<PFN_wgpuRenderPipelineRelease>(context.procRenderPipelineRelease);
    if (!state || !createShader || !releaseShader || !createPipeline || !releasePipeline) {
        if (detailOut) *detailOut = "particle_pipeline_api_missing";
        return false;
    }
    state->procShaderModuleRelease = context.procShaderModuleRelease;
    state->procRenderPipelineRelease = context.procRenderPipelineRelease;
    if (state->shaderModule && state->renderPipeline) return true;

    WGPUShaderSourceWGSLRaw wgsl{};
    wgsl.chain.sType = kWGPUSTypeShaderSourceWGSL;
    wgsl.code.data = kParticleShaderWgsl;
    wgsl.code.length = static_cast<size_t>(-1);
    WGPUShaderModuleDescriptorRaw shaderDesc{};
    shaderDesc.nextInChain = &wgsl.chain;
    shaderDesc.label.data = "mfx_particle_shader";
    shaderDesc.label.length = static_cast<size_t>(-1);
    state->shaderModule = createShader(context.device, &shaderDesc);
    if (!state->shaderModule) {
        if (detailOut) *detailOut = "particle_shader_create_failed";
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

    WGPUVertexBufferLayoutRaw vbo{};
    vbo.stepMode = kWGPUVertexStepModeVertex;
    vbo.arrayStride = sizeof(ParticleVertexGpu);
    vbo.attributeCount = 3;
    vbo.attributes = attrs;

    WGPUBlendComponentRaw comp{};
    comp.operation = kWGPUBlendOperationAdd;
    comp.srcFactor = kWGPUBlendFactorOne;
    comp.dstFactor = kWGPUBlendFactorOneMinusSrcAlpha;
    WGPUBlendStateRaw blend{};
    blend.color = comp;
    blend.alpha = comp;
    WGPUColorTargetStateRaw target{};
    target.format = kWGPUTextureFormatBGRA8Unorm;
    target.blend = &blend;
    target.writeMask = kWGPUColorWriteMaskAll;

    WGPUVertexStateRaw vertex{};
    vertex.module = state->shaderModule;
    vertex.entryPoint.data = "vs_main";
    vertex.entryPoint.length = static_cast<size_t>(-1);
    vertex.bufferCount = 1;
    vertex.buffers = &vbo;

    WGPUFragmentStateRaw fragment{};
    fragment.module = state->shaderModule;
    fragment.entryPoint.data = "fs_main";
    fragment.entryPoint.length = static_cast<size_t>(-1);
    fragment.targetCount = 1;
    fragment.targets = &target;

    WGPUPrimitiveStateRaw primitive{};
    primitive.topology = kWGPUPrimitiveTopologyTriangleList;
    WGPUMultisampleStateRaw msaa{};
    msaa.count = 1;
    msaa.mask = 0xFFFFFFFFu;

    WGPURenderPipelineDescriptorRaw desc{};
    desc.label.data = "mfx_particle_pipeline";
    desc.label.length = static_cast<size_t>(-1);
    desc.vertex = vertex;
    desc.primitive = primitive;
    desc.multisample = msaa;
    desc.fragment = &fragment;
    state->renderPipeline = createPipeline(context.device, &desc);
    if (!state->renderPipeline) {
        releaseShader(state->shaderModule);
        state->shaderModule = nullptr;
        if (detailOut) *detailOut = "particle_pipeline_create_failed";
        return false;
    }
    return true;
}

bool EnsureBuffer(DawnParticleSurfaceRendererState* state, const DawnRuntimePresentContext& context, uint64_t bytes, std::string* detailOut) {
    const auto createBuffer = reinterpret_cast<PFN_wgpuDeviceCreateBuffer>(context.procDeviceCreateBuffer);
    const auto release = reinterpret_cast<PFN_wgpuBufferRelease>(context.procBufferRelease);
    if (!state || !createBuffer || !release) {
        if (detailOut) *detailOut = "particle_buffer_api_missing";
        return false;
    }
    state->procBufferRelease = context.procBufferRelease;
    if (state->vertexBuffer && state->vertexBufferBytes >= bytes) return true;
    ReleaseBuffer(state);
    WGPUBufferDescriptorRaw bd{};
    bd.label.data = "mfx_particle_vertex_buffer";
    bd.label.length = static_cast<size_t>(-1);
    bd.usage = kWGPUBufferUsageVertex | kWGPUBufferUsageCopyDst;
    bd.size = (std::max)(bytes, 4096ull);
    bd.mappedAtCreation = 0;
    state->vertexBuffer = createBuffer(context.device, &bd);
    if (!state->vertexBuffer) {
        if (detailOut) *detailOut = "particle_vertex_buffer_create_failed";
        return false;
    }
    state->vertexBufferBytes = bd.size;
    return true;
}

} // namespace

void ReleaseDawnParticleSurfaceRendererState(DawnParticleSurfaceRendererState* state) noexcept {
    if (!state) return;
    ReleaseBuffer(state);
    if (state->renderPipeline) {
        const auto release = reinterpret_cast<PFN_wgpuRenderPipelineRelease>(state->procRenderPipelineRelease);
        if (release) release(state->renderPipeline);
        state->renderPipeline = nullptr;
    }
    if (state->shaderModule) {
        const auto release = reinterpret_cast<PFN_wgpuShaderModuleRelease>(state->procShaderModuleRelease);
        if (release) release(state->shaderModule);
        state->shaderModule = nullptr;
    }
    state->procShaderModuleRelease = nullptr;
    state->procRenderPipelineRelease = nullptr;
    state->procBufferRelease = nullptr;
    state->vertexBufferBytes = 0;
}

bool EncodeDawnParticleSurfacePass(
    DawnParticleSurfaceRendererState* state,
    const DawnRuntimePresentContext& context,
    const OverlayGpuCommandStream* commandStream,
    int surfaceWidth,
    int surfaceHeight,
    void* renderPassEncoder,
    uint32_t* drawnParticleSpritesOut,
    std::string* detailOut) {
    if (drawnParticleSpritesOut) *drawnParticleSpritesOut = 0;
    if (!state || !commandStream || !renderPassEncoder || !context.device || !context.queue) {
        if (detailOut) *detailOut = "particle_draw_context_invalid";
        return false;
    }

    std::vector<ParticleVertexGpu> vertices{};
    const uint32_t sprites = BuildParticleVertices(commandStream, surfaceWidth, surfaceHeight, &vertices);
    if (sprites == 0 || vertices.empty()) {
        if (detailOut) *detailOut = "particle_draw_noop";
        return true;
    }

    const auto writeBuffer = reinterpret_cast<PFN_wgpuQueueWriteBuffer>(context.procQueueWriteBuffer);
    const auto setPipeline = reinterpret_cast<PFN_wgpuRenderPassEncoderSetPipeline>(context.procRenderPassEncoderSetPipeline);
    const auto setVertexBuffer =
        reinterpret_cast<PFN_wgpuRenderPassEncoderSetVertexBuffer>(context.procRenderPassEncoderSetVertexBuffer);
    const auto draw = reinterpret_cast<PFN_wgpuRenderPassEncoderDraw>(context.procRenderPassEncoderDraw);
    if (!writeBuffer || !setPipeline || !setVertexBuffer || !draw) {
        if (detailOut) *detailOut = "particle_draw_api_missing";
        return false;
    }

    if (!EnsurePipeline(state, context, detailOut)) return false;
    const uint64_t bytes = (uint64_t)vertices.size() * sizeof(ParticleVertexGpu);
    if (!EnsureBuffer(state, context, bytes, detailOut)) return false;

    writeBuffer(context.queue, state->vertexBuffer, 0, vertices.data(), (size_t)bytes);
    setPipeline(renderPassEncoder, state->renderPipeline);
    setVertexBuffer(renderPassEncoder, 0, state->vertexBuffer, 0, bytes);
    draw(renderPassEncoder, (uint32_t)vertices.size(), 1, 0, 0);

    if (drawnParticleSpritesOut) *drawnParticleSpritesOut = sprites;
    if (detailOut) {
        *detailOut = "particle_draw_ok_s" + std::to_string(sprites) + "_v" + std::to_string(vertices.size());
    }
    return true;
}

} // namespace mousefx::gpu

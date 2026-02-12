#include "pch.h"

#include "DawnSurfaceInterop.h"

#include "DawnSurfaceCommandStats.h"
#include "DawnRuntime.h"

namespace mousefx::gpu {
namespace {

struct WGPUChainedStructRaw {
    WGPUChainedStructRaw* next = nullptr;
    uint32_t sType = 0;
};

struct WGPUStringViewRaw {
    const char* data = nullptr;
    size_t length = static_cast<size_t>(-1);
};

struct WGPUColorRaw {
    double r = 0.0;
    double g = 0.0;
    double b = 0.0;
    double a = 0.0;
};

struct WGPUSurfaceSourceWindowsHWNDRaw {
    WGPUChainedStructRaw chain{};
    void* hinstance = nullptr;
    void* hwnd = nullptr;
};

struct WGPUSurfaceDescriptorRaw {
    WGPUChainedStructRaw* nextInChain = nullptr;
    WGPUStringViewRaw label{};
};

struct WGPUSurfaceConfigurationRaw {
    WGPUChainedStructRaw* nextInChain = nullptr;
    void* device = nullptr;
    uint32_t format = 0;
    uint64_t usage = 0;
    uint32_t width = 0;
    uint32_t height = 0;
    size_t viewFormatCount = 0;
    const uint32_t* viewFormats = nullptr;
    uint32_t alphaMode = 0;
    uint32_t presentMode = 0;
};

struct WGPUSurfaceTextureRaw {
    WGPUChainedStructRaw* nextInChain = nullptr;
    void* texture = nullptr;
    uint32_t status = 0;
};

struct WGPURenderPassColorAttachmentRaw {
    WGPUChainedStructRaw* nextInChain = nullptr;
    void* view = nullptr;
    uint32_t depthSlice = 0xFFFFFFFFu;
    void* resolveTarget = nullptr;
    uint32_t loadOp = 0;
    uint32_t storeOp = 0;
    WGPUColorRaw clearValue{};
};

struct WGPURenderPassDescriptorRaw {
    WGPUChainedStructRaw* nextInChain = nullptr;
    WGPUStringViewRaw label{};
    size_t colorAttachmentCount = 0;
    const WGPURenderPassColorAttachmentRaw* colorAttachments = nullptr;
    const void* depthStencilAttachment = nullptr;
    void* occlusionQuerySet = nullptr;
    const void* timestampWrites = nullptr;
};

using PFN_wgpuInstanceCreateSurface = void* (*)(void*, const WGPUSurfaceDescriptorRaw*);
using PFN_wgpuSurfaceConfigure = void (*)(void*, const WGPUSurfaceConfigurationRaw*);
using PFN_wgpuSurfaceGetCurrentTexture = void (*)(void*, WGPUSurfaceTextureRaw*);
using PFN_wgpuSurfacePresent = uint32_t (*)(void*);
using PFN_wgpuSurfaceUnconfigure = void (*)(void*);
using PFN_wgpuSurfaceRelease = void (*)(void*);
using PFN_wgpuTextureRelease = void (*)(void*);
using PFN_wgpuTextureCreateView = void* (*)(void*, const void*);
using PFN_wgpuTextureViewRelease = void (*)(void*);
using PFN_wgpuDeviceCreateCommandEncoder = void* (*)(void*, const void*);
using PFN_wgpuCommandEncoderBeginRenderPass = void* (*)(void*, const WGPURenderPassDescriptorRaw*);
using PFN_wgpuRenderPassEncoderEnd = void (*)(void*);
using PFN_wgpuRenderPassEncoderRelease = void (*)(void*);
using PFN_wgpuCommandEncoderFinish = void* (*)(void*, const void*);
using PFN_wgpuCommandEncoderRelease = void (*)(void*);
using PFN_wgpuCommandBufferRelease = void (*)(void*);
using PFN_wgpuQueueSubmit = void (*)(void*, size_t, const void*);

constexpr uint32_t kWGPUSTypeSurfaceSourceWindowsHWND = 0x00000005;
constexpr uint32_t kWGPUTextureFormatBGRA8Unorm = 0x0000001B;
constexpr uint64_t kWGPUTextureUsageRenderAttachment = 0x0000000000000010ULL;
constexpr uint32_t kWGPUCompositeAlphaModeAuto = 0x00000000;
constexpr uint32_t kWGPUPresentModeFifo = 0x00000001;
constexpr uint32_t kWGPUSurfaceGetCurrentTextureStatusSuccessOptimal = 0x00000001;
constexpr uint32_t kWGPUSurfaceGetCurrentTextureStatusSuccessSuboptimal = 0x00000002;
constexpr uint32_t kWGPUStatusSuccess = 0x00000001;
constexpr uint32_t kWGPULoadOpClear = 0x00000002;
constexpr uint32_t kWGPUStoreOpStore = 0x00000001;

} // namespace

void ReleaseDawnSurfaceInteropState(DawnSurfaceInteropState* state) noexcept {
    if (!state) return;
    ReleaseDawnTrailSurfaceRendererState(&state->trailRenderer);
    ReleaseDawnParticleSurfaceRendererState(&state->particleRenderer);
    ReleaseDawnRippleSurfaceRendererState(&state->rippleRenderer);
    if (!state->surface) return;
    const auto unconfigure = reinterpret_cast<PFN_wgpuSurfaceUnconfigure>(state->procSurfaceUnconfigure);
    const auto release = reinterpret_cast<PFN_wgpuSurfaceRelease>(state->procSurfaceRelease);
    if (unconfigure) unconfigure(state->surface);
    if (release) release(state->surface);
    state->surface = nullptr;
    state->hwnd = nullptr;
    state->width = 0;
    state->height = 0;
}

bool PresentDawnSurfaceClearPass(
    DawnSurfaceInteropState* state,
    const DawnRuntimePresentContext& context,
    void* hwnd,
    int width,
    int height,
    const OverlayGpuCommandStream* commandStream,
    bool* hadGpuVisualContentOut,
    std::string* detailOut) {
    if (hadGpuVisualContentOut) *hadGpuVisualContentOut = false;
    if (!state || !hwnd || width <= 0 || height <= 0) {
        if (detailOut) *detailOut = "gpu_present_frame_invalid";
        return false;
    }
    if (!context.ready) {
        if (detailOut) *detailOut = "gpu_present_runtime_context_not_ready";
        return false;
    }

    const auto createSurface = reinterpret_cast<PFN_wgpuInstanceCreateSurface>(context.procInstanceCreateSurface);
    const auto surfaceConfigure = reinterpret_cast<PFN_wgpuSurfaceConfigure>(context.procSurfaceConfigure);
    const auto surfaceGetCurrentTexture = reinterpret_cast<PFN_wgpuSurfaceGetCurrentTexture>(context.procSurfaceGetCurrentTexture);
    const auto surfacePresent = reinterpret_cast<PFN_wgpuSurfacePresent>(context.procSurfacePresent);
    const auto surfaceUnconfigure = reinterpret_cast<PFN_wgpuSurfaceUnconfigure>(context.procSurfaceUnconfigure);
    const auto surfaceRelease = reinterpret_cast<PFN_wgpuSurfaceRelease>(context.procSurfaceRelease);
    const auto textureRelease = reinterpret_cast<PFN_wgpuTextureRelease>(context.procTextureRelease);
    const auto textureCreateView = reinterpret_cast<PFN_wgpuTextureCreateView>(context.procTextureCreateView);
    const auto textureViewRelease = reinterpret_cast<PFN_wgpuTextureViewRelease>(context.procTextureViewRelease);
    const auto deviceCreateCommandEncoder = reinterpret_cast<PFN_wgpuDeviceCreateCommandEncoder>(context.procDeviceCreateCommandEncoder);
    const auto commandEncoderBeginRenderPass = reinterpret_cast<PFN_wgpuCommandEncoderBeginRenderPass>(context.procCommandEncoderBeginRenderPass);
    const auto renderPassEncoderEnd = reinterpret_cast<PFN_wgpuRenderPassEncoderEnd>(context.procRenderPassEncoderEnd);
    const auto renderPassEncoderRelease = reinterpret_cast<PFN_wgpuRenderPassEncoderRelease>(context.procRenderPassEncoderRelease);
    const auto commandEncoderFinish = reinterpret_cast<PFN_wgpuCommandEncoderFinish>(context.procCommandEncoderFinish);
    const auto commandEncoderRelease = reinterpret_cast<PFN_wgpuCommandEncoderRelease>(context.procCommandEncoderRelease);
    const auto commandBufferRelease = reinterpret_cast<PFN_wgpuCommandBufferRelease>(context.procCommandBufferRelease);
    const auto queueSubmit = reinterpret_cast<PFN_wgpuQueueSubmit>(context.procQueueSubmit);
    if (!createSurface || !surfaceConfigure || !surfaceGetCurrentTexture || !surfacePresent || !surfaceRelease ||
        !textureRelease || !textureCreateView || !textureViewRelease || !deviceCreateCommandEncoder ||
        !commandEncoderBeginRenderPass || !renderPassEncoderEnd || !commandEncoderFinish || !queueSubmit) {
        if (detailOut) *detailOut = "gpu_present_runtime_proc_missing";
        return false;
    }

    state->procSurfaceUnconfigure = reinterpret_cast<void*>(surfaceUnconfigure);
    state->procSurfaceRelease = reinterpret_cast<void*>(surfaceRelease);
    if (state->surface && state->hwnd != hwnd) {
        ReleaseDawnSurfaceInteropState(state);
    }

    if (!state->surface) {
        WGPUSurfaceSourceWindowsHWNDRaw source{};
        source.chain.sType = kWGPUSTypeSurfaceSourceWindowsHWND;
        source.hinstance = reinterpret_cast<void*>(GetModuleHandleW(nullptr));
        source.hwnd = hwnd;

        WGPUSurfaceDescriptorRaw desc{};
        desc.nextInChain = &source.chain;
        desc.label.data = "mfx_overlay_surface";
        desc.label.length = static_cast<size_t>(-1);

        state->surface = createSurface(context.instance, &desc);
        if (!state->surface) {
            if (detailOut) *detailOut = "gpu_present_surface_create_failed";
            return false;
        }
        state->hwnd = hwnd;
        state->width = 0;
        state->height = 0;
    }

    if (state->width != width || state->height != height) {
        WGPUSurfaceConfigurationRaw cfg{};
        cfg.device = context.device;
        cfg.format = kWGPUTextureFormatBGRA8Unorm;
        cfg.usage = kWGPUTextureUsageRenderAttachment;
        cfg.width = static_cast<uint32_t>(width);
        cfg.height = static_cast<uint32_t>(height);
        cfg.alphaMode = kWGPUCompositeAlphaModeAuto;
        cfg.presentMode = kWGPUPresentModeFifo;
        surfaceConfigure(state->surface, &cfg);
        state->width = width;
        state->height = height;
    }

    WGPUSurfaceTextureRaw currentTexture{};
    surfaceGetCurrentTexture(state->surface, &currentTexture);
    if (!currentTexture.texture ||
        (currentTexture.status != kWGPUSurfaceGetCurrentTextureStatusSuccessOptimal &&
         currentTexture.status != kWGPUSurfaceGetCurrentTextureStatusSuccessSuboptimal)) {
        if (currentTexture.status >= 3) {
            state->width = 0;
            state->height = 0;
        }
        if (detailOut) *detailOut = "gpu_present_surface_acquire_failed";
        return false;
    }

    void* textureView = textureCreateView(currentTexture.texture, nullptr);
    if (!textureView) {
        textureRelease(currentTexture.texture);
        if (detailOut) *detailOut = "gpu_present_texture_view_failed";
        return false;
    }

    void* encoder = deviceCreateCommandEncoder(context.device, nullptr);
    if (!encoder) {
        textureViewRelease(textureView);
        textureRelease(currentTexture.texture);
        if (detailOut) *detailOut = "gpu_present_encoder_create_failed";
        return false;
    }

    WGPURenderPassColorAttachmentRaw colorAttachment{};
    colorAttachment.view = textureView;
    colorAttachment.loadOp = kWGPULoadOpClear;
    colorAttachment.storeOp = kWGPUStoreOpStore;
    colorAttachment.clearValue = WGPUColorRaw{0.0, 0.0, 0.0, 0.0};

    WGPURenderPassDescriptorRaw passDesc{};
    passDesc.colorAttachmentCount = 1;
    passDesc.colorAttachments = &colorAttachment;
    void* pass = commandEncoderBeginRenderPass(encoder, &passDesc);
    if (!pass) {
        if (commandEncoderRelease) commandEncoderRelease(encoder);
        textureViewRelease(textureView);
        textureRelease(currentTexture.texture);
        if (detailOut) *detailOut = "gpu_present_begin_render_pass_failed";
        return false;
    }

    std::string trailDetail;
    uint32_t drawnTrailTriangles = 0;
    if (!EncodeDawnTrailSurfacePass(
            &state->trailRenderer,
            context,
            commandStream,
            width,
            height,
            pass,
            &drawnTrailTriangles,
            &trailDetail)) {
        if (renderPassEncoderRelease) renderPassEncoderRelease(pass);
        if (commandEncoderRelease) commandEncoderRelease(encoder);
        textureViewRelease(textureView);
        textureRelease(currentTexture.texture);
        if (detailOut) {
            *detailOut = trailDetail.empty()
                ? "gpu_present_trail_draw_failed"
                : "gpu_present_trail_draw_failed_" + trailDetail;
        }
        return false;
    }

    std::string particleDetail;
    uint32_t drawnParticleSprites = 0;
    if (!EncodeDawnParticleSurfacePass(
            &state->particleRenderer,
            context,
            commandStream,
            width,
            height,
            pass,
            &drawnParticleSprites,
            &particleDetail)) {
        if (renderPassEncoderRelease) renderPassEncoderRelease(pass);
        if (commandEncoderRelease) commandEncoderRelease(encoder);
        textureViewRelease(textureView);
        textureRelease(currentTexture.texture);
        if (detailOut) {
            *detailOut = particleDetail.empty()
                ? "gpu_present_particle_draw_failed"
                : "gpu_present_particle_draw_failed_" + particleDetail;
        }
        return false;
    }

    std::string rippleDetail;
    uint32_t drawnRipplePulses = 0;
    if (!EncodeDawnRippleSurfacePass(
            &state->rippleRenderer,
            context,
            commandStream,
            width,
            height,
            pass,
            &drawnRipplePulses,
            &rippleDetail)) {
        if (renderPassEncoderRelease) renderPassEncoderRelease(pass);
        if (commandEncoderRelease) commandEncoderRelease(encoder);
        textureViewRelease(textureView);
        textureRelease(currentTexture.texture);
        if (detailOut) {
            *detailOut = rippleDetail.empty()
                ? "gpu_present_ripple_draw_failed"
                : "gpu_present_ripple_draw_failed_" + rippleDetail;
        }
        return false;
    }
    if (hadGpuVisualContentOut) *hadGpuVisualContentOut =
        (drawnTrailTriangles > 0 || drawnParticleSprites > 0 || drawnRipplePulses > 0);
    renderPassEncoderEnd(pass);
    if (renderPassEncoderRelease) renderPassEncoderRelease(pass);

    void* commandBuffer = commandEncoderFinish(encoder, nullptr);
    if (commandEncoderRelease) commandEncoderRelease(encoder);
    if (!commandBuffer) {
        textureViewRelease(textureView);
        textureRelease(currentTexture.texture);
        if (detailOut) *detailOut = "gpu_present_encoder_finish_failed";
        return false;
    }

    void* submitList[1] = {commandBuffer};
    queueSubmit(context.queue, 1, submitList);
    if (commandBufferRelease) commandBufferRelease(commandBuffer);
    textureViewRelease(textureView);
    textureRelease(currentTexture.texture);

    const uint32_t presentStatus = surfacePresent(state->surface);
    if (presentStatus != kWGPUStatusSuccess) {
        if (detailOut) *detailOut = "gpu_present_surface_present_failed";
        return false;
    }

    if (detailOut) {
        const DawnSurfaceCommandStats stats = BuildDawnSurfaceCommandStats(commandStream);
        if (drawnTrailTriangles > 0 || drawnParticleSprites > 0 || drawnRipplePulses > 0) {
            *detailOut = "gpu_present_surface_submit_ok";
            if (drawnTrailTriangles > 0) {
                *detailOut += "_trail_t" + std::to_string(drawnTrailTriangles);
            }
            if (drawnParticleSprites > 0) {
                *detailOut += "_particle_s" + std::to_string(drawnParticleSprites);
            }
            if (drawnRipplePulses > 0) {
                *detailOut += "_ripple_p" + std::to_string(drawnRipplePulses);
            }
        } else {
            *detailOut = "gpu_present_surface_clearpass_submit_ok";
        }
        if (!trailDetail.empty()) {
            *detailOut += "_" + trailDetail;
        }
        if (!particleDetail.empty()) {
            *detailOut += "_" + particleDetail;
        }
        if (!rippleDetail.empty()) {
            *detailOut += "_" + rippleDetail;
        }
        *detailOut += BuildDawnSurfaceCommandStatsSuffix(stats);
    }
    return true;
}

} // namespace mousefx::gpu

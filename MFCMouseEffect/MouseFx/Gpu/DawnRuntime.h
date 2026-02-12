#pragma once

#include <cstdint>
#include <string>

namespace mousefx::gpu {

struct DawnRuntimeInitResult {
    bool ok = false;
    std::string backend = "cpu";
    std::string detail = "dawn_not_available";
};

struct DawnRuntimeProbeInfo {
    uint64_t generation = 0;
    bool compiled = false;
    bool hasDisplayAdapter = false;
    bool moduleLoaded = false;
    bool hasCoreProc = false;
    bool hasCreateInstance = false;
    bool hasRequestAdapter = false;
    bool hasRequestDevice = false;
    bool hasCreateSurface = false;
    bool hasGetQueue = false;
    bool hasSurfacePresent = false;
    bool hasWaitAny = false;
    bool hasModernRequestAdapter = false;
    bool hasModernRequestDevice = false;
    bool hasInstanceProcessEvents = false;
    bool canCreateInstance = false;
    bool canRequestAdapter = false;
    bool canCreateDevice = false;
    std::string moduleName = "";
    std::string detail = "probe_not_run";
};

struct DawnRuntimeStatus {
    DawnRuntimeProbeInfo probe{};
    std::string lastInitDetail = "init_not_run";
    uint64_t initAttempts = 0;
    uint64_t lastInitTickMs = 0;
    bool readyForDeviceStage = false;
    bool queueReady = false;
    bool commandEncoderReady = false;
    bool modernAbiDetected = false;
    bool modernAbiNativeReady = false;
    std::string modernAbiStrategy = "none";
    std::string modernAbiNativeDetail = "unknown";
    std::string modernAbiPrimeDetail = "not_attempted";
};

struct DawnRuntimeSymbolStatus {
    bool moduleLoaded = false;
    std::string moduleName = "";
    bool hasWgpuGetProcAddress = false;
    bool hasCreateInstance = false;
    bool hasRequestAdapterLegacy = false;
    bool hasRequestDeviceLegacy = false;
    bool hasRequestAdapterModern = false;
    bool hasRequestDeviceModern = false;
    bool hasWaitAny = false;
    bool hasInstanceProcessEvents = false;
    std::string summary = "unknown";
};

struct DawnRuntimePresentContext {
    bool ready = false;
    void* module = nullptr;
    void* instance = nullptr;
    void* device = nullptr;
    void* queue = nullptr;
    void* procInstanceCreateSurface = nullptr;
    void* procSurfaceConfigure = nullptr;
    void* procSurfaceGetCurrentTexture = nullptr;
    void* procSurfacePresent = nullptr;
    void* procSurfaceUnconfigure = nullptr;
    void* procSurfaceRelease = nullptr;
    void* procTextureRelease = nullptr;
    void* procTextureCreateView = nullptr;
    void* procTextureViewRelease = nullptr;
    void* procDeviceCreateCommandEncoder = nullptr;
    void* procCommandEncoderBeginRenderPass = nullptr;
    void* procCommandEncoderFinish = nullptr;
    void* procCommandEncoderRelease = nullptr;
    void* procCommandBufferRelease = nullptr;
    void* procQueueSubmit = nullptr;
    void* procQueueWriteBuffer = nullptr;
    void* procDeviceCreateBuffer = nullptr;
    void* procBufferRelease = nullptr;
    void* procDeviceCreateShaderModule = nullptr;
    void* procShaderModuleRelease = nullptr;
    void* procDeviceCreateRenderPipeline = nullptr;
    void* procRenderPipelineRelease = nullptr;
    void* procRenderPassEncoderEnd = nullptr;
    void* procRenderPassEncoderRelease = nullptr;
    void* procRenderPassEncoderSetPipeline = nullptr;
    void* procRenderPassEncoderSetVertexBuffer = nullptr;
    void* procRenderPassEncoderDraw = nullptr;
};

bool IsDawnCompiled();

DawnRuntimeInitResult TryInitializeDawnRuntime();
DawnRuntimeProbeInfo GetDawnRuntimeProbeInfo();
DawnRuntimeStatus GetDawnRuntimeStatus();
// Non-blocking status query for hot paths (render ticks).
// If runtime mutex is busy, returns last cached snapshot.
DawnRuntimeStatus GetDawnRuntimeStatusFast();
void ResetDawnRuntimeProbe();
bool TrySubmitNoopQueueWork(std::string* detailOut = nullptr);
bool TrySubmitEmptyCommandBuffer(std::string* detailOut = nullptr);
bool TrySubmitEmptyCommandBufferTagged(const char* tag, std::string* detailOut = nullptr);
bool TrySubmitTrailBakedPacket(uint32_t bakedVertices, uint32_t uploadBytes, std::string* detailOut = nullptr);
bool TrySubmitRippleBakedPacket(uint32_t bakedVertices, uint32_t uploadBytes, std::string* detailOut = nullptr);
bool TrySubmitRippleClickBakedPacket(uint32_t bakedVertices, uint32_t uploadBytes, std::string* detailOut = nullptr);
bool TrySubmitRippleHoverBakedPacket(uint32_t bakedVertices, uint32_t uploadBytes, std::string* detailOut = nullptr);
bool TrySubmitRippleHoldBakedPacket(uint32_t bakedVertices, uint32_t uploadBytes, std::string* detailOut = nullptr);
bool TrySubmitParticleBakedPacket(uint32_t bakedSprites, uint32_t uploadBytes, std::string* detailOut = nullptr);
bool TrySubmitMixedBakedPacket(uint32_t rippleVertices, uint32_t particleSprites, uint32_t uploadBytes, std::string* detailOut = nullptr);
DawnRuntimeSymbolStatus GetDawnRuntimeSymbolStatus();
DawnRuntimePresentContext GetDawnRuntimePresentContext();

} // namespace mousefx::gpu

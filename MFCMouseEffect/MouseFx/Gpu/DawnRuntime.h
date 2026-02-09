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
};

bool IsDawnCompiled();

DawnRuntimeInitResult TryInitializeDawnRuntime();
DawnRuntimeProbeInfo GetDawnRuntimeProbeInfo();
DawnRuntimeStatus GetDawnRuntimeStatus();
void ResetDawnRuntimeProbe();
bool TrySubmitNoopQueueWork(std::string* detailOut = nullptr);

} // namespace mousefx::gpu

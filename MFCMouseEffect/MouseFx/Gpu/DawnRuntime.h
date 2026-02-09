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
    bool canCreateInstance = false;
    std::string moduleName = "";
    std::string detail = "probe_not_run";
};

bool IsDawnCompiled();

DawnRuntimeInitResult TryInitializeDawnRuntime();
DawnRuntimeProbeInfo GetDawnRuntimeProbeInfo();
void ResetDawnRuntimeProbe();

} // namespace mousefx::gpu

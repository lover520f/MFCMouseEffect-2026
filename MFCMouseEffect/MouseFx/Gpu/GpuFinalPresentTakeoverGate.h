#pragma once

#include <cstdint>
#include <string>

namespace mousefx::gpu {

struct GpuFinalPresentTakeoverGateInput {
    bool optInEnabled = false;
    bool runtimeCapabilityLikelyAvailable = false;
    bool hostChainActive = false;
};

struct GpuFinalPresentTakeoverGateStatus {
    uint64_t probeTickMs = 0;
    bool integrationEnabledAtBuild = false;
    bool explicitOnByFile = false;
    bool forcedOffByFile = false;
    bool optInEnabled = false;
    bool runtimeCapabilityLikelyAvailable = false;
    bool hostChainActive = false;
    bool ready = false;
    std::string detail = "takeover_gate_not_probed";
};

GpuFinalPresentTakeoverGateStatus GetGpuFinalPresentTakeoverGateStatus(
    const GpuFinalPresentTakeoverGateInput& input,
    bool refresh = false);

} // namespace mousefx::gpu

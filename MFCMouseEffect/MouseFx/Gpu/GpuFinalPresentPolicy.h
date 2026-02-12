#pragma once

#include <cstdint>
#include <string>

namespace mousefx::gpu {

struct GpuFinalPresentPolicyInput {
    bool optInEnabled = false;
    bool forceLayeredCpuFallback = false;
    std::string activeBackend = "cpu";
    std::string pipelineMode = "cpu_layered";
    uint32_t activeLayerCount = 0;
    bool allLayersGpuExclusive = false;
    bool runtimeCapabilityLikelyAvailable = false;
    uint64_t processUptimeMs = 0;
    bool hasRecentGpuCommandActivity = false;
    bool hostChainActive = false;
};

struct GpuFinalPresentPolicyDecision {
    bool useLayeredSurfaces = true;
    bool eligibleForGpuFinalPresent = false;
    std::string detail = "gpu_present_policy_unknown";
};

GpuFinalPresentPolicyDecision ResolveGpuFinalPresentPolicy(const GpuFinalPresentPolicyInput& input);

} // namespace mousefx::gpu

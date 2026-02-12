#include "pch.h"

#include "GpuFinalPresentPolicy.h"

namespace mousefx::gpu {

GpuFinalPresentPolicyDecision ResolveGpuFinalPresentPolicy(const GpuFinalPresentPolicyInput& input) {
    GpuFinalPresentPolicyDecision out{};
    out.useLayeredSurfaces = true;
    out.eligibleForGpuFinalPresent = false;

    if (!input.optInEnabled) {
        out.detail = "gpu_present_optin_required";
        return out;
    }
    if (input.forceLayeredCpuFallback) {
        out.detail = "gpu_present_disabled_auto_rollback_layered_cpu";
        return out;
    }
    if (input.activeBackend != "dawn") {
        out.detail = "gpu_present_backend_not_dawn";
        return out;
    }
    if (input.pipelineMode != "dawn_compositor") {
        out.detail = "gpu_present_pipeline_not_compositor";
        return out;
    }
    if (input.activeLayerCount == 0) {
        out.detail = "gpu_present_no_active_layers";
        return out;
    }
    if (!input.allLayersGpuExclusive) {
        out.detail = "gpu_present_nonexclusive_layers_present";
        return out;
    }
    if (!input.runtimeCapabilityLikelyAvailable) {
        out.detail = "gpu_present_runtime_capability_missing";
        return out;
    }
    if (input.processUptimeMs < 6000) {
        out.detail = "gpu_present_startup_guard_active";
        return out;
    }
    if (!input.hasRecentGpuCommandActivity) {
        out.detail = "gpu_present_waiting_for_interaction_window";
        return out;
    }
    if (!input.hostChainActive) {
        out.detail = "gpu_present_host_chain_not_active";
        return out;
    }
    if (!input.hostChainTakeoverReady) {
        out.detail = "gpu_present_host_chain_takeover_not_ready";
        return out;
    }

    out.useLayeredSurfaces = false;
    out.eligibleForGpuFinalPresent = true;
    out.detail = "gpu_present_policy_eligible";
    return out;
}

} // namespace mousefx::gpu

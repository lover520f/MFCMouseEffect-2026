#pragma once

#include <cstdint>
#include <string>

namespace mousefx::gpu {

struct GpuFinalPresentHostChainStatus {
    uint64_t probeTickMs = 0;
    uint64_t activationAttempts = 0;
    uint64_t activationSuccess = 0;
    uint64_t activationFailure = 0;
    bool optInEnabled = false;
    bool runtimeCapabilityLikelyAvailable = false;
    bool readyForActivation = false;
    bool active = false;
    std::string detail = "host_chain_not_probed";
};

GpuFinalPresentHostChainStatus GetGpuFinalPresentHostChainStatus(bool refresh = false);

} // namespace mousefx::gpu

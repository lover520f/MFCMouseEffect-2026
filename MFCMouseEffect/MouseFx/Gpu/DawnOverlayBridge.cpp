#include "pch.h"

#include "DawnOverlayBridge.h"
#include "DawnRuntime.h"

namespace mousefx::gpu {

DawnOverlayBridgeStatus GetDawnOverlayBridgeStatus() {
    DawnOverlayBridgeStatus status{};
    const DawnRuntimeProbeInfo probe = GetDawnRuntimeProbeInfo();
    status.compositorApisReady = probe.hasCreateSurface && probe.hasGetQueue && probe.hasSurfacePresent && probe.canCreateDevice;
    status.compositorDetail = status.compositorApisReady ? "compositor_api_ready" : "compositor_api_missing";
#ifdef MOUSEFX_ENABLE_DAWN_OVERLAY_BRIDGE
    status.compiled = true;
    status.available = true;
    status.mode = "host_compat";
    status.detail = "bridge_enabled_host_compat";
#else
    status.compiled = false;
    status.available = false;
    status.mode = "none";
    status.detail = "bridge_not_compiled";
#endif
    return status;
}

bool IsDawnOverlayBridgeAvailable() {
    return GetDawnOverlayBridgeStatus().available;
}

} // namespace mousefx::gpu

#include "pch.h"

#include "DawnOverlayBridge.h"

namespace mousefx::gpu {

DawnOverlayBridgeStatus GetDawnOverlayBridgeStatus() {
    DawnOverlayBridgeStatus status{};
#ifdef MOUSEFX_ENABLE_DAWN_OVERLAY_BRIDGE
    status.compiled = true;
    status.available = true;
    status.detail = "bridge_enabled_host_compat";
#else
    status.compiled = false;
    status.available = false;
    status.detail = "bridge_not_compiled";
#endif
    return status;
}

bool IsDawnOverlayBridgeAvailable() {
    return GetDawnOverlayBridgeStatus().available;
}

} // namespace mousefx::gpu

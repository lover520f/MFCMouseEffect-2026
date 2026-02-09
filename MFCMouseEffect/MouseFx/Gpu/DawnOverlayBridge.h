#pragma once

#include <string>

namespace mousefx::gpu {

struct DawnOverlayBridgeStatus {
    bool compiled = false;
    bool available = false;
    std::string mode = "none";
    std::string requestedMode = "none";
    bool compositorApisReady = false;
    std::string compositorDetail = "not_checked";
    std::string detail = "bridge_not_compiled";
};

DawnOverlayBridgeStatus GetDawnOverlayBridgeStatus();
bool IsDawnOverlayBridgeAvailable();

} // namespace mousefx::gpu

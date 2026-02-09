#pragma once

#include <string>

namespace mousefx::gpu {

struct DawnOverlayBridgeStatus {
    bool compiled = false;
    bool available = false;
    std::string detail = "bridge_not_compiled";
};

DawnOverlayBridgeStatus GetDawnOverlayBridgeStatus();
bool IsDawnOverlayBridgeAvailable();

} // namespace mousefx::gpu

#include "pch.h"

#include "DawnOverlayBridge.h"
#include "DawnRuntime.h"

#include <cstdlib>
#include <mutex>

namespace mousefx::gpu {
namespace {
std::mutex g_bridgeModeMutex{};
std::string g_requestedModeOverride{};

std::string ToLowerAscii(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return s;
}

std::string NormalizeRequestedBridgeMode(const char* raw) {
    if (!raw || !*raw) return "host_compat";
    const std::string v = ToLowerAscii(std::string(raw));
    if (v == "compositor") return "compositor";
    return "host_compat";
}

std::string NormalizeRequestedBridgeMode(const std::string& raw) {
    return NormalizeRequestedBridgeMode(raw.c_str());
}

std::string ReadRequestedBridgeModeFromEnv() {
    char* raw = nullptr;
    size_t len = 0;
    const errno_t err = _dupenv_s(&raw, &len, "MFX_DAWN_BRIDGE_MODE");
    if (err != 0 || !raw || len == 0) {
        if (raw) free(raw);
        return "host_compat";
    }
    const std::string out = NormalizeRequestedBridgeMode(raw);
    free(raw);
    return out;
}

std::string ReadRequestedBridgeMode() {
    std::lock_guard<std::mutex> lock(g_bridgeModeMutex);
    if (!g_requestedModeOverride.empty()) {
        return NormalizeRequestedBridgeMode(g_requestedModeOverride);
    }
    return ReadRequestedBridgeModeFromEnv();
}

} // namespace

DawnOverlayBridgeStatus GetDawnOverlayBridgeStatus() {
    DawnOverlayBridgeStatus status{};
    const DawnRuntimeProbeInfo probe = GetDawnRuntimeProbeInfo();
    status.compositorApisReady = probe.hasCreateSurface && probe.hasGetQueue && probe.hasSurfacePresent && probe.canCreateDevice;
    status.compositorDetail = status.compositorApisReady ? "compositor_api_ready" : "compositor_api_missing";
#ifdef MOUSEFX_ENABLE_DAWN_OVERLAY_BRIDGE
    status.requestedMode = ReadRequestedBridgeMode();
    status.compiled = true;
    status.available = true;
    if (status.requestedMode == "compositor") {
        if (status.compositorApisReady) {
            status.mode = "compositor";
            status.detail = "bridge_enabled_compositor";
        } else {
            status.mode = "host_compat";
            status.detail = "bridge_fallback_host_compat_compositor_not_ready";
        }
    } else {
        status.mode = "host_compat";
        status.detail = "bridge_enabled_host_compat";
    }
#else
    status.compiled = false;
    status.available = false;
    status.mode = "none";
    status.requestedMode = "none";
    status.detail = "bridge_not_compiled";
#endif
    return status;
}

bool IsDawnOverlayBridgeAvailable() {
    return GetDawnOverlayBridgeStatus().available;
}

void SetRequestedBridgeMode(const std::string& mode) {
    std::lock_guard<std::mutex> lock(g_bridgeModeMutex);
    g_requestedModeOverride = NormalizeRequestedBridgeMode(mode);
}

std::string GetRequestedBridgeMode() {
    return ReadRequestedBridgeMode();
}

} // namespace mousefx::gpu

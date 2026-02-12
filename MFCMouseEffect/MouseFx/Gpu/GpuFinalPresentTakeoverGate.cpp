#include "pch.h"

#include "GpuFinalPresentTakeoverGate.h"

#include <mutex>

namespace mousefx::gpu {
namespace {

constexpr uint64_t kConfigProbeTtlMs = 2000;

std::wstring ResolveExeDir() {
    wchar_t modulePath[MAX_PATH] = {};
    const DWORD len = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) return L".";
    std::wstring exePath(modulePath, modulePath + len);
    const size_t pos = exePath.find_last_of(L"\\/");
    return (pos == std::wstring::npos) ? L"." : exePath.substr(0, pos);
}

bool FileExists(const std::wstring& path) {
    const DWORD attrs = GetFileAttributesW(path.c_str());
    return attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY) == 0;
}

struct CachedConfig {
    uint64_t probeTickMs = 0;
    bool integrationEnabledAtBuild = false;
    bool explicitOnByFile = false;
    bool forcedOffByFile = false;
};

CachedConfig ProbeConfigNow() {
    CachedConfig out{};
    out.probeTickMs = GetTickCount64();
#if defined(MOUSEFX_ENABLE_GPU_FINAL_PRESENT_TAKEOVER)
    out.integrationEnabledAtBuild = true;
#else
    out.integrationEnabledAtBuild = false;
#endif

    const std::wstring exeDir = ResolveExeDir();
    const std::wstring onPath = exeDir + L"\\.local\\diag\\gpu_final_present_takeover.on";
    const std::wstring offPath = exeDir + L"\\.local\\diag\\gpu_final_present_takeover.off";
    out.explicitOnByFile = FileExists(onPath);
    out.forcedOffByFile = FileExists(offPath);
    return out;
}

CachedConfig GetCachedConfig(bool refresh) {
    static std::mutex s_mutex;
    static CachedConfig s_cached{};

    const uint64_t now = GetTickCount64();
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (!refresh && s_cached.probeTickMs != 0 && now >= s_cached.probeTickMs &&
            (now - s_cached.probeTickMs) <= kConfigProbeTtlMs) {
            return s_cached;
        }
    }

    CachedConfig probed = ProbeConfigNow();
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_cached = probed;
        return s_cached;
    }
}

} // namespace

GpuFinalPresentTakeoverGateStatus GetGpuFinalPresentTakeoverGateStatus(
    const GpuFinalPresentTakeoverGateInput& input,
    bool refresh) {
    const CachedConfig cfg = GetCachedConfig(refresh);

    GpuFinalPresentTakeoverGateStatus out{};
    out.probeTickMs = cfg.probeTickMs;
    out.integrationEnabledAtBuild = cfg.integrationEnabledAtBuild;
    out.explicitOnByFile = cfg.explicitOnByFile;
    out.forcedOffByFile = cfg.forcedOffByFile;
    out.optInEnabled = input.optInEnabled;
    out.runtimeCapabilityLikelyAvailable = input.runtimeCapabilityLikelyAvailable;
    out.hostChainActive = input.hostChainActive;
    out.ready = false;
    out.detail.clear();

    if (!out.integrationEnabledAtBuild) {
        out.detail = "takeover_not_integrated_at_build";
        return out;
    }
    if (out.forcedOffByFile) {
        out.detail = "takeover_forced_off_by_file";
        return out;
    }
    if (!out.optInEnabled) {
        out.detail = "takeover_opt_in_disabled";
        return out;
    }
    if (!out.runtimeCapabilityLikelyAvailable) {
        out.detail = "takeover_runtime_capability_missing";
        return out;
    }
    if (!out.hostChainActive) {
        out.detail = "takeover_host_chain_not_active";
        return out;
    }

    out.ready = true;
    out.detail = out.explicitOnByFile
        ? "takeover_ready_explicit_on"
        : "takeover_ready_default_on";
    return out;
}

} // namespace mousefx::gpu

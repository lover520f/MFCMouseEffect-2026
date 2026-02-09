#include "pch.h"

#include "DawnRuntime.h"
#include "GpuHardwareProbe.h"

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <windows.h>
#include <mutex>

namespace mousefx::gpu {
namespace {

HMODULE g_dawnModule = nullptr;
bool g_dawnProbeAttempted = false;
DawnRuntimeProbeInfo g_probe{};
uint64_t g_probeGeneration = 0;
std::mutex g_probeMutex{};
uint64_t g_initAttempts = 0;
uint64_t g_lastInitTickMs = 0;
std::string g_lastInitDetail = "init_not_run";

using PFN_wgpuCreateInstance = void* (*)(const void*);
using PFN_wgpuInstanceRelease = void (*)(void*);
using PFN_wgpuInstanceRequestAdapter = void (*)(void*, const void*, void (*)(int, void*, const char*, void*), void*);
using PFN_wgpuAdapterRequestDevice = void (*)(void*, const void*, void (*)(int, void*, const char*, void*), void*);
using PFN_wgpuAdapterRelease = void (*)(void*);
using PFN_wgpuDeviceRelease = void (*)(void*);

HMODULE LoadFirstAvailableDawnModule(std::string* outName) {
    struct Candidate {
        const wchar_t* wide;
        const char* narrow;
    };
    const Candidate candidates[] = {
        {L"webgpu_dawn.dll", "webgpu_dawn.dll"},
        {L"dawn_native.dll", "dawn_native.dll"},
        {L"dawn.dll", "dawn.dll"},
    };

    for (const Candidate& c : candidates) {
        HMODULE mod = LoadLibraryW(c.wide);
        if (mod) {
            if (outName) *outName = c.narrow;
            return mod;
        }
    }
    return nullptr;
}

bool HasDawnCoreProc(HMODULE mod) {
    if (!mod) return false;
    FARPROC p0 = GetProcAddress(mod, "wgpuGetProcAddress");
    FARPROC p1 = GetProcAddress(mod, "dawnProcSetProcs");
    return (p0 != nullptr) || (p1 != nullptr);
}

bool HasCreateInstanceSymbol(HMODULE mod) {
    if (!mod) return false;
    // Common names seen across loaders.
    FARPROC p0 = GetProcAddress(mod, "wgpuCreateInstance");
    FARPROC p1 = GetProcAddress(mod, "webgpuCreateInstance");
    return (p0 != nullptr) || (p1 != nullptr);
}

bool HasRequestAdapterSymbol(HMODULE mod) {
    if (!mod) return false;
    FARPROC p0 = GetProcAddress(mod, "wgpuInstanceRequestAdapter");
    FARPROC p1 = GetProcAddress(mod, "webgpuInstanceRequestAdapter");
    return (p0 != nullptr) || (p1 != nullptr);
}

bool HasRequestDeviceSymbol(HMODULE mod) {
    if (!mod) return false;
    FARPROC p0 = GetProcAddress(mod, "wgpuAdapterRequestDevice");
    FARPROC p1 = GetProcAddress(mod, "webgpuAdapterRequestDevice");
    return (p0 != nullptr) || (p1 != nullptr);
}

FARPROC ResolveCreateInstanceProc(HMODULE mod) {
    if (!mod) return nullptr;
    FARPROC p0 = GetProcAddress(mod, "wgpuCreateInstance");
    if (p0) return p0;
    return GetProcAddress(mod, "webgpuCreateInstance");
}

FARPROC ResolveInstanceReleaseProc(HMODULE mod) {
    if (!mod) return nullptr;
    FARPROC p0 = GetProcAddress(mod, "wgpuInstanceRelease");
    if (p0) return p0;
    return GetProcAddress(mod, "webgpuInstanceRelease");
}

FARPROC ResolveRequestAdapterProc(HMODULE mod) {
    if (!mod) return nullptr;
    FARPROC p0 = GetProcAddress(mod, "wgpuInstanceRequestAdapter");
    if (p0) return p0;
    return GetProcAddress(mod, "webgpuInstanceRequestAdapter");
}

FARPROC ResolveRequestDeviceProc(HMODULE mod) {
    if (!mod) return nullptr;
    FARPROC p0 = GetProcAddress(mod, "wgpuAdapterRequestDevice");
    if (p0) return p0;
    return GetProcAddress(mod, "webgpuAdapterRequestDevice");
}

FARPROC ResolveAdapterReleaseProc(HMODULE mod) {
    if (!mod) return nullptr;
    FARPROC p0 = GetProcAddress(mod, "wgpuAdapterRelease");
    if (p0) return p0;
    return GetProcAddress(mod, "webgpuAdapterRelease");
}

FARPROC ResolveDeviceReleaseProc(HMODULE mod) {
    if (!mod) return nullptr;
    FARPROC p0 = GetProcAddress(mod, "wgpuDeviceRelease");
    if (p0) return p0;
    return GetProcAddress(mod, "webgpuDeviceRelease");
}

template <typename TStatus = int>
struct RequestResult {
    bool done = false;
    TStatus status = {};
    void* handle = nullptr;
    std::string message{};
    std::mutex mu{};
    std::condition_variable cv{};
};

template <typename TStatus = int>
bool WaitForRequest(RequestResult<TStatus>& result, uint32_t timeoutMs) {
    std::unique_lock<std::mutex> lock(result.mu);
    return result.cv.wait_for(lock, std::chrono::milliseconds(timeoutMs), [&result]() {
        return result.done;
    });
}

void OnRequestAdapter(int status, void* adapter, const char* message, void* userdata) {
    RequestResult<int>* out = reinterpret_cast<RequestResult<int>*>(userdata);
    if (!out) return;
    {
        std::lock_guard<std::mutex> lock(out->mu);
        out->done = true;
        out->status = status;
        out->handle = adapter;
        out->message = message ? message : "";
    }
    out->cv.notify_one();
}

void OnRequestDevice(int status, void* device, const char* message, void* userdata) {
    RequestResult<int>* out = reinterpret_cast<RequestResult<int>*>(userdata);
    if (!out) return;
    {
        std::lock_guard<std::mutex> lock(out->mu);
        out->done = true;
        out->status = status;
        out->handle = device;
        out->message = message ? message : "";
    }
    out->cv.notify_one();
}

void RunProbeIfNeededLocked() {
    if (g_dawnProbeAttempted) return;
    g_dawnProbeAttempted = true;

    g_probe = {};
    g_probe.generation = ++g_probeGeneration;
    g_probe.compiled = IsDawnCompiled();
    g_probe.hasDisplayAdapter = HasDesktopDisplayAdapter();
    if (!g_probe.hasDisplayAdapter) {
        g_probe.detail = "no_display_adapter";
        return;
    }

    if (!g_probe.compiled) {
        g_probe.detail = "dawn_disabled_at_build";
        return;
    }

    std::string modName;
    g_dawnModule = LoadFirstAvailableDawnModule(&modName);
    g_probe.moduleLoaded = (g_dawnModule != nullptr);
    g_probe.moduleName = modName;
    if (!g_probe.moduleLoaded) {
        g_probe.detail = "dawn_loader_missing";
        return;
    }

    g_probe.hasCoreProc = HasDawnCoreProc(g_dawnModule);
    g_probe.hasCreateInstance = HasCreateInstanceSymbol(g_dawnModule);
    g_probe.hasRequestAdapter = HasRequestAdapterSymbol(g_dawnModule);
    g_probe.hasRequestDevice = HasRequestDeviceSymbol(g_dawnModule);
    g_probe.canCreateInstance = false;
    g_probe.canRequestAdapter = false;
    g_probe.canCreateDevice = false;
    if (!g_probe.hasCoreProc) {
        g_probe.detail = "dawn_symbols_missing";
        return;
    }
    if (!(g_probe.hasCreateInstance && g_probe.hasRequestAdapter && g_probe.hasRequestDevice)) {
        g_probe.detail = "dawn_symbols_partial";
        return;
    }
    g_probe.detail = "dawn_runtime_ready_for_device_stage";
}

DawnRuntimeInitResult FailLocked(const char* detail) {
    DawnRuntimeInitResult result{};
    result.ok = false;
    result.backend = "cpu";
    result.detail = detail ? detail : "unknown_error";
    g_lastInitDetail = result.detail;
    g_lastInitTickMs = GetTickCount64();
    return result;
}

} // namespace

DawnRuntimeProbeInfo GetDawnRuntimeProbeInfo() {
    std::lock_guard<std::mutex> lock(g_probeMutex);
    RunProbeIfNeededLocked();
    return g_probe;
}

void ResetDawnRuntimeProbe() {
    std::lock_guard<std::mutex> lock(g_probeMutex);
    if (g_dawnModule) {
        FreeLibrary(g_dawnModule);
        g_dawnModule = nullptr;
    }
    g_dawnProbeAttempted = false;
    g_probe = {};
    g_probe.detail = "probe_reset";
    g_probe.generation = ++g_probeGeneration;
    g_initAttempts = 0;
    g_lastInitTickMs = 0;
    g_lastInitDetail = "init_not_run";
}

DawnRuntimeStatus GetDawnRuntimeStatus() {
    std::lock_guard<std::mutex> lock(g_probeMutex);
    RunProbeIfNeededLocked();

    DawnRuntimeStatus status{};
    status.probe = g_probe;
    status.lastInitDetail = g_lastInitDetail;
    status.initAttempts = g_initAttempts;
    status.lastInitTickMs = g_lastInitTickMs;
    status.readyForDeviceStage =
        (g_probe.detail == "dawn_runtime_ready_for_device_stage") ||
        (g_lastInitDetail == "dawn_instance_ok_no_device") ||
        (g_lastInitDetail == "dawn_device_ready_cpu_bridge_pending");
    return status;
}

bool IsDawnCompiled() {
#ifdef MOUSEFX_ENABLE_DAWN
    return true;
#else
    return false;
#endif
}

DawnRuntimeInitResult TryInitializeDawnRuntime() {
    std::lock_guard<std::mutex> lock(g_probeMutex);
    RunProbeIfNeededLocked();
    ++g_initAttempts;

    if (!g_probe.hasDisplayAdapter) {
        return FailLocked("no_display_adapter");
    }

    if (!g_probe.compiled) {
        return FailLocked("dawn_disabled_at_build");
    }

#ifdef MOUSEFX_ENABLE_DAWN
    if (!g_probe.moduleLoaded) {
        return FailLocked("dawn_loader_missing");
    }
    if (!g_probe.hasCoreProc) {
        return FailLocked("dawn_symbols_missing");
    }
    if (!(g_probe.hasCreateInstance && g_probe.hasRequestAdapter)) {
        return FailLocked("dawn_symbols_partial");
    }
    if (!g_probe.hasRequestDevice) {
        return FailLocked("dawn_symbols_partial");
    }

    const FARPROC createProc = ResolveCreateInstanceProc(g_dawnModule);
    if (!createProc) {
        g_probe.detail = "dawn_create_instance_proc_missing";
        return FailLocked("dawn_create_instance_proc_missing");
    }

    void* instance = reinterpret_cast<PFN_wgpuCreateInstance>(createProc)(nullptr);
    if (!instance) {
        g_probe.detail = "dawn_create_instance_failed";
        return FailLocked("dawn_create_instance_failed");
    }

    g_probe.canCreateInstance = true;
    const FARPROC requestAdapterProc = ResolveRequestAdapterProc(g_dawnModule);
    if (!requestAdapterProc) {
        return FailLocked("dawn_request_adapter_proc_missing");
    }
    RequestResult<int> adapterResult{};
    reinterpret_cast<PFN_wgpuInstanceRequestAdapter>(requestAdapterProc)(instance, nullptr, OnRequestAdapter, &adapterResult);
    if (!WaitForRequest(adapterResult, 1500)) {
        return FailLocked("dawn_request_adapter_timeout");
    }
    if (!adapterResult.handle) {
        return FailLocked("dawn_request_adapter_failed");
    }
    g_probe.canRequestAdapter = true;

    const FARPROC requestDeviceProc = ResolveRequestDeviceProc(g_dawnModule);
    if (!requestDeviceProc) {
        return FailLocked("dawn_request_device_proc_missing");
    }
    RequestResult<int> deviceResult{};
    reinterpret_cast<PFN_wgpuAdapterRequestDevice>(requestDeviceProc)(adapterResult.handle, nullptr, OnRequestDevice, &deviceResult);
    if (!WaitForRequest(deviceResult, 2000)) {
        return FailLocked("dawn_request_device_timeout");
    }
    if (!deviceResult.handle) {
        return FailLocked("dawn_request_device_failed");
    }
    g_probe.canCreateDevice = true;

    FARPROC deviceReleaseProc = ResolveDeviceReleaseProc(g_dawnModule);
    if (deviceReleaseProc) {
        reinterpret_cast<PFN_wgpuDeviceRelease>(deviceReleaseProc)(deviceResult.handle);
    }
    FARPROC adapterReleaseProc = ResolveAdapterReleaseProc(g_dawnModule);
    if (adapterReleaseProc) {
        reinterpret_cast<PFN_wgpuAdapterRelease>(adapterReleaseProc)(adapterResult.handle);
    }
    FARPROC releaseProc = ResolveInstanceReleaseProc(g_dawnModule);
    if (releaseProc) {
        reinterpret_cast<PFN_wgpuInstanceRelease>(releaseProc)(instance);
    }

    // Stage 14 status:
    // adapter/device handshake works, but overlay renderer bridge is still CPU-host based.
    DawnRuntimeInitResult result{};
    result.ok = false;
    result.backend = "cpu";
    result.detail = "dawn_device_ready_cpu_bridge_pending";
    g_probe.detail = "dawn_runtime_ready_for_device_stage";
    g_lastInitDetail = result.detail;
    g_lastInitTickMs = GetTickCount64();
    return result;
#else
    return FailLocked("dawn_disabled_at_build");
#endif
}

} // namespace mousefx::gpu

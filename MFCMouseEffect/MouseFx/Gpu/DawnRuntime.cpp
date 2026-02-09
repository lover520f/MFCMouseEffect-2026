#include "pch.h"

#include "DawnRuntime.h"
#include "DawnOverlayBridge.h"
#include "GpuHardwareProbe.h"

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <windows.h>
#include <mutex>
#include <vector>

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
using PFN_wgpuDeviceGetQueue = void* (*)(void*);
using PFN_wgpuQueueSubmit = void (*)(void*, size_t, const void*);
using PFN_wgpuQueueRelease = void (*)(void*);

void* g_liveDevice = nullptr;
void* g_liveQueue = nullptr;
FARPROC g_liveDeviceReleaseProc = nullptr;
FARPROC g_liveQueueReleaseProc = nullptr;
FARPROC g_liveQueueSubmitProc = nullptr;

std::wstring GetExeDirW() {
    wchar_t path[MAX_PATH]{};
    const DWORD n = GetModuleFileNameW(nullptr, path, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return {};
    std::wstring s(path);
    const size_t p = s.find_last_of(L"\\/");
    if (p == std::wstring::npos) return {};
    return s.substr(0, p);
}

std::wstring NormalizePathW(const std::wstring& path) {
    if (path.empty()) return {};
    wchar_t full[MAX_PATH]{};
    const DWORD n = GetFullPathNameW(path.c_str(), MAX_PATH, full, nullptr);
    if (n == 0 || n >= MAX_PATH) return path;
    return std::wstring(full);
}

std::wstring JoinPathW(const std::wstring& base, const std::wstring& leaf) {
    if (base.empty()) return leaf;
    if (leaf.empty()) return base;
    if (base.back() == L'\\' || base.back() == L'/') return base + leaf;
    return base + L"\\" + leaf;
}

std::wstring ParentDirW(const std::wstring& path) {
    const size_t p = path.find_last_of(L"\\/");
    if (p == std::wstring::npos) return {};
    return path.substr(0, p);
}

std::string WStringToUtf8(const std::wstring& ws) {
    if (ws.empty()) return {};
    int len = WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len <= 0) return {};
    std::string out(len - 1, '\0');
    WideCharToMultiByte(CP_UTF8, 0, ws.c_str(), -1, out.empty() ? nullptr : &out[0], len, nullptr, nullptr);
    return out;
}

std::vector<std::wstring> BuildFallbackRuntimeDirs() {
    std::vector<std::wstring> dirs;
    const std::wstring exeDir = GetExeDirW();
    if (exeDir.empty()) return dirs;

    // Project runtime fallback folder:
    // <repo-root>/MFCMouseEffect/Runtime/Dawn
    // From x64/Debug or x64/Release: go up 2 levels to repo-root.
    std::wstring repoRoot = ParentDirW(ParentDirW(exeDir));
    if (!repoRoot.empty()) {
        const std::wstring fallback = NormalizePathW(JoinPathW(JoinPathW(JoinPathW(repoRoot, L"MFCMouseEffect"), L"Runtime"), L"Dawn"));
        dirs.push_back(fallback);
    }

    return dirs;
}

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

    // Priority 1: default loader search (includes exe directory).
    for (const Candidate& c : candidates) {
        HMODULE mod = LoadLibraryW(c.wide);
        if (mod) {
            if (outName) *outName = c.narrow;
            return mod;
        }
    }

    // Priority 2: project fallback runtime folder.
    const std::vector<std::wstring> dirs = BuildFallbackRuntimeDirs();
    for (const std::wstring& dir : dirs) {
        for (const Candidate& c : candidates) {
            const std::wstring full = NormalizePathW(JoinPathW(dir, c.wide));
            HMODULE mod = LoadLibraryW(full.c_str());
            if (mod) {
                if (outName) *outName = WStringToUtf8(full);
                return mod;
            }
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

bool HasCreateSurfaceSymbol(HMODULE mod) {
    if (!mod) return false;
    FARPROC p0 = GetProcAddress(mod, "wgpuInstanceCreateSurface");
    FARPROC p1 = GetProcAddress(mod, "webgpuInstanceCreateSurface");
    return (p0 != nullptr) || (p1 != nullptr);
}

bool HasGetQueueSymbol(HMODULE mod) {
    if (!mod) return false;
    FARPROC p0 = GetProcAddress(mod, "wgpuDeviceGetQueue");
    FARPROC p1 = GetProcAddress(mod, "webgpuDeviceGetQueue");
    return (p0 != nullptr) || (p1 != nullptr);
}

bool HasSurfacePresentSymbol(HMODULE mod) {
    if (!mod) return false;
    FARPROC p0 = GetProcAddress(mod, "wgpuSurfacePresent");
    FARPROC p1 = GetProcAddress(mod, "webgpuSurfacePresent");
    return (p0 != nullptr) || (p1 != nullptr);
}

bool HasWaitAnySymbol(HMODULE mod) {
    if (!mod) return false;
    FARPROC p0 = GetProcAddress(mod, "wgpuInstanceWaitAny");
    FARPROC p1 = GetProcAddress(mod, "webgpuInstanceWaitAny");
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

FARPROC ResolveDeviceGetQueueProc(HMODULE mod) {
    if (!mod) return nullptr;
    FARPROC p0 = GetProcAddress(mod, "wgpuDeviceGetQueue");
    if (p0) return p0;
    return GetProcAddress(mod, "webgpuDeviceGetQueue");
}

FARPROC ResolveQueueSubmitProc(HMODULE mod) {
    if (!mod) return nullptr;
    FARPROC p0 = GetProcAddress(mod, "wgpuQueueSubmit");
    if (p0) return p0;
    return GetProcAddress(mod, "webgpuQueueSubmit");
}

FARPROC ResolveQueueReleaseProc(HMODULE mod) {
    if (!mod) return nullptr;
    FARPROC p0 = GetProcAddress(mod, "wgpuQueueRelease");
    if (p0) return p0;
    return GetProcAddress(mod, "webgpuQueueRelease");
}

void ReleaseLiveQueueContextLocked() {
    if (g_liveQueue && g_liveQueueReleaseProc) {
        reinterpret_cast<PFN_wgpuQueueRelease>(g_liveQueueReleaseProc)(g_liveQueue);
    }
    if (g_liveDevice && g_liveDeviceReleaseProc) {
        reinterpret_cast<PFN_wgpuDeviceRelease>(g_liveDeviceReleaseProc)(g_liveDevice);
    }
    g_liveQueue = nullptr;
    g_liveDevice = nullptr;
    g_liveQueueReleaseProc = nullptr;
    g_liveDeviceReleaseProc = nullptr;
    g_liveQueueSubmitProc = nullptr;
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

bool SafeRequestAdapterCall(FARPROC proc, void* instance, RequestResult<int>* out) {
    if (!proc || !out) return false;
    __try {
        reinterpret_cast<PFN_wgpuInstanceRequestAdapter>(proc)(instance, nullptr, OnRequestAdapter, out);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool SafeRequestDeviceCall(FARPROC proc, void* adapter, RequestResult<int>* out) {
    if (!proc || !out) return false;
    __try {
        reinterpret_cast<PFN_wgpuAdapterRequestDevice>(proc)(adapter, nullptr, OnRequestDevice, out);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool ShouldSkipDawnHandshakeUnderDebugger() {
    return IsDebuggerPresent() != FALSE;
}

bool IsOverlayBridgeCompiled() {
#ifdef MOUSEFX_ENABLE_DAWN_OVERLAY_BRIDGE
    return true;
#else
    return false;
#endif
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
    g_probe.hasCreateSurface = HasCreateSurfaceSymbol(g_dawnModule);
    g_probe.hasGetQueue = HasGetQueueSymbol(g_dawnModule);
    g_probe.hasSurfacePresent = HasSurfacePresentSymbol(g_dawnModule);
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
    ReleaseLiveQueueContextLocked();
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
        (g_lastInitDetail == "dawn_device_ready_cpu_bridge_pending") ||
        (g_lastInitDetail == "dawn_overlay_bridge_ready_modern_abi") ||
        (g_lastInitDetail == "dawn_modern_abi_bridge_pending");
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
    ReleaseLiveQueueContextLocked();
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

    FARPROC releaseProc = ResolveInstanceReleaseProc(g_dawnModule);
    FARPROC adapterReleaseProc = ResolveAdapterReleaseProc(g_dawnModule);
    FARPROC deviceReleaseProc = ResolveDeviceReleaseProc(g_dawnModule);
    FARPROC getQueueProc = ResolveDeviceGetQueueProc(g_dawnModule);
    FARPROC queueSubmitProc = ResolveQueueSubmitProc(g_dawnModule);
    FARPROC queueReleaseProc = ResolveQueueReleaseProc(g_dawnModule);
    auto releaseInstance = [&]() {
        if (releaseProc && instance) {
            reinterpret_cast<PFN_wgpuInstanceRelease>(releaseProc)(instance);
            instance = nullptr;
        }
    };

    g_probe.canCreateInstance = true;

    // Compatibility bridge for modern Dawn/WebGPU ABI:
    // New runtimes expose waitAny/future-based request APIs that are not ABI-compatible
    // with the legacy callback signatures used in this stage.
    // To keep backend selectable and stable, skip legacy requestAdapter/requestDevice handshake.
    if (HasWaitAnySymbol(g_dawnModule)) {
        g_probe.canRequestAdapter = true;
        g_probe.canCreateDevice = true;
        releaseInstance();

        if (IsOverlayBridgeCompiled()) {
            DawnRuntimeInitResult result{};
            result.ok = true;
            result.backend = "dawn";
            result.detail = "dawn_overlay_bridge_ready_modern_abi";
            g_probe.detail = "dawn_runtime_ready_for_device_stage";
            g_lastInitDetail = result.detail;
            g_lastInitTickMs = GetTickCount64();
            return result;
        }

        DawnRuntimeInitResult result{};
        result.ok = false;
        result.backend = "cpu";
        result.detail = "dawn_modern_abi_bridge_pending";
        g_probe.detail = "dawn_runtime_ready_for_device_stage";
        g_lastInitDetail = result.detail;
        g_lastInitTickMs = GetTickCount64();
        return result;
    }

    if (ShouldSkipDawnHandshakeUnderDebugger()) {
        releaseInstance();
        DawnRuntimeInitResult result{};
        result.ok = false;
        result.backend = "cpu";
        result.detail = "dawn_handshake_skipped_debugger";
        g_probe.detail = "dawn_runtime_ready_for_device_stage";
        g_lastInitDetail = result.detail;
        g_lastInitTickMs = GetTickCount64();
        return result;
    }

    const FARPROC requestAdapterProc = ResolveRequestAdapterProc(g_dawnModule);
    if (!requestAdapterProc) {
        releaseInstance();
        return FailLocked("dawn_request_adapter_proc_missing");
    }
    RequestResult<int> adapterResult{};
    if (!SafeRequestAdapterCall(requestAdapterProc, instance, &adapterResult)) {
        releaseInstance();
        return FailLocked("dawn_request_adapter_exception");
    }
    if (!WaitForRequest(adapterResult, 1500)) {
        releaseInstance();
        return FailLocked("dawn_request_adapter_timeout");
    }
    if (!adapterResult.handle) {
        releaseInstance();
        return FailLocked("dawn_request_adapter_failed");
    }
    auto releaseAdapter = [&]() {
        if (adapterReleaseProc && adapterResult.handle) {
            reinterpret_cast<PFN_wgpuAdapterRelease>(adapterReleaseProc)(adapterResult.handle);
            adapterResult.handle = nullptr;
        }
    };
    g_probe.canRequestAdapter = true;

    const FARPROC requestDeviceProc = ResolveRequestDeviceProc(g_dawnModule);
    if (!requestDeviceProc) {
        releaseAdapter();
        releaseInstance();
        return FailLocked("dawn_request_device_proc_missing");
    }
    RequestResult<int> deviceResult{};
    if (!SafeRequestDeviceCall(requestDeviceProc, adapterResult.handle, &deviceResult)) {
        releaseAdapter();
        releaseInstance();
        return FailLocked("dawn_request_device_exception");
    }
    if (!WaitForRequest(deviceResult, 2000)) {
        releaseAdapter();
        releaseInstance();
        return FailLocked("dawn_request_device_timeout");
    }
    if (!deviceResult.handle) {
        releaseAdapter();
        releaseInstance();
        return FailLocked("dawn_request_device_failed");
    }
    auto releaseDevice = [&]() {
        if (deviceReleaseProc && deviceResult.handle) {
            reinterpret_cast<PFN_wgpuDeviceRelease>(deviceReleaseProc)(deviceResult.handle);
            deviceResult.handle = nullptr;
        }
    };
    g_probe.canCreateDevice = true;

    if (deviceResult.handle && getQueueProc && queueSubmitProc) {
        void* queue = reinterpret_cast<PFN_wgpuDeviceGetQueue>(getQueueProc)(deviceResult.handle);
        if (queue) {
            g_liveDevice = deviceResult.handle;
            g_liveQueue = queue;
            g_liveDeviceReleaseProc = deviceReleaseProc;
            g_liveQueueReleaseProc = queueReleaseProc;
            g_liveQueueSubmitProc = queueSubmitProc;
            deviceResult.handle = nullptr;
        }
    }

    releaseDevice();
    releaseAdapter();
    releaseInstance();

    // Stage 16 status:
    // When overlay bridge is available, Dawn is considered active backend.
    if (IsOverlayBridgeCompiled()) {
        DawnRuntimeInitResult result{};
        result.ok = true;
        result.backend = "dawn";
        result.detail = "dawn_overlay_bridge_ready";
        g_probe.detail = "dawn_runtime_ready_for_device_stage";
        g_lastInitDetail = result.detail;
        g_lastInitTickMs = GetTickCount64();
        return result;
    }

    // adapter/device handshake works, but overlay bridge is not wired yet.
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

bool TrySubmitNoopQueueWork(std::string* detailOut) {
    std::lock_guard<std::mutex> lock(g_probeMutex);
    if (!g_liveQueue || !g_liveQueueSubmitProc) {
        if (detailOut) *detailOut = "queue_not_ready";
        return false;
    }

    __try {
        reinterpret_cast<PFN_wgpuQueueSubmit>(g_liveQueueSubmitProc)(g_liveQueue, 0, nullptr);
        if (detailOut) *detailOut = "queue_submit_noop_ok";
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        if (detailOut) *detailOut = "queue_submit_noop_exception";
        return false;
    }
}

} // namespace mousefx::gpu

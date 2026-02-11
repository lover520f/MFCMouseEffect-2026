#include "pch.h"

#include "DawnRuntime.h"
#include "DawnOverlayBridge.h"
#include "GpuHardwareProbe.h"

#include <chrono>
#include <condition_variable>
#include <cstring>
#include <windows.h>
#include <atomic>
#include <mutex>
#include <sstream>
#include <vector>
#include <memory>

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
std::string g_modernAbiPrimeDetail = "not_attempted";
bool g_legacyPrimeCompatBlocked = false;
std::shared_ptr<const DawnRuntimeStatus> g_cachedRuntimeStatus{};
DawnRuntimeSymbolStatus g_cachedSymbolStatus{};
uint64_t g_cachedSymbolStatusTickMs = 0;
uint64_t g_cachedSymbolStatusProbeGeneration = 0;
bool g_cachedSymbolStatusValid = false;
constexpr uint64_t kSymbolStatusCacheMs = 2000;

using PFN_wgpuCreateInstance = void* (*)(const void*);
using PFN_wgpuInstanceRelease = void (*)(void*);
using PFN_wgpuInstanceRequestAdapter = void (*)(void*, const void*, void (*)(int, void*, const char*, void*), void*);
using PFN_wgpuAdapterRequestDevice = void (*)(void*, const void*, void (*)(int, void*, const char*, void*), void*);
using PFN_wgpuAdapterRelease = void (*)(void*);
using PFN_wgpuDeviceRelease = void (*)(void*);
using PFN_wgpuDeviceGetQueue = void* (*)(void*);
using PFN_wgpuQueueSubmit = void (*)(void*, size_t, const void*);
using PFN_wgpuQueueRelease = void (*)(void*);
using PFN_wgpuDeviceCreateCommandEncoder = void* (*)(void*, const void*);
using PFN_wgpuCommandEncoderFinish = void* (*)(void*, const void*);
using PFN_wgpuCommandEncoderRelease = void (*)(void*);
using PFN_wgpuCommandBufferRelease = void (*)(void*);
using WGPUProcRaw = void (*)(void);
using WGPUFutureId = uint64_t;
using WGPUCallbackModeRaw = uint32_t;
using WGPUWaitStatusRaw = uint32_t;
using WGPURequestAdapterStatusRaw = uint32_t;
using WGPURequestDeviceStatusRaw = uint32_t;
struct WGPUStringViewRaw {
    const char* data = nullptr;
    size_t length = static_cast<size_t>(-1);
};
using PFN_wgpuGetProcAddress = WGPUProcRaw (*)(WGPUStringViewRaw);
using PFN_dawnNativeGetProcsMangled = const void* (*)();
struct WGPUFutureRaw {
    WGPUFutureId id = 0;
};
struct WGPUFutureWaitInfoRaw {
    WGPUFutureRaw future{};
    uint32_t completed = 0;
};
struct WGPURequestAdapterOptionsRaw {
    void* nextInChain = nullptr;
    uint32_t featureLevel = 0;
    uint32_t powerPreference = 0;
    uint32_t forceFallbackAdapter = 0;
    uint32_t backendType = 0;
    void* compatibleSurface = nullptr;
};
struct WGPUInstanceDescriptorRaw {
    void* nextInChain = nullptr;
    size_t requiredFeatureCount = 0;
    const uint32_t* requiredFeatures = nullptr;
    const void* requiredLimits = nullptr;
};
using PFN_wgpuRequestAdapterCallbackModern = void (*)(WGPURequestAdapterStatusRaw, void*, WGPUStringViewRaw, void*, void*);
using PFN_wgpuRequestDeviceCallbackModern = void (*)(WGPURequestDeviceStatusRaw, void*, WGPUStringViewRaw, void*, void*);
struct WGPURequestAdapterCallbackInfoRaw {
    void* nextInChain = nullptr;
    WGPUCallbackModeRaw mode = 0;
    PFN_wgpuRequestAdapterCallbackModern callback = nullptr;
    void* userdata1 = nullptr;
    void* userdata2 = nullptr;
};
struct WGPURequestDeviceCallbackInfoRaw {
    void* nextInChain = nullptr;
    WGPUCallbackModeRaw mode = 0;
    PFN_wgpuRequestDeviceCallbackModern callback = nullptr;
    void* userdata1 = nullptr;
    void* userdata2 = nullptr;
};
using PFN_wgpuInstanceRequestAdapterModern = WGPUFutureRaw (*)(void*, const void*, WGPURequestAdapterCallbackInfoRaw);
using PFN_wgpuAdapterRequestDeviceModern = WGPUFutureRaw (*)(void*, const void*, WGPURequestDeviceCallbackInfoRaw);
using PFN_wgpuInstanceWaitAny = WGPUWaitStatusRaw (*)(void*, size_t, WGPUFutureWaitInfoRaw*, uint64_t);
using PFN_wgpuInstanceProcessEvents = void (*)(void*);

constexpr WGPUCallbackModeRaw kWGPUCallbackModeWaitAnyOnly = 1u;
constexpr WGPUCallbackModeRaw kWGPUCallbackModeAllowProcessEvents = 2u;
constexpr WGPUCallbackModeRaw kWGPUCallbackModeAllowSpontaneous = 3u;
constexpr WGPUWaitStatusRaw kWGPUWaitStatusSuccess = 1u;
constexpr WGPUWaitStatusRaw kWGPUWaitStatusTimedOut = 2u;
constexpr WGPURequestAdapterStatusRaw kWGPURequestAdapterStatusSuccess = 1u;
constexpr WGPURequestDeviceStatusRaw kWGPURequestDeviceStatusSuccess = 1u;
constexpr uint32_t kWGPUFeatureLevelCompatibility = 1u;

void* g_liveDevice = nullptr;
void* g_liveQueue = nullptr;
FARPROC g_liveDeviceReleaseProc = nullptr;
FARPROC g_liveQueueReleaseProc = nullptr;
FARPROC g_liveQueueSubmitProc = nullptr;
FARPROC g_liveCreateCommandEncoderProc = nullptr;
FARPROC g_liveCommandEncoderFinishProc = nullptr;
FARPROC g_liveCommandEncoderReleaseProc = nullptr;
FARPROC g_liveCommandBufferReleaseProc = nullptr;

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

bool FileExistsW(const std::wstring& path) {
    if (path.empty()) return false;
    DWORD attr = GetFileAttributesW(path.c_str());
    return attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY) == 0;
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

bool TryPreloadD3DCompiler47(std::string* detailOut) {
    const wchar_t* kName = L"d3dcompiler_47.dll";

    auto tryLoad = [&](const std::wstring& fullPath, const char* tag) -> bool {
        if (fullPath.empty()) return false;
        if (!FileExistsW(fullPath)) return false;
        HMODULE mod = LoadLibraryW(fullPath.c_str());
        if (!mod) return false;
        if (detailOut) {
            std::string p = WStringToUtf8(fullPath);
            *detailOut = std::string(tag) + ":" + (p.empty() ? "loaded" : p);
        }
        return true;
    };

    // 1) exe dir
    const std::wstring exeDir = GetExeDirW();
    if (!exeDir.empty()) {
        if (tryLoad(JoinPathW(exeDir, kName), "d3dcompiler47_preload_exe")) return true;
    }

    // 2) project runtime fallback dir
    for (const std::wstring& dir : BuildFallbackRuntimeDirs()) {
        if (tryLoad(JoinPathW(dir, kName), "d3dcompiler47_preload_runtime")) return true;
    }

    // 3) system32 absolute path
    wchar_t sysDir[MAX_PATH]{};
    UINT n = GetSystemDirectoryW(sysDir, MAX_PATH);
    if (n > 0 && n < MAX_PATH) {
        if (tryLoad(JoinPathW(std::wstring(sysDir), kName), "d3dcompiler47_preload_system32")) return true;
    }

    // 4) default search as last resort
    HMODULE mod = LoadLibraryW(kName);
    if (mod) {
        if (detailOut) *detailOut = "d3dcompiler47_preload_default:loaded";
        return true;
    }

    if (detailOut) *detailOut = "d3dcompiler47_preload_failed";
    return false;
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

bool HasAnySymbol(HMODULE mod, const char* const* names, size_t count) {
    if (!mod || !names || count == 0) return false;
    for (size_t i = 0; i < count; ++i) {
        const char* n = names[i];
        if (!n || !*n) continue;
        if (GetProcAddress(mod, n)) return true;
    }
    return false;
}

bool HasModernRequestAdapterSymbol(HMODULE mod) {
    if (HasWaitAnySymbol(mod) && HasRequestAdapterSymbol(mod)) return true;
    static const char* kNames[] = {
        "wgpuInstanceRequestAdapter2",
        "webgpuInstanceRequestAdapter2",
        "wgpuInstanceRequestAdapterFuture",
        "webgpuInstanceRequestAdapterFuture"
    };
    return HasAnySymbol(mod, kNames, sizeof(kNames) / sizeof(kNames[0]));
}

bool HasModernRequestDeviceSymbol(HMODULE mod) {
    if (HasWaitAnySymbol(mod) && HasRequestDeviceSymbol(mod)) return true;
    static const char* kNames[] = {
        "wgpuAdapterRequestDevice2",
        "webgpuAdapterRequestDevice2",
        "wgpuAdapterRequestDeviceFuture",
        "webgpuAdapterRequestDeviceFuture"
    };
    return HasAnySymbol(mod, kNames, sizeof(kNames) / sizeof(kNames[0]));
}

bool HasInstanceProcessEventsSymbol(HMODULE mod) {
    static const char* kNames[] = {
        "wgpuInstanceProcessEvents",
        "webgpuInstanceProcessEvents"
    };
    return HasAnySymbol(mod, kNames, sizeof(kNames) / sizeof(kNames[0]));
}

FARPROC ResolveCreateInstanceProc(HMODULE mod) {
    if (!mod) return nullptr;
    FARPROC p0 = GetProcAddress(mod, "wgpuCreateInstance");
    if (p0) return p0;
    return GetProcAddress(mod, "webgpuCreateInstance");
}

FARPROC ResolveViaWgpuGetProcAddress(HMODULE mod, const char* primary, const char* secondary = nullptr, std::string* sourceOut = nullptr) {
    if (!mod || !primary || !*primary) return nullptr;
    struct DawnProcTablePrefixRaw {
        void* createInstance;
        void* getInstanceFeatures;
        void* getInstanceLimits;
        void* hasInstanceFeature;
        PFN_wgpuGetProcAddress getProcAddress;
    };

    PFN_wgpuGetProcAddress getProc = nullptr;

    FARPROC getProcRaw = GetProcAddress(mod, "wgpuGetProcAddress");
    if (getProcRaw) {
        getProc = reinterpret_cast<PFN_wgpuGetProcAddress>(getProcRaw);
        if (sourceOut) *sourceOut = "wgpu_get_proc_address";
    }

    if (!getProc) {
        static const char* kGetProcsCandidates[] = {
            "?GetProcs@native@dawn@@YAAEBUDawnProcTable@@XZ",
            "?GetProcs@native@dawn@@YAPEBUDawnProcTable@@XZ"
        };
        FARPROC getProcsSym = nullptr;
        for (const char* name : kGetProcsCandidates) {
            getProcsSym = GetProcAddress(mod, name);
            if (getProcsSym) break;
        }
        if (getProcsSym) {
            auto getProcs = reinterpret_cast<PFN_dawnNativeGetProcsMangled>(getProcsSym);
            const DawnProcTablePrefixRaw* table =
                reinterpret_cast<const DawnProcTablePrefixRaw*>(getProcs());
            if (table) {
                getProc = table->getProcAddress;
                if (sourceOut) *sourceOut = "dawn_get_procs_table";
            }
        }
    }
    if (!getProc) return nullptr;

    WGPUStringViewRaw sv0{};
    sv0.data = primary;
    sv0.length = static_cast<size_t>(-1);
    WGPUProcRaw p = getProc(sv0);
    if (p) return reinterpret_cast<FARPROC>(p);

    if (secondary && *secondary) {
        WGPUStringViewRaw sv1{};
        sv1.data = secondary;
        sv1.length = static_cast<size_t>(-1);
        p = getProc(sv1);
        if (p) return reinterpret_cast<FARPROC>(p);
    }
    return nullptr;
}

FARPROC ResolveInstanceReleaseProc(HMODULE mod) {
    if (!mod) return nullptr;
    FARPROC p0 = GetProcAddress(mod, "wgpuInstanceRelease");
    if (p0) return p0;
    return GetProcAddress(mod, "webgpuInstanceRelease");
}

FARPROC ResolveRequestAdapterProc(HMODULE mod, std::string* sourceOut = nullptr) {
    if (!mod) return nullptr;
    FARPROC pByGetProc = ResolveViaWgpuGetProcAddress(mod, "wgpuInstanceRequestAdapter", "webgpuInstanceRequestAdapter", sourceOut);
    if (pByGetProc) return pByGetProc;
    FARPROC p0 = GetProcAddress(mod, "wgpuInstanceRequestAdapter");
    if (p0) {
        if (sourceOut) *sourceOut = "export_wgpu";
        return p0;
    }
    FARPROC p1 = GetProcAddress(mod, "webgpuInstanceRequestAdapter");
    if (p1 && sourceOut) *sourceOut = "export_webgpu";
    return p1;
}

FARPROC ResolveRequestDeviceProc(HMODULE mod, std::string* sourceOut = nullptr) {
    if (!mod) return nullptr;
    FARPROC pByGetProc = ResolveViaWgpuGetProcAddress(mod, "wgpuAdapterRequestDevice", "webgpuAdapterRequestDevice", sourceOut);
    if (pByGetProc) return pByGetProc;
    FARPROC p0 = GetProcAddress(mod, "wgpuAdapterRequestDevice");
    if (p0) {
        if (sourceOut) *sourceOut = "export_wgpu";
        return p0;
    }
    FARPROC p1 = GetProcAddress(mod, "webgpuAdapterRequestDevice");
    if (p1 && sourceOut) *sourceOut = "export_webgpu";
    return p1;
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

bool HasExport(HMODULE mod, const char* name) {
    if (!mod || !name || !*name) return false;
    return GetProcAddress(mod, name) != nullptr;
}

FARPROC ResolveDeviceCreateCommandEncoderProc(HMODULE mod) {
    if (!mod) return nullptr;
    FARPROC p0 = GetProcAddress(mod, "wgpuDeviceCreateCommandEncoder");
    if (p0) return p0;
    return GetProcAddress(mod, "webgpuDeviceCreateCommandEncoder");
}

FARPROC ResolveCommandEncoderFinishProc(HMODULE mod) {
    if (!mod) return nullptr;
    FARPROC p0 = GetProcAddress(mod, "wgpuCommandEncoderFinish");
    if (p0) return p0;
    return GetProcAddress(mod, "webgpuCommandEncoderFinish");
}

FARPROC ResolveCommandEncoderReleaseProc(HMODULE mod) {
    if (!mod) return nullptr;
    FARPROC p0 = GetProcAddress(mod, "wgpuCommandEncoderRelease");
    if (p0) return p0;
    return GetProcAddress(mod, "webgpuCommandEncoderRelease");
}

FARPROC ResolveCommandBufferReleaseProc(HMODULE mod) {
    if (!mod) return nullptr;
    FARPROC p0 = GetProcAddress(mod, "wgpuCommandBufferRelease");
    if (p0) return p0;
    return GetProcAddress(mod, "webgpuCommandBufferRelease");
}

FARPROC ResolveInstanceWaitAnyProc(HMODULE mod, std::string* sourceOut = nullptr) {
    if (!mod) return nullptr;
    FARPROC pByGetProc = ResolveViaWgpuGetProcAddress(mod, "wgpuInstanceWaitAny", "webgpuInstanceWaitAny", sourceOut);
    if (pByGetProc) return pByGetProc;
    FARPROC p0 = GetProcAddress(mod, "wgpuInstanceWaitAny");
    if (p0) {
        if (sourceOut) *sourceOut = "export_wgpu";
        return p0;
    }
    FARPROC p1 = GetProcAddress(mod, "webgpuInstanceWaitAny");
    if (p1 && sourceOut) *sourceOut = "export_webgpu";
    return p1;
}

FARPROC ResolveInstanceProcessEventsProc(HMODULE mod, std::string* sourceOut = nullptr) {
    if (!mod) return nullptr;
    FARPROC pByGetProc = ResolveViaWgpuGetProcAddress(mod, "wgpuInstanceProcessEvents", "webgpuInstanceProcessEvents", sourceOut);
    if (pByGetProc) return pByGetProc;
    FARPROC p0 = GetProcAddress(mod, "wgpuInstanceProcessEvents");
    if (p0) {
        if (sourceOut) *sourceOut = "export_wgpu";
        return p0;
    }
    FARPROC p1 = GetProcAddress(mod, "webgpuInstanceProcessEvents");
    if (p1 && sourceOut) *sourceOut = "export_webgpu";
    return p1;
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
    g_liveCreateCommandEncoderProc = nullptr;
    g_liveCommandEncoderFinishProc = nullptr;
    g_liveCommandEncoderReleaseProc = nullptr;
    g_liveCommandBufferReleaseProc = nullptr;
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

std::string StringViewToUtf8(const WGPUStringViewRaw& sv) {
    if (!sv.data) return {};
    size_t len = sv.length;
    if (len == static_cast<size_t>(-1)) {
        len = std::strlen(sv.data);
    }
    return std::string(sv.data, sv.data + len);
}

std::string ClipMessageForDiag(const std::string& s) {
    if (s.empty()) return {};
    constexpr size_t kMax = 120;
    if (s.size() <= kMax) return s;
    return s.substr(0, kMax) + "...";
}

void OnRequestAdapterModern(WGPURequestAdapterStatusRaw status, void* adapter, WGPUStringViewRaw message, void* userdata1, void* userdata2) {
    (void)userdata2;
    RequestResult<uint32_t>* out = reinterpret_cast<RequestResult<uint32_t>*>(userdata1);
    if (!out) return;
    {
        std::lock_guard<std::mutex> lock(out->mu);
        out->done = true;
        out->status = status;
        out->handle = adapter;
        out->message = StringViewToUtf8(message);
    }
    out->cv.notify_one();
}

void OnRequestDeviceModern(WGPURequestDeviceStatusRaw status, void* device, WGPUStringViewRaw message, void* userdata1, void* userdata2) {
    (void)userdata2;
    RequestResult<uint32_t>* out = reinterpret_cast<RequestResult<uint32_t>*>(userdata1);
    if (!out) return;
    {
        std::lock_guard<std::mutex> lock(out->mu);
        out->done = true;
        out->status = status;
        out->handle = device;
        out->message = StringViewToUtf8(message);
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

bool SafeQueueSubmitNoopCall(FARPROC proc, void* queue) {
    if (!proc || !queue) return false;
    __try {
        reinterpret_cast<PFN_wgpuQueueSubmit>(proc)(queue, 0, nullptr);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool SafeModernRequestAdapterInvoke(
    FARPROC proc,
    void* instance,
    const void* options,
    const WGPURequestAdapterCallbackInfoRaw* callbackInfo,
    WGPUFutureRaw* outFuture) {
    if (!proc || !instance || !callbackInfo || !outFuture) return false;
    __try {
        *outFuture = reinterpret_cast<PFN_wgpuInstanceRequestAdapterModern>(proc)(instance, options, *callbackInfo);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool SafeModernRequestDeviceInvoke(
    FARPROC proc,
    void* adapter,
    const WGPURequestDeviceCallbackInfoRaw* callbackInfo,
    WGPUFutureRaw* outFuture) {
    if (!proc || !adapter || !callbackInfo || !outFuture) return false;
    __try {
        *outFuture = reinterpret_cast<PFN_wgpuAdapterRequestDeviceModern>(proc)(adapter, nullptr, *callbackInfo);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool SafeModernWaitAnyInvoke(
    FARPROC waitAnyProc,
    void* instance,
    size_t futureCount,
    WGPUFutureWaitInfoRaw* waitInfos,
    uint64_t timeoutNs,
    WGPUWaitStatusRaw* outStatus) {
    if (!waitAnyProc || !instance || !waitInfos || !outStatus) return false;
    __try {
        *outStatus = reinterpret_cast<PFN_wgpuInstanceWaitAny>(waitAnyProc)(instance, futureCount, waitInfos, timeoutNs);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool SafeRequestAdapterCallModern(
    FARPROC requestProc,
    FARPROC waitAnyProc,
    FARPROC processEventsProc,
    void* instance,
    RequestResult<uint32_t>* out,
    WGPUCallbackModeRaw callbackMode,
    uint32_t timeoutMs,
    std::string* reasonOut = nullptr) {
    if (!requestProc || !waitAnyProc || !instance || !out) {
        if (reasonOut) *reasonOut = "input_missing";
        return false;
    }

    WGPURequestAdapterCallbackInfoRaw cbInfo{};
    cbInfo.mode = callbackMode;
    cbInfo.callback = OnRequestAdapterModern;
    cbInfo.userdata1 = out;
    WGPURequestAdapterOptionsRaw options{};
    options.featureLevel = kWGPUFeatureLevelCompatibility;

    WGPUFutureRaw future{};
    if (!SafeModernRequestAdapterInvoke(requestProc, instance, &options, &cbInfo, &future)) {
        if (reasonOut) *reasonOut = "invoke_exception";
        return false;
    }
    if (future.id == 0) {
        if (reasonOut) *reasonOut = "future_zero";
        return false;
    }

    WGPUFutureWaitInfoRaw waitInfo{};
    waitInfo.future = future;
    const uint64_t deadline = GetTickCount64() + timeoutMs;
    bool usedUnexpectedWaitStatusFallback = false;
    while (GetTickCount64() <= deadline) {
        waitInfo.completed = 0;
        WGPUWaitStatusRaw waitStatus = 0;
        if (!SafeModernWaitAnyInvoke(waitAnyProc, instance, 1, &waitInfo, 50ull * 1000ull * 1000ull, &waitStatus)) {
            if (reasonOut) *reasonOut = "wait_any_exception";
            return false;
        }
        if (waitStatus == kWGPUWaitStatusSuccess) {
            if (waitInfo.completed) break;
        } else if (waitStatus == kWGPUWaitStatusTimedOut) {
            // keep polling
        } else if (processEventsProc) {
            // Some runtimes may transiently return non-success/non-timeout statuses
            // during callback progress. Keep polling until deadline instead of failing immediately.
            usedUnexpectedWaitStatusFallback = true;
            reinterpret_cast<PFN_wgpuInstanceProcessEvents>(processEventsProc)(instance);
            std::unique_lock<std::mutex> lock(out->mu);
            if (out->done) break;
            lock.unlock();
            Sleep(5);
            continue;
        } else {
            if (reasonOut) {
                std::ostringstream oss;
                oss << "wait_status_" << waitStatus;
                *reasonOut = oss.str();
            }
            return false;
        }
        if (processEventsProc) {
            reinterpret_cast<PFN_wgpuInstanceProcessEvents>(processEventsProc)(instance);
        }
        std::unique_lock<std::mutex> lock(out->mu);
        if (out->done) break;
    }

    if (!WaitForRequest(*out, 600)) {
        if (reasonOut) *reasonOut = "callback_timeout";
        return false;
    }
    if (reasonOut) {
        *reasonOut = usedUnexpectedWaitStatusFallback ? "ok_wait_status_fallback" : "ok";
    }
    return true;
}

bool SafeRequestDeviceCallModern(
    FARPROC requestProc,
    FARPROC waitAnyProc,
    FARPROC processEventsProc,
    void* instance,
    void* adapter,
    RequestResult<uint32_t>* out,
    WGPUCallbackModeRaw callbackMode,
    uint32_t timeoutMs,
    std::string* reasonOut = nullptr) {
    if (!requestProc || !waitAnyProc || !instance || !adapter || !out) {
        if (reasonOut) *reasonOut = "input_missing";
        return false;
    }

    WGPURequestDeviceCallbackInfoRaw cbInfo{};
    cbInfo.mode = callbackMode;
    cbInfo.callback = OnRequestDeviceModern;
    cbInfo.userdata1 = out;

    WGPUFutureRaw future{};
    if (!SafeModernRequestDeviceInvoke(requestProc, adapter, &cbInfo, &future)) {
        if (reasonOut) *reasonOut = "invoke_exception";
        return false;
    }
    if (future.id == 0) {
        if (reasonOut) *reasonOut = "future_zero";
        return false;
    }

    WGPUFutureWaitInfoRaw waitInfo{};
    waitInfo.future = future;
    const uint64_t deadline = GetTickCount64() + timeoutMs;
    bool usedUnexpectedWaitStatusFallback = false;
    while (GetTickCount64() <= deadline) {
        waitInfo.completed = 0;
        WGPUWaitStatusRaw waitStatus = 0;
        if (!SafeModernWaitAnyInvoke(waitAnyProc, instance, 1, &waitInfo, 50ull * 1000ull * 1000ull, &waitStatus)) {
            if (reasonOut) *reasonOut = "wait_any_exception";
            return false;
        }
        if (waitStatus == kWGPUWaitStatusSuccess) {
            if (waitInfo.completed) break;
        } else if (waitStatus == kWGPUWaitStatusTimedOut) {
            // keep polling
        } else if (processEventsProc) {
            usedUnexpectedWaitStatusFallback = true;
            reinterpret_cast<PFN_wgpuInstanceProcessEvents>(processEventsProc)(instance);
            std::unique_lock<std::mutex> lock(out->mu);
            if (out->done) break;
            lock.unlock();
            Sleep(5);
            continue;
        } else {
            if (reasonOut) {
                std::ostringstream oss;
                oss << "wait_status_" << waitStatus;
                *reasonOut = oss.str();
            }
            return false;
        }
        if (processEventsProc) {
            reinterpret_cast<PFN_wgpuInstanceProcessEvents>(processEventsProc)(instance);
        }
        std::unique_lock<std::mutex> lock(out->mu);
        if (out->done) break;
    }

    if (!WaitForRequest(*out, 600)) {
        if (reasonOut) *reasonOut = "callback_timeout";
        return false;
    }
    if (reasonOut) {
        *reasonOut = usedUnexpectedWaitStatusFallback ? "ok_wait_status_fallback" : "ok";
    }
    return true;
}

bool TryPrimeQueueWithLegacyCallbacks(
    void* instance,
    FARPROC requestAdapterProc,
    FARPROC requestDeviceProc,
    FARPROC adapterReleaseProc,
    FARPROC deviceReleaseProc,
    FARPROC getQueueProc,
    FARPROC queueSubmitProc,
    FARPROC queueReleaseProc,
    FARPROC createEncoderProc,
    FARPROC finishProc,
    FARPROC encoderReleaseProc,
    FARPROC bufferReleaseProc,
    std::string* detailOut) {
    if (!instance) {
        if (detailOut) *detailOut = "instance_null";
        return false;
    }
    if (!requestAdapterProc) {
        if (detailOut) *detailOut = "request_adapter_proc_missing";
        return false;
    }
    if (!requestDeviceProc) {
        if (detailOut) *detailOut = "request_device_proc_missing";
        return false;
    }

    RequestResult<int> adapterResult{};
    if (!SafeRequestAdapterCall(requestAdapterProc, instance, &adapterResult)) {
        if (detailOut) *detailOut = "request_adapter_exception";
        return false;
    }
    if (!WaitForRequest(adapterResult, 400)) {
        if (detailOut) *detailOut = "request_adapter_timeout";
        return false;
    }
    if (!adapterResult.handle) {
        if (detailOut) *detailOut = "request_adapter_failed";
        return false;
    }

    auto releaseAdapter = [&]() {
        if (adapterReleaseProc && adapterResult.handle) {
            reinterpret_cast<PFN_wgpuAdapterRelease>(adapterReleaseProc)(adapterResult.handle);
            adapterResult.handle = nullptr;
        }
    };

    RequestResult<int> deviceResult{};
    if (!SafeRequestDeviceCall(requestDeviceProc, adapterResult.handle, &deviceResult)) {
        if (detailOut) *detailOut = "request_device_exception";
        releaseAdapter();
        return false;
    }
    if (!WaitForRequest(deviceResult, 600)) {
        if (detailOut) *detailOut = "request_device_timeout";
        releaseAdapter();
        return false;
    }
    if (!deviceResult.handle) {
        if (detailOut) *detailOut = "request_device_failed";
        releaseAdapter();
        return false;
    }

    if (!getQueueProc || !queueSubmitProc) {
        if (detailOut) *detailOut = !getQueueProc ? "device_get_queue_proc_missing" : "queue_submit_proc_missing";
        if (deviceReleaseProc && deviceResult.handle) {
            reinterpret_cast<PFN_wgpuDeviceRelease>(deviceReleaseProc)(deviceResult.handle);
            deviceResult.handle = nullptr;
        }
        releaseAdapter();
        return false;
    }

    void* queue = reinterpret_cast<PFN_wgpuDeviceGetQueue>(getQueueProc)(deviceResult.handle);
    if (!queue) {
        if (detailOut) *detailOut = "device_get_queue_failed";
        if (deviceReleaseProc && deviceResult.handle) {
            reinterpret_cast<PFN_wgpuDeviceRelease>(deviceReleaseProc)(deviceResult.handle);
            deviceResult.handle = nullptr;
        }
        releaseAdapter();
        return false;
    }

    g_liveDevice = deviceResult.handle;
    g_liveQueue = queue;
    g_liveDeviceReleaseProc = deviceReleaseProc;
    g_liveQueueReleaseProc = queueReleaseProc;
    g_liveQueueSubmitProc = queueSubmitProc;
    g_liveCreateCommandEncoderProc = createEncoderProc;
    g_liveCommandEncoderFinishProc = finishProc;
    g_liveCommandEncoderReleaseProc = encoderReleaseProc;
    g_liveCommandBufferReleaseProc = bufferReleaseProc;
    deviceResult.handle = nullptr;
    if (detailOut) *detailOut = "queue_ready";
    releaseAdapter();
    return true;
}

bool TryPrimeQueueWithModernWaitAny(
    void* instance,
    FARPROC requestAdapterProc,
    FARPROC requestDeviceProc,
    FARPROC waitAnyProc,
    FARPROC processEventsProc,
    FARPROC adapterReleaseProc,
    FARPROC deviceReleaseProc,
    FARPROC getQueueProc,
    FARPROC queueSubmitProc,
    FARPROC queueReleaseProc,
    FARPROC createEncoderProc,
    FARPROC finishProc,
    FARPROC encoderReleaseProc,
    FARPROC bufferReleaseProc,
    const std::string& requestAdapterSource,
    const std::string& requestDeviceSource,
    const std::string& waitAnySource,
    const std::string& processEventsSource,
    std::string* detailOut) {
    if (!instance) {
        if (detailOut) *detailOut = "instance_null";
        return false;
    }
    if (!requestAdapterProc || !requestDeviceProc) {
        if (detailOut) *detailOut = "request_proc_missing";
        return false;
    }
    if (!waitAnyProc) {
        if (detailOut) *detailOut = "wait_any_proc_missing";
        return false;
    }

    auto modeName = [](WGPUCallbackModeRaw mode) -> const char* {
        if (mode == kWGPUCallbackModeWaitAnyOnly) return "wait_any_only";
        if (mode == kWGPUCallbackModeAllowProcessEvents) return "allow_process_events";
        if (mode == kWGPUCallbackModeAllowSpontaneous) return "allow_spontaneous";
        return "unknown_mode";
    };
    auto shouldRetry = [](const std::string& reason) -> bool {
        if (reason.empty()) return false;
        return reason.find("callback_timeout") != std::string::npos ||
               reason.find("wait_status_") != std::string::npos ||
               reason.find("wait_any_exception") != std::string::npos;
    };

    const WGPUCallbackModeRaw modeCandidates[] = {
        kWGPUCallbackModeWaitAnyOnly,
        kWGPUCallbackModeAllowProcessEvents,
        kWGPUCallbackModeAllowSpontaneous,
    };

    RequestResult<uint32_t> adapterResult{};
    RequestResult<uint32_t> deviceResult{};
    auto resetRequestResult = [](RequestResult<uint32_t>& r) {
        std::lock_guard<std::mutex> lock(r.mu);
        r.done = false;
        r.status = 0;
        r.handle = nullptr;
        r.message.clear();
    };
    auto releaseAdapter = [&]() {
        if (adapterReleaseProc && adapterResult.handle) {
            reinterpret_cast<PFN_wgpuAdapterRelease>(adapterReleaseProc)(adapterResult.handle);
            adapterResult.handle = nullptr;
        }
    };
    auto releaseDevice = [&]() {
        if (deviceReleaseProc && deviceResult.handle) {
            reinterpret_cast<PFN_wgpuDeviceRelease>(deviceReleaseProc)(deviceResult.handle);
            deviceResult.handle = nullptr;
        }
    };

    WGPUCallbackModeRaw selectedMode = modeCandidates[0];
    std::string adapterReason;
    std::string deviceReason;
    bool primeOk = false;

    for (size_t i = 0; i < _countof(modeCandidates); ++i) {
        selectedMode = modeCandidates[i];
        resetRequestResult(adapterResult);
        resetRequestResult(deviceResult);
        adapterReason.clear();
        deviceReason.clear();
        releaseAdapter();
        releaseDevice();

        if (!SafeRequestAdapterCallModern(requestAdapterProc, waitAnyProc, processEventsProc, instance, &adapterResult, selectedMode, 1500, &adapterReason)) {
            if (i + 1 < _countof(modeCandidates) && shouldRetry(adapterReason)) {
                continue;
            }
            if (detailOut) {
                std::ostringstream oss;
                oss << "modern_request_adapter_" << (adapterReason.empty() ? "failed" : adapterReason)
                    << "_mode:" << modeName(selectedMode)
                    << "@ra:" << (requestAdapterSource.empty() ? "unknown" : requestAdapterSource)
                    << ",wa:" << (waitAnySource.empty() ? "unknown" : waitAnySource)
                    << ",pe:" << (processEventsSource.empty() ? "none" : processEventsSource);
                *detailOut = oss.str();
            }
            return false;
        }

        if (!adapterResult.handle || adapterResult.status != kWGPURequestAdapterStatusSuccess) {
            if (i + 1 < _countof(modeCandidates)) {
                continue;
            }
            if (detailOut) {
                std::ostringstream oss;
                oss << "modern_request_adapter_status_" << adapterResult.status;
                if (!adapterResult.message.empty()) {
                    oss << "_msg:" << ClipMessageForDiag(adapterResult.message);
                }
                *detailOut = oss.str();
            }
            return false;
        }

        if (!SafeRequestDeviceCallModern(requestDeviceProc, waitAnyProc, processEventsProc, instance, adapterResult.handle, &deviceResult, selectedMode, 2000, &deviceReason)) {
            releaseAdapter();
            if (i + 1 < _countof(modeCandidates) && shouldRetry(deviceReason)) {
                continue;
            }
            if (detailOut) {
                std::ostringstream oss;
                oss << "modern_request_device_" << (deviceReason.empty() ? "failed" : deviceReason)
                    << "_mode:" << modeName(selectedMode)
                    << "@rd:" << (requestDeviceSource.empty() ? "unknown" : requestDeviceSource)
                    << ",wa:" << (waitAnySource.empty() ? "unknown" : waitAnySource)
                    << ",pe:" << (processEventsSource.empty() ? "none" : processEventsSource);
                *detailOut = oss.str();
            }
            return false;
        }

        if (!deviceResult.handle || deviceResult.status != kWGPURequestDeviceStatusSuccess) {
            const bool adapterConsumed =
                deviceResult.status == 3 &&
                deviceResult.message.find("consumed") != std::string::npos;
            releaseDevice();
            releaseAdapter();
            if (i + 1 < _countof(modeCandidates) && adapterConsumed) {
                continue;
            }
            if (detailOut) {
                std::ostringstream oss;
                oss << "modern_request_device_status_" << deviceResult.status;
                if (!deviceResult.message.empty()) {
                    oss << "_msg:" << ClipMessageForDiag(deviceResult.message);
                }
                oss << "_mode:" << modeName(selectedMode);
                *detailOut = oss.str();
            }
            return false;
        }

        primeOk = true;
        break;
    }

    if (!primeOk) {
        if (detailOut) *detailOut = "modern_prime_mode_exhausted";
        return false;
    }

    if (!getQueueProc || !queueSubmitProc) {
        if (detailOut) *detailOut = !getQueueProc ? "device_get_queue_proc_missing" : "queue_submit_proc_missing";
        releaseDevice();
        releaseAdapter();
        return false;
    }

    void* queue = reinterpret_cast<PFN_wgpuDeviceGetQueue>(getQueueProc)(deviceResult.handle);
    if (!queue) {
        if (detailOut) *detailOut = "device_get_queue_failed";
        releaseDevice();
        releaseAdapter();
        return false;
    }

    g_liveDevice = deviceResult.handle;
    g_liveQueue = queue;
    g_liveDeviceReleaseProc = deviceReleaseProc;
    g_liveQueueReleaseProc = queueReleaseProc;
    g_liveQueueSubmitProc = queueSubmitProc;
    g_liveCreateCommandEncoderProc = createEncoderProc;
    g_liveCommandEncoderFinishProc = finishProc;
    g_liveCommandEncoderReleaseProc = encoderReleaseProc;
    g_liveCommandBufferReleaseProc = bufferReleaseProc;
    deviceResult.handle = nullptr;
    if (detailOut) *detailOut = "queue_ready";
    releaseAdapter();
    return true;
}

bool SafeSubmitEmptyCommandBufferCall(
    FARPROC createEncoderProc,
    FARPROC finishProc,
    FARPROC encoderReleaseProc,
    FARPROC bufferReleaseProc,
    FARPROC queueSubmitProc,
    void* device,
    void* queue) {
    if (!createEncoderProc || !finishProc || !queueSubmitProc || !device || !queue) return false;
    __try {
        void* encoder = reinterpret_cast<PFN_wgpuDeviceCreateCommandEncoder>(createEncoderProc)(device, nullptr);
        if (!encoder) return false;
        void* commandBuffer = reinterpret_cast<PFN_wgpuCommandEncoderFinish>(finishProc)(encoder, nullptr);
        if (encoderReleaseProc) {
            reinterpret_cast<PFN_wgpuCommandEncoderRelease>(encoderReleaseProc)(encoder);
            encoder = nullptr;
        }
        if (!commandBuffer) return false;

        void* submitList[1] = { commandBuffer };
        reinterpret_cast<PFN_wgpuQueueSubmit>(queueSubmitProc)(queue, 1, submitList);
        if (bufferReleaseProc) {
            reinterpret_cast<PFN_wgpuCommandBufferRelease>(bufferReleaseProc)(commandBuffer);
            commandBuffer = nullptr;
        }
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

bool ShouldSkipDawnHandshakeUnderDebugger() {
    return IsDebuggerPresent() != FALSE;
}

bool ShouldSkipLegacyPrimeOnModernAbi() {
    return g_probe.hasWaitAny;
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
    g_probe.hasWaitAny = HasWaitAnySymbol(g_dawnModule);
    g_probe.hasModernRequestAdapter = HasModernRequestAdapterSymbol(g_dawnModule);
    g_probe.hasModernRequestDevice = HasModernRequestDeviceSymbol(g_dawnModule);
    g_probe.hasInstanceProcessEvents = HasInstanceProcessEventsSymbol(g_dawnModule);
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

DawnRuntimeStatus BuildRuntimeStatusLocked() {
    DawnRuntimeStatus status{};
    status.probe = g_probe;
    status.lastInitDetail = g_lastInitDetail;
    status.initAttempts = g_initAttempts;
    status.lastInitTickMs = g_lastInitTickMs;
    status.readyForDeviceStage =
        (g_probe.detail == "dawn_runtime_ready_for_device_stage") ||
        (g_lastInitDetail == "dawn_instance_ok_no_device") ||
        (g_lastInitDetail == "dawn_device_ready_cpu_bridge_pending") ||
        (g_lastInitDetail == "dawn_overlay_bridge_ready_modern_abi_queue_ready") ||
        (g_lastInitDetail == "dawn_overlay_bridge_ready_modern_abi") ||
        (g_lastInitDetail == "dawn_modern_abi_bridge_pending");
    status.queueReady = (g_liveDevice != nullptr) && (g_liveQueue != nullptr) && (g_liveQueueSubmitProc != nullptr);
    status.commandEncoderReady = status.queueReady &&
        (g_liveCreateCommandEncoderProc != nullptr) &&
        (g_liveCommandEncoderFinishProc != nullptr);
    status.modernAbiDetected = status.probe.hasWaitAny;
    status.modernAbiNativeReady =
        status.modernAbiDetected &&
        status.probe.hasModernRequestAdapter &&
        status.probe.hasModernRequestDevice;
    if (!status.modernAbiDetected) {
        status.modernAbiNativeDetail = "not_modern_abi";
    } else if (status.probe.hasRequestAdapter && status.probe.hasRequestDevice &&
               !status.probe.hasModernRequestAdapter && !status.probe.hasModernRequestDevice) {
        status.modernAbiNativeDetail = "legacy_request_symbols_only";
    } else if (!status.probe.hasModernRequestAdapter) {
        status.modernAbiNativeDetail = "missing_modern_request_adapter_symbol";
    } else if (!status.probe.hasModernRequestDevice) {
        status.modernAbiNativeDetail = "missing_modern_request_device_symbol";
    } else if (!status.probe.hasInstanceProcessEvents) {
        status.modernAbiNativeDetail = "missing_instance_process_events_symbol";
    } else {
        status.modernAbiNativeDetail = "modern_native_symbols_ready";
    }
    status.modernAbiPrimeDetail = g_modernAbiPrimeDetail;
    if (!status.modernAbiDetected) {
        status.modernAbiStrategy = "legacy_callbacks";
    } else if (g_legacyPrimeCompatBlocked && g_modernAbiPrimeDetail.rfind("legacy_", 0) == 0) {
        status.modernAbiStrategy = "legacy_prime_blocked";
    } else if (status.modernAbiPrimeDetail == "queue_ready") {
        status.modernAbiStrategy = "modern_queue_ready";
    } else {
        status.modernAbiStrategy = "modern_waitany_prime";
    }
    return status;
}

void StoreRuntimeStatusSnapshotLocked(const DawnRuntimeStatus& status) {
    std::shared_ptr<const DawnRuntimeStatus> snapshot =
        std::make_shared<DawnRuntimeStatus>(status);
    std::atomic_store_explicit(
        &g_cachedRuntimeStatus,
        snapshot,
        std::memory_order_release);
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
    g_modernAbiPrimeDetail = "not_attempted";
    g_legacyPrimeCompatBlocked = false;
    g_cachedRuntimeStatus.reset();
    g_cachedSymbolStatusValid = false;
    g_cachedSymbolStatusTickMs = 0;
    g_cachedSymbolStatusProbeGeneration = 0;
    g_cachedSymbolStatus = DawnRuntimeSymbolStatus{};
}

DawnRuntimeStatus GetDawnRuntimeStatus() {
    std::lock_guard<std::mutex> lock(g_probeMutex);
    RunProbeIfNeededLocked();
    DawnRuntimeStatus status = BuildRuntimeStatusLocked();
    StoreRuntimeStatusSnapshotLocked(status);
    return status;
}

DawnRuntimeStatus GetDawnRuntimeStatusFast() {
    {
        std::unique_lock<std::mutex> lock(g_probeMutex, std::try_to_lock);
        if (lock.owns_lock()) {
            RunProbeIfNeededLocked();
            DawnRuntimeStatus status = BuildRuntimeStatusLocked();
            StoreRuntimeStatusSnapshotLocked(status);
            return status;
        }
    }

    auto cached = std::atomic_load_explicit(&g_cachedRuntimeStatus, std::memory_order_acquire);
    if (cached) {
        return *cached;
    }

    DawnRuntimeStatus fallback{};
    fallback.lastInitDetail = "status_snapshot_not_ready";
    fallback.modernAbiPrimeDetail = "status_snapshot_not_ready";
    return fallback;
}

DawnRuntimeSymbolStatus GetDawnRuntimeSymbolStatus() {
    std::lock_guard<std::mutex> lock(g_probeMutex);
    RunProbeIfNeededLocked();

    const uint64_t nowMs = GetTickCount64();
    if (g_cachedSymbolStatusValid &&
        g_cachedSymbolStatusProbeGeneration == g_probe.generation &&
        nowMs >= g_cachedSymbolStatusTickMs &&
        (nowMs - g_cachedSymbolStatusTickMs) <= kSymbolStatusCacheMs) {
        return g_cachedSymbolStatus;
    }

    DawnRuntimeSymbolStatus out{};
    out.moduleLoaded = (g_dawnModule != nullptr);
    out.moduleName = g_probe.moduleName;
    if (!out.moduleLoaded) {
        out.summary = "module_not_loaded";
        return out;
    }

    out.hasWgpuGetProcAddress = HasExport(g_dawnModule, "wgpuGetProcAddress");
    out.hasCreateInstance = HasExport(g_dawnModule, "wgpuCreateInstance") || HasExport(g_dawnModule, "webgpuCreateInstance");
    out.hasRequestAdapterLegacy = HasExport(g_dawnModule, "wgpuInstanceRequestAdapter") || HasExport(g_dawnModule, "webgpuInstanceRequestAdapter");
    out.hasRequestDeviceLegacy = HasExport(g_dawnModule, "wgpuAdapterRequestDevice") || HasExport(g_dawnModule, "webgpuAdapterRequestDevice");
    out.hasRequestAdapterModern = HasModernRequestAdapterSymbol(g_dawnModule);
    out.hasRequestDeviceModern = HasModernRequestDeviceSymbol(g_dawnModule);
    out.hasWaitAny = HasWaitAnySymbol(g_dawnModule);
    out.hasInstanceProcessEvents = HasInstanceProcessEventsSymbol(g_dawnModule);

    if (out.hasRequestAdapterModern && out.hasRequestDeviceModern) {
        out.summary = "modern_request_symbols_ready";
    } else if (out.hasRequestAdapterLegacy && out.hasRequestDeviceLegacy) {
        out.summary = "legacy_request_symbols_only";
    } else {
        out.summary = "request_symbols_incomplete";
    }

    g_cachedSymbolStatus = out;
    g_cachedSymbolStatusTickMs = nowMs;
    g_cachedSymbolStatusProbeGeneration = g_probe.generation;
    g_cachedSymbolStatusValid = true;
    return out;
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
    std::string d3dCompilerPreloadDetail;
    TryPreloadD3DCompiler47(&d3dCompilerPreloadDetail);

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

    WGPUInstanceDescriptorRaw instanceDesc{};
    void* instance = reinterpret_cast<PFN_wgpuCreateInstance>(createProc)(&instanceDesc);
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
    FARPROC createEncoderProc = ResolveDeviceCreateCommandEncoderProc(g_dawnModule);
    FARPROC finishProc = ResolveCommandEncoderFinishProc(g_dawnModule);
    FARPROC encoderReleaseProc = ResolveCommandEncoderReleaseProc(g_dawnModule);
    FARPROC bufferReleaseProc = ResolveCommandBufferReleaseProc(g_dawnModule);
    std::string requestAdapterSource;
    std::string requestDeviceSource;
    FARPROC requestAdapterProc = ResolveRequestAdapterProc(g_dawnModule, &requestAdapterSource);
    FARPROC requestDeviceProc = ResolveRequestDeviceProc(g_dawnModule, &requestDeviceSource);
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
        bool primed = false;
        g_modernAbiPrimeDetail = "attempting_modern_prime";
        std::string primeDetail;
        std::string waitAnySource;
        std::string processEventsSource;
        FARPROC waitAnyProc = ResolveInstanceWaitAnyProc(g_dawnModule, &waitAnySource);
        FARPROC processEventsProc = ResolveInstanceProcessEventsProc(g_dawnModule, &processEventsSource);
        primed = TryPrimeQueueWithModernWaitAny(
            instance,
            requestAdapterProc,
            requestDeviceProc,
            waitAnyProc,
            processEventsProc,
            adapterReleaseProc,
            deviceReleaseProc,
            getQueueProc,
            queueSubmitProc,
            queueReleaseProc,
            createEncoderProc,
            finishProc,
            encoderReleaseProc,
            bufferReleaseProc,
            requestAdapterSource,
            requestDeviceSource,
            waitAnySource,
            processEventsSource,
            &primeDetail);
        g_modernAbiPrimeDetail = primeDetail.empty() ? "unknown" : primeDetail;
        if (!d3dCompilerPreloadDetail.empty() &&
            g_modernAbiPrimeDetail.find("d3dcompiler_47.dll") != std::string::npos) {
            g_modernAbiPrimeDetail += "|preload=" + d3dCompilerPreloadDetail;
        }
        if (!primed &&
            (g_modernAbiPrimeDetail == "modern_request_adapter_exception" ||
             g_modernAbiPrimeDetail == "modern_request_device_exception")) {
            g_legacyPrimeCompatBlocked = true;
        }
        releaseInstance();

        if (IsOverlayBridgeCompiled()) {
            DawnRuntimeInitResult result{};
            result.ok = true;
            result.backend = "dawn";
            result.detail = primed
                ? "dawn_overlay_bridge_ready_modern_abi_queue_ready"
                : "dawn_overlay_bridge_ready_modern_abi";
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
            g_liveCreateCommandEncoderProc = createEncoderProc;
            g_liveCommandEncoderFinishProc = finishProc;
            g_liveCommandEncoderReleaseProc = encoderReleaseProc;
            g_liveCommandBufferReleaseProc = bufferReleaseProc;
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
    void* queue = nullptr;
    FARPROC submitProc = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_probeMutex);
        queue = g_liveQueue;
        submitProc = g_liveQueueSubmitProc;
    }

    if (!queue || !submitProc) {
        if (detailOut) *detailOut = "queue_not_ready";
        return false;
    }

    if (SafeQueueSubmitNoopCall(submitProc, queue)) {
        if (detailOut) *detailOut = "queue_submit_noop_ok";
        return true;
    }
    if (detailOut) *detailOut = "queue_submit_noop_exception";
    return false;
}

bool TrySubmitEmptyCommandBufferTagged(const char* tag, std::string* detailOut) {
    void* device = nullptr;
    void* queue = nullptr;
    FARPROC createEncoderProc = nullptr;
    FARPROC finishProc = nullptr;
    FARPROC encoderReleaseProc = nullptr;
    FARPROC bufferReleaseProc = nullptr;
    FARPROC queueSubmitProc = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_probeMutex);
        device = g_liveDevice;
        queue = g_liveQueue;
        createEncoderProc = g_liveCreateCommandEncoderProc;
        finishProc = g_liveCommandEncoderFinishProc;
        encoderReleaseProc = g_liveCommandEncoderReleaseProc;
        bufferReleaseProc = g_liveCommandBufferReleaseProc;
        queueSubmitProc = g_liveQueueSubmitProc;
    }

    if (!device || !queue || !queueSubmitProc) {
        if (detailOut) *detailOut = "queue_or_device_not_ready";
        return false;
    }
    if (!createEncoderProc || !finishProc) {
        if (detailOut) *detailOut = "command_encoder_api_missing";
        return false;
    }

    if (SafeSubmitEmptyCommandBufferCall(
            createEncoderProc,
            finishProc,
            encoderReleaseProc,
            bufferReleaseProc,
            queueSubmitProc,
            device,
            queue)) {
        if (detailOut) {
            if (tag && *tag) {
                *detailOut = std::string("empty_command_buffer_submit_ok_") + tag;
            } else {
                *detailOut = "empty_command_buffer_submit_ok";
            }
        }
        return true;
    }

    if (detailOut) {
        if (tag && *tag) {
            *detailOut = std::string("empty_command_buffer_submit_exception_") + tag;
        } else {
            *detailOut = "empty_command_buffer_submit_exception";
        }
    }
    return false;
}

bool TrySubmitEmptyCommandBuffer(std::string* detailOut) {
    return TrySubmitEmptyCommandBufferTagged(nullptr, detailOut);
}

bool TrySubmitTrailBakedPacket(uint32_t bakedVertices, uint32_t uploadBytes, std::string* detailOut) {
    if (bakedVertices == 0 || uploadBytes == 0) {
        if (detailOut) *detailOut = "trail_packet_empty";
        return false;
    }

    std::string submitDetail;
    if (!TrySubmitEmptyCommandBufferTagged("trail_pass", &submitDetail)) {
        if (detailOut) {
            if (submitDetail.empty()) {
                *detailOut = "trail_packet_submit_fail";
            } else {
                *detailOut = std::string("trail_packet_submit_fail_") + submitDetail;
            }
        }
        return false;
    }

    if (detailOut) {
        std::ostringstream oss;
        oss << "trail_packet_submit_ok_v" << bakedVertices << "_u" << uploadBytes;
        *detailOut = oss.str();
    }
    return true;
}

bool TrySubmitRippleBakedPacket(uint32_t bakedVertices, uint32_t uploadBytes, std::string* detailOut) {
    if (bakedVertices == 0 || uploadBytes == 0) {
        if (detailOut) *detailOut = "ripple_packet_empty";
        return false;
    }

    std::string submitDetail;
    if (!TrySubmitEmptyCommandBufferTagged("ripple_pass", &submitDetail)) {
        if (detailOut) {
            if (submitDetail.empty()) {
                *detailOut = "ripple_packet_submit_fail";
            } else {
                *detailOut = std::string("ripple_packet_submit_fail_") + submitDetail;
            }
        }
        return false;
    }

    if (detailOut) {
        std::ostringstream oss;
        oss << "ripple_packet_submit_ok_v" << bakedVertices << "_u" << uploadBytes;
        *detailOut = oss.str();
    }
    return true;
}

bool TrySubmitParticleBakedPacket(uint32_t bakedSprites, uint32_t uploadBytes, std::string* detailOut) {
    if (bakedSprites == 0 || uploadBytes == 0) {
        if (detailOut) *detailOut = "particle_packet_empty";
        return false;
    }

    std::string submitDetail;
    if (!TrySubmitEmptyCommandBufferTagged("particle_pass", &submitDetail)) {
        if (detailOut) {
            if (submitDetail.empty()) {
                *detailOut = "particle_packet_submit_fail";
            } else {
                *detailOut = std::string("particle_packet_submit_fail_") + submitDetail;
            }
        }
        return false;
    }

    if (detailOut) {
        std::ostringstream oss;
        oss << "particle_packet_submit_ok_s" << bakedSprites << "_u" << uploadBytes;
        *detailOut = oss.str();
    }
    return true;
}

} // namespace mousefx::gpu

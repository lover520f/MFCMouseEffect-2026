#include "pch.h"

#include "DawnRuntime.h"
#include "GpuHardwareProbe.h"

#include <windows.h>

namespace mousefx::gpu {
namespace {

HMODULE g_dawnModule = nullptr;
bool g_dawnProbeAttempted = false;
DawnRuntimeProbeInfo g_probe{};

using PFN_wgpuCreateInstance = void* (*)(const void*);
using PFN_wgpuInstanceRelease = void (*)(void*);

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

void RunProbeIfNeeded() {
    if (g_dawnProbeAttempted) return;
    g_dawnProbeAttempted = true;

    g_probe = {};
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
    g_probe.canCreateInstance = false;
    if (!g_probe.hasCoreProc) {
        g_probe.detail = "dawn_symbols_missing";
        return;
    }
    if (!(g_probe.hasCreateInstance && g_probe.hasRequestAdapter)) {
        g_probe.detail = "dawn_symbols_partial";
        return;
    }
    g_probe.detail = "dawn_runtime_ready_for_device_stage";
}

} // namespace

const DawnRuntimeProbeInfo& GetDawnRuntimeProbeInfo() {
    RunProbeIfNeeded();
    return g_probe;
}

bool IsDawnCompiled() {
#ifdef MOUSEFX_ENABLE_DAWN
    return true;
#else
    return false;
#endif
}

DawnRuntimeInitResult TryInitializeDawnRuntime() {
    RunProbeIfNeeded();

    DawnRuntimeInitResult result{};
    if (!g_probe.hasDisplayAdapter) {
        result.ok = false;
        result.backend = "cpu";
        result.detail = "no_display_adapter";
        return result;
    }

    if (!g_probe.compiled) {
        result.ok = false;
        result.backend = "cpu";
        result.detail = "dawn_disabled_at_build";
        return result;
    }

#ifdef MOUSEFX_ENABLE_DAWN
    if (!g_probe.moduleLoaded) {
        result.ok = false;
        result.backend = "cpu";
        result.detail = "dawn_loader_missing";
        return result;
    }
    if (!g_probe.hasCoreProc) {
        result.ok = false;
        result.backend = "cpu";
        result.detail = "dawn_symbols_missing";
        return result;
    }
    if (!(g_probe.hasCreateInstance && g_probe.hasRequestAdapter)) {
        result.ok = false;
        result.backend = "cpu";
        result.detail = "dawn_symbols_partial";
        return result;
    }

    const FARPROC createProc = ResolveCreateInstanceProc(g_dawnModule);
    if (!createProc) {
        result.ok = false;
        result.backend = "cpu";
        result.detail = "dawn_create_instance_proc_missing";
        g_probe.detail = result.detail;
        return result;
    }

    void* instance = reinterpret_cast<PFN_wgpuCreateInstance>(createProc)(nullptr);
    if (!instance) {
        result.ok = false;
        result.backend = "cpu";
        result.detail = "dawn_create_instance_failed";
        g_probe.detail = result.detail;
        return result;
    }

    g_probe.canCreateInstance = true;
    FARPROC releaseProc = ResolveInstanceReleaseProc(g_dawnModule);
    if (releaseProc) {
        reinterpret_cast<PFN_wgpuInstanceRelease>(releaseProc)(instance);
    }

    // Stage 6 status:
    // Dawn instance creation works, but adapter/device/surface is not wired yet.
    result.ok = false;
    result.backend = "cpu";
    result.detail = "dawn_instance_ok_no_device";
    g_probe.detail = "dawn_runtime_ready_for_device_stage";
    return result;
#else
    result.ok = false;
    result.backend = "cpu";
    result.detail = "dawn_disabled_at_build";
    return result;
#endif
}

} // namespace mousefx::gpu

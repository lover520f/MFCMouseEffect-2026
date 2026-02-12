#include "pch.h"

#include "GpuFinalPresentCapabilityProbe.h"

#include <mutex>

namespace mousefx::gpu {
namespace {

constexpr uint64_t kProbeCacheTtlMs = 2000;

void ProbeSingleDll(
    const wchar_t* moduleName,
    const char* procName,
    bool* dllLoadedOut,
    bool* procAvailableOut) {
    if (!dllLoadedOut || !procAvailableOut) return;
    *dllLoadedOut = false;
    *procAvailableOut = false;

    HMODULE module = GetModuleHandleW(moduleName);
    bool loadedByThisProbe = false;
    if (!module) {
        module = LoadLibraryW(moduleName);
        loadedByThisProbe = (module != nullptr);
    }
    if (!module) return;

    *dllLoadedOut = true;
    FARPROC proc = GetProcAddress(module, procName);
    *procAvailableOut = (proc != nullptr);

    if (loadedByThisProbe) {
        FreeLibrary(module);
    }
}

GpuFinalPresentCapability ProbeNow() {
    GpuFinalPresentCapability status{};
    status.probeTickMs = GetTickCount64();

    ProbeSingleDll(L"dcomp.dll", "DCompositionCreateDevice", &status.dcompDllLoaded, &status.dcompCreateDeviceProc);
    ProbeSingleDll(L"d3d11.dll", "D3D11CreateDevice", &status.d3d11DllLoaded, &status.d3d11CreateDeviceProc);
    ProbeSingleDll(L"dxgi.dll", "CreateDXGIFactory1", &status.dxgiDllLoaded, &status.createDxgiFactoryProc);

    status.likelyAvailable =
        status.dcompCreateDeviceProc &&
        status.d3d11CreateDeviceProc &&
        status.createDxgiFactoryProc;

    if (status.likelyAvailable) {
        status.detail = "dcomp_d3d11_dxgi_runtime_ready";
    } else if (!status.dcompDllLoaded) {
        status.detail = "dcomp_dll_missing";
    } else if (!status.dcompCreateDeviceProc) {
        status.detail = "dcomp_create_device_proc_missing";
    } else if (!status.d3d11DllLoaded) {
        status.detail = "d3d11_dll_missing";
    } else if (!status.d3d11CreateDeviceProc) {
        status.detail = "d3d11_create_device_proc_missing";
    } else if (!status.dxgiDllLoaded) {
        status.detail = "dxgi_dll_missing";
    } else if (!status.createDxgiFactoryProc) {
        status.detail = "dxgi_factory_proc_missing";
    } else {
        status.detail = "final_present_probe_unknown";
    }
    return status;
}

} // namespace

GpuFinalPresentCapability GetGpuFinalPresentCapability(bool refresh) {
    static std::mutex s_mutex;
    static GpuFinalPresentCapability s_cached{};

    const uint64_t now = GetTickCount64();
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (!refresh && s_cached.probeTickMs != 0 && now >= s_cached.probeTickMs &&
            (now - s_cached.probeTickMs) <= kProbeCacheTtlMs) {
            return s_cached;
        }
    }

    GpuFinalPresentCapability probed = ProbeNow();
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_cached = probed;
        return s_cached;
    }
}

} // namespace mousefx::gpu

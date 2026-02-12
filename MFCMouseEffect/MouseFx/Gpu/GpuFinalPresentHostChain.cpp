#include "pch.h"

#include "GpuFinalPresentHostChain.h"

#include "GpuFinalPresentCapabilityProbe.h"

#include <d3d11.h>
#include <dcomp.h>
#include <fstream>
#include <mutex>

namespace mousefx::gpu {
namespace {

constexpr uint64_t kCacheTtlMs = 800;

using PfnD3D11CreateDevice = HRESULT (WINAPI*)(
    IDXGIAdapter*,
    D3D_DRIVER_TYPE,
    HMODULE,
    UINT,
    const D3D_FEATURE_LEVEL*,
    UINT,
    UINT,
    ID3D11Device**,
    D3D_FEATURE_LEVEL*,
    ID3D11DeviceContext**);

using PfnDCompositionCreateDevice = HRESULT (WINAPI*)(IUnknown*, REFIID, void**);

struct HostChainRuntimeState {
    ID3D11Device* d3d11Device = nullptr;
    ID3D11DeviceContext* d3d11Context = nullptr;
    IDCompositionDevice* dcompDevice = nullptr;
    bool active = false;
    uint64_t activationAttempts = 0;
    uint64_t activationSuccess = 0;
    uint64_t activationFailure = 0;
    std::string lastDetail = "host_chain_not_initialized";
};

void ReleaseRuntimeState(HostChainRuntimeState* state, const char* detail) {
    if (!state) return;
    if (state->dcompDevice) {
        state->dcompDevice->Release();
        state->dcompDevice = nullptr;
    }
    if (state->d3d11Context) {
        state->d3d11Context->Release();
        state->d3d11Context = nullptr;
    }
    if (state->d3d11Device) {
        state->d3d11Device->Release();
        state->d3d11Device = nullptr;
    }
    state->active = false;
    state->lastDetail = (detail && *detail) ? detail : "host_chain_released";
}

bool IsGpuFinalPresentOptInEnabled() {
    wchar_t modulePath[MAX_PATH] = {};
    const DWORD len = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) return false;

    std::wstring exePath(modulePath, modulePath + len);
    const size_t pos = exePath.find_last_of(L"\\/");
    const std::wstring exeDir = (pos == std::wstring::npos) ? L"." : exePath.substr(0, pos);
    const std::wstring optInPath = exeDir + L"\\.local\\diag\\gpu_final_present.optin";
    std::ifstream fin(optInPath, std::ios::binary);
    return fin.good();
}

bool TryActivateHostChain(HostChainRuntimeState* state, std::string* detailOut) {
    if (!state) return false;
    if (state->active && state->d3d11Device && state->dcompDevice) {
        if (detailOut) *detailOut = "host_chain_active_d3d11_dcomp_device_ready";
        return true;
    }

    state->activationAttempts += 1;
    std::string detail = "host_chain_activation_unknown_failure";

    HMODULE d3d11 = LoadLibraryW(L"d3d11.dll");
    if (!d3d11) {
        detail = "host_chain_d3d11_load_failed";
        state->activationFailure += 1;
        if (detailOut) *detailOut = detail;
        state->lastDetail = detail;
        return false;
    }
    HMODULE dcomp = LoadLibraryW(L"dcomp.dll");
    if (!dcomp) {
        detail = "host_chain_dcomp_load_failed";
        state->activationFailure += 1;
        if (detailOut) *detailOut = detail;
        state->lastDetail = detail;
        FreeLibrary(d3d11);
        return false;
    }

    const auto pCreateDevice = reinterpret_cast<PfnD3D11CreateDevice>(
        GetProcAddress(d3d11, "D3D11CreateDevice"));
    const auto pDCompCreate = reinterpret_cast<PfnDCompositionCreateDevice>(
        GetProcAddress(dcomp, "DCompositionCreateDevice"));
    if (!pCreateDevice || !pDCompCreate) {
        detail = (!pCreateDevice) ? "host_chain_d3d11_create_device_proc_missing"
                                  : "host_chain_dcomp_create_device_proc_missing";
        state->activationFailure += 1;
        if (detailOut) *detailOut = detail;
        state->lastDetail = detail;
        FreeLibrary(dcomp);
        FreeLibrary(d3d11);
        return false;
    }

    ID3D11Device* d3d11Device = nullptr;
    ID3D11DeviceContext* d3d11Context = nullptr;
    D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_0;
    HRESULT hr = pCreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        nullptr,
        0,
        D3D11_SDK_VERSION,
        &d3d11Device,
        &level,
        &d3d11Context);
    if (FAILED(hr) || !d3d11Device) {
        // Hardware fallback for environments without hardware creation permission.
        hr = pCreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            0,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &d3d11Device,
            &level,
            &d3d11Context);
    }
    if (FAILED(hr) || !d3d11Device) {
        detail = "host_chain_d3d11_device_create_failed";
        state->activationFailure += 1;
        if (d3d11Context) d3d11Context->Release();
        if (d3d11Device) d3d11Device->Release();
        if (detailOut) *detailOut = detail;
        state->lastDetail = detail;
        FreeLibrary(dcomp);
        FreeLibrary(d3d11);
        return false;
    }

    IDCompositionDevice* dcompDevice = nullptr;
    hr = pDCompCreate(
        d3d11Device,
        __uuidof(IDCompositionDevice),
        reinterpret_cast<void**>(&dcompDevice));
    if (FAILED(hr) || !dcompDevice) {
        detail = "host_chain_dcomp_device_create_failed";
        state->activationFailure += 1;
        d3d11Context->Release();
        d3d11Device->Release();
        if (detailOut) *detailOut = detail;
        state->lastDetail = detail;
        FreeLibrary(dcomp);
        FreeLibrary(d3d11);
        return false;
    }

    ReleaseRuntimeState(state, "host_chain_replaced");
    state->d3d11Device = d3d11Device;
    state->d3d11Context = d3d11Context;
    state->dcompDevice = dcompDevice;
    state->active = true;
    state->activationSuccess += 1;
    detail = "host_chain_active_d3d11_dcomp_device_ready";
    state->lastDetail = detail;
    if (detailOut) *detailOut = detail;

    FreeLibrary(dcomp);
    FreeLibrary(d3d11);
    return true;
}

GpuFinalPresentHostChainStatus BuildStatus(bool refresh) {
    static std::mutex s_mutex;
    static GpuFinalPresentHostChainStatus s_cached{};
    static HostChainRuntimeState s_runtime{};

    const uint64_t now = GetTickCount64();
    std::lock_guard<std::mutex> lock(s_mutex);
    if (!refresh &&
        s_cached.probeTickMs != 0 &&
        now >= s_cached.probeTickMs &&
        (now - s_cached.probeTickMs) <= kCacheTtlMs) {
        return s_cached;
    }

    GpuFinalPresentHostChainStatus out{};
    out.probeTickMs = now;
    out.optInEnabled = IsGpuFinalPresentOptInEnabled();
    const GpuFinalPresentCapability cap = GetGpuFinalPresentCapability(refresh);
    out.runtimeCapabilityLikelyAvailable = cap.likelyAvailable;

    if (!out.optInEnabled) {
        ReleaseRuntimeState(&s_runtime, "host_chain_optin_required");
        out.active = false;
        out.readyForActivation = false;
        out.detail = "host_chain_optin_required";
    } else if (!out.runtimeCapabilityLikelyAvailable) {
        ReleaseRuntimeState(&s_runtime, "host_chain_runtime_capability_missing");
        out.active = false;
        out.readyForActivation = false;
        out.detail = "host_chain_runtime_capability_missing";
    } else {
        out.readyForActivation = true;
        std::string detail;
        const bool activated = TryActivateHostChain(&s_runtime, &detail);
        out.active = activated;
        out.detail = detail.empty() ? s_runtime.lastDetail : detail;
    }

    out.activationAttempts = s_runtime.activationAttempts;
    out.activationSuccess = s_runtime.activationSuccess;
    out.activationFailure = s_runtime.activationFailure;
    s_cached = out;
    return s_cached;
}

} // namespace

GpuFinalPresentHostChainStatus GetGpuFinalPresentHostChainStatus(bool refresh) {
    return BuildStatus(refresh);
}

} // namespace mousefx::gpu

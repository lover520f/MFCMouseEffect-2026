#include "pch.h"

#include "D3D11DCompPresenter.h"

#include <filesystem>
#include <fstream>
#include <iterator>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dcomp.lib")

namespace mousefx::gpu {

namespace {
std::filesystem::path ResolveDiagDirFromCurrentModule() {
    wchar_t modulePath[MAX_PATH]{};
    const DWORD n = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
    if (n == 0 || n >= MAX_PATH) return {};
    return std::filesystem::path(modulePath).parent_path() / L".local" / L"diag";
}

void WriteAutoDisableMarker(const char* reason) {
    const std::filesystem::path diagDir = ResolveDiagDirFromCurrentModule();
    if (diagDir.empty()) return;
    std::error_code ec;
    std::filesystem::create_directories(diagDir, ec);
    if (ec) return;

    const std::filesystem::path marker = diagDir / L"gpu_final_present_takeover.off.disabled_by_codex";
    std::ofstream out(marker, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return;
    out << (reason ? reason : "unknown");
}

struct TakeoverControlResult {
    bool enabled = false;
    std::string source = "default_off";
};

bool IsTruthyLeadingChar(wchar_t ch) {
    return ch == L'1' || ch == L't' || ch == L'T' || ch == L'y' || ch == L'Y';
}

TakeoverControlResult ResolveTakeoverControl() {
    TakeoverControlResult result{};
    const std::filesystem::path diagDir = ResolveDiagDirFromCurrentModule();
    if (!diagDir.empty()) {
        const std::filesystem::path offFile = diagDir / L"gpu_final_present_takeover.off";
        const std::filesystem::path onFile = diagDir / L"gpu_final_present_takeover.on";
        const std::filesystem::path autoOffFile = diagDir / L"gpu_final_present_takeover.off.disabled_by_codex";
        std::error_code ec;
        if (std::filesystem::exists(offFile, ec) && !ec) {
            result.enabled = false;
            result.source = "file_off";
            return result;
        }
        ec.clear();
        if (std::filesystem::exists(onFile, ec) && !ec) {
            result.enabled = true;
            result.source = "file_on";
            return result;
        }
        ec.clear();
        if (std::filesystem::exists(autoOffFile, ec) && !ec) {
            result.enabled = false;
            result.source = "file_off_auto";
            return result;
        }
    }

    wchar_t value[8]{};
    const DWORD envN = GetEnvironmentVariableW(L"MOUSEFX_GPU_DCOMP_TAKEOVER", value, static_cast<DWORD>(std::size(value)));
    if (envN == 0 || envN >= std::size(value)) {
        return result;
    }
    result.enabled = IsTruthyLeadingChar(value[0]);
    result.source = "env";
    return result;
}
} // namespace

const wchar_t* D3D11DCompPresenter::ProbeClassName() {
    return L"MouseFxDCompProbeWindow";
}

bool D3D11DCompPresenter::EnsureProbeClassRegistered() {
    static bool registered = false;
    static bool ok = false;
    if (registered) return ok;
    registered = true;
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &D3D11DCompPresenter::ProbeWndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = ProbeClassName();
    ok = (RegisterClassExW(&wc) != 0) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    return ok;
}

LRESULT CALLBACK D3D11DCompPresenter::ProbeWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_NCHITTEST) return HTTRANSPARENT;
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool D3D11DCompPresenter::CreateProbeWindowAndTarget() {
    DestroyProbeWindowAndTarget();
    if (!dcompDevice_) return false;
    if (!EnsureProbeClassRegistered()) {
        status_.detail = "probe_class_register_failed";
        return false;
    }

    probeHwnd_ = CreateWindowExW(
        WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
        ProbeClassName(),
        L"",
        WS_POPUP,
        0,
        0,
        1,
        1,
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        nullptr);
    if (!probeHwnd_) {
        status_.detail = "probe_window_create_failed";
        return false;
    }

    HRESULT hr = dcompDevice_->CreateTargetForHwnd(probeHwnd_, TRUE, dcompTarget_.GetAddressOf());
    if (FAILED(hr) || !dcompTarget_) {
        status_.detail = "dcomp_create_target_failed";
        DestroyProbeWindowAndTarget();
        return false;
    }

    hr = dcompDevice_->CreateVisual(dcompRootVisual_.GetAddressOf());
    if (FAILED(hr) || !dcompRootVisual_) {
        status_.detail = "dcomp_create_visual_failed";
        DestroyProbeWindowAndTarget();
        return false;
    }

    hr = dcompTarget_->SetRoot(dcompRootVisual_.Get());
    if (FAILED(hr)) {
        status_.detail = "dcomp_set_root_failed";
        DestroyProbeWindowAndTarget();
        return false;
    }

    hr = dcompDevice_->Commit();
    if (FAILED(hr)) {
        status_.detail = "dcomp_commit_failed";
        DestroyProbeWindowAndTarget();
        return false;
    }
    return true;
}

void D3D11DCompPresenter::DestroyProbeWindowAndTarget() {
    dcompRootVisual_.Reset();
    dcompTarget_.Reset();
    if (probeHwnd_) {
        DestroyWindow(probeHwnd_);
        probeHwnd_ = nullptr;
    }
}

bool D3D11DCompPresenter::TryActivateTakeoverPath() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!status_.initialized) {
        status_.detail = "takeover_skipped_not_initialized";
        return false;
    }
    if (status_.takeoverActive) {
        return true;
    }
    if (!status_.takeoverEnabled) {
        status_.detail = "takeover_disabled";
        return false;
    }
    if (!status_.takeoverEligible) {
        status_.detail = "takeover_not_eligible";
        return false;
    }
    if (takeoverAttempted_) {
        return false;
    }

    takeoverAttempted_ = true;
    status_.takeoverAttempts += 1;

    // Stage-7 safety policy:
    // keep layered final present as the only visible path until the full DComp presenter is implemented.
    status_.takeoverFallbacks += 1;
    status_.takeoverActive = false;
    status_.takeoverEnabled = false;
    status_.takeoverControl = "runtime_auto_off";
    status_.detail = "takeover_trial_not_implemented_fallback_layered";
    WriteAutoDisableMarker("takeover_trial_not_implemented_fallback_layered");
    return false;
}

bool D3D11DCompPresenter::Initialize() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (status_.initialized) {
        return true;
    }

    status_ = D3D11DCompPresenterStatus{};
    d3d11Device_.Reset();
    d3d11Context_.Reset();
    dxgiDevice_.Reset();
    dcompDevice_.Reset();
    dcompTarget_.Reset();
    dcompRootVisual_.Reset();
    probeHwnd_ = nullptr;
    takeoverAttempted_ = false;
    const TakeoverControlResult control = ResolveTakeoverControl();
    status_.takeoverEnabled = control.enabled;
    status_.takeoverControl = control.source;

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };
    const UINT featureLevelCount = static_cast<UINT>(sizeof(featureLevels) / sizeof(featureLevels[0]));
    D3D_FEATURE_LEVEL chosenLevel = D3D_FEATURE_LEVEL_11_0;
    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        flags,
        featureLevels,
        featureLevelCount,
        D3D11_SDK_VERSION,
        d3d11Device_.GetAddressOf(),
        &chosenLevel,
        d3d11Context_.GetAddressOf());
    if (FAILED(hr)) {
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_WARP,
            nullptr,
            flags,
            featureLevels,
            featureLevelCount,
            D3D11_SDK_VERSION,
            d3d11Device_.GetAddressOf(),
            &chosenLevel,
            d3d11Context_.GetAddressOf());
    }
    (void)chosenLevel;
    if (FAILED(hr) || !d3d11Device_ || !d3d11Context_) {
        status_.detail = "d3d11_create_device_failed";
        return false;
    }
    status_.d3d11DeviceReady = true;

    hr = d3d11Device_.As(&dxgiDevice_);
    if (FAILED(hr) || !dxgiDevice_) {
        status_.detail = "dxgi_device_query_failed";
        return false;
    }

    hr = DCompositionCreateDevice(
        dxgiDevice_.Get(),
        __uuidof(IDCompositionDevice),
        reinterpret_cast<void**>(dcompDevice_.GetAddressOf()));
    if (FAILED(hr) || !dcompDevice_) {
        status_.detail = "dcomp_create_device_failed";
        return false;
    }

    status_.initialized = true;
    status_.dcompDeviceReady = true;
    status_.detail = "d3d11_dcomp_device_ready";

    if (!CreateProbeWindowAndTarget()) {
        return false;
    }
    status_.dcompTargetReady = true;
    status_.takeoverEligible = status_.d3d11DeviceReady && status_.dcompDeviceReady && status_.dcompTargetReady;
    status_.detail = status_.takeoverEnabled ? "dcomp_probe_ready_takeover_enabled" : "dcomp_probe_ready_takeover_disabled";
    return true;
}

void D3D11DCompPresenter::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    DestroyProbeWindowAndTarget();
    dcompDevice_.Reset();
    dxgiDevice_.Reset();
    d3d11Context_.Reset();
    d3d11Device_.Reset();
    takeoverAttempted_ = false;
    status_ = D3D11DCompPresenterStatus{};
    status_.detail = "shutdown";
}

D3D11DCompPresenterStatus D3D11DCompPresenter::GetStatus() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return status_;
}

} // namespace mousefx::gpu

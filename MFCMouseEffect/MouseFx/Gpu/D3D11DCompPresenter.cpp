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

void ArchiveStaleAutoOffMarker(const std::filesystem::path& autoOffFile) {
    if (autoOffFile.empty()) return;
    std::error_code ec;
    if (!std::filesystem::exists(autoOffFile, ec) || ec) return;
    const auto ts = GetTickCount64();
    std::filesystem::path archived = autoOffFile;
    archived += L".stale_ignored_";
    archived += std::to_wstring(ts);
    std::filesystem::rename(autoOffFile, archived, ec);
}

struct TakeoverControlResult {
    bool enabled = false;
    std::string source = "default_off";
    std::string detail = "";
};

bool IsTruthyLeadingChar(wchar_t ch) {
    return ch == L'1' || ch == L't' || ch == L'T' || ch == L'y' || ch == L'Y';
}

TakeoverControlResult ResolveTakeoverControl() {
    TakeoverControlResult result{};
    result.detail = "no_control_file_or_env";
    const std::filesystem::path exePath = []() -> std::filesystem::path {
        wchar_t modulePath[MAX_PATH]{};
        const DWORD n = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
        if (n == 0 || n >= MAX_PATH) return {};
        return std::filesystem::path(modulePath);
    }();
    const std::filesystem::path diagDir = ResolveDiagDirFromCurrentModule();
    if (!diagDir.empty()) {
        const std::filesystem::path offFile = diagDir / L"gpu_final_present_takeover.off";
        const std::filesystem::path onFile = diagDir / L"gpu_final_present_takeover.on";
        const std::filesystem::path autoOffFile = diagDir / L"gpu_final_present_takeover.off.disabled_by_codex";
        std::error_code ec;
        if (std::filesystem::exists(offFile, ec) && !ec) {
            result.enabled = false;
            result.source = "file_off";
            result.detail = "manual_off_file_present";
            return result;
        }
        ec.clear();
        if (std::filesystem::exists(onFile, ec) && !ec) {
            result.enabled = true;
            result.source = "file_on";
            result.detail = "manual_on_file_present";
            return result;
        }
        ec.clear();
        if (std::filesystem::exists(autoOffFile, ec) && !ec) {
            std::error_code fileEc;
            const auto autoOffTime = std::filesystem::last_write_time(autoOffFile, fileEc);
            std::error_code exeEc;
            const auto exeTime = exePath.empty() ? std::filesystem::file_time_type{} : std::filesystem::last_write_time(exePath, exeEc);
            if (!fileEc && !exeEc && !exePath.empty() && autoOffTime < exeTime) {
                ArchiveStaleAutoOffMarker(autoOffFile);
                result.enabled = false;
                result.source = "auto_off_ignored_after_new_build";
                result.detail = "auto_off_marker_older_than_exe_archived";
            } else {
                result.enabled = false;
                result.source = "file_off_auto";
                result.detail = "auto_off_marker_active";
                return result;
            }
        }
    }

    wchar_t value[8]{};
    const DWORD envN = GetEnvironmentVariableW(L"MOUSEFX_GPU_DCOMP_TAKEOVER", value, static_cast<DWORD>(std::size(value)));
    if (envN == 0 || envN >= std::size(value)) {
        return result;
    }
    result.enabled = IsTruthyLeadingChar(value[0]);
    result.source = "env";
    result.detail = result.enabled ? "env_enabled" : "env_disabled";
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

bool D3D11DCompPresenter::CreateProbeCompositionSwapChain() {
    if (!dxgiDevice_ || !dcompRootVisual_ || !dcompDevice_) {
        status_.detail = "takeover_swapchain_missing_prerequisites";
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
    HRESULT hr = dxgiDevice_->GetAdapter(adapter.GetAddressOf());
    if (FAILED(hr) || !adapter) {
        status_.detail = "takeover_swapchain_get_adapter_failed";
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIFactory2> factory2;
    hr = adapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(factory2.GetAddressOf()));
    if (FAILED(hr) || !factory2) {
        status_.detail = "takeover_swapchain_get_factory2_failed";
        return false;
    }

    DXGI_SWAP_CHAIN_DESC1 desc{};
    desc.Width = 1;
    desc.Height = 1;
    desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    desc.Stereo = FALSE;
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    desc.BufferCount = 2;
    desc.Scaling = DXGI_SCALING_STRETCH;
    desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
    desc.Flags = 0;

    compositionSwapChain_.Reset();
    hr = factory2->CreateSwapChainForComposition(
        d3d11Device_.Get(),
        &desc,
        nullptr,
        compositionSwapChain_.GetAddressOf());
    if (FAILED(hr) || !compositionSwapChain_) {
        status_.detail = "takeover_swapchain_create_failed";
        return false;
    }

    hr = dcompRootVisual_->SetContent(compositionSwapChain_.Get());
    if (FAILED(hr)) {
        status_.detail = "takeover_swapchain_set_content_failed";
        compositionSwapChain_.Reset();
        return false;
    }

    hr = dcompDevice_->Commit();
    if (FAILED(hr)) {
        status_.detail = "takeover_swapchain_commit_failed";
        compositionSwapChain_.Reset();
        return false;
    }

    // Probe present once with transparent frame to validate end-to-end composition chain.
    hr = compositionSwapChain_->Present(0, 0);
    if (FAILED(hr)) {
        status_.detail = "takeover_swapchain_present_failed";
        compositionSwapChain_.Reset();
        return false;
    }
    status_.compositionSwapChainReady = true;
    return true;
}

void D3D11DCompPresenter::DestroyProbeWindowAndTarget() {
    compositionSwapChain_.Reset();
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

    if (!CreateProbeCompositionSwapChain()) {
        status_.takeoverFallbacks += 1;
        status_.takeoverActive = false;
        status_.takeoverEnabled = false;
        status_.takeoverControl = "runtime_auto_off";
        status_.takeoverControlDetail = "takeover_trial_swapchain_create_failed";
        WriteAutoDisableMarker("takeover_trial_swapchain_create_failed");
        return false;
    }

    // Stage-11 safety policy:
    // takeover chain is proven on hidden probe path, but visible layered present remains authoritative.
    status_.takeoverFallbacks += 1;
    status_.takeoverActive = false;
    status_.takeoverEnabled = false;
    status_.takeoverControl = "runtime_auto_off";
    status_.takeoverControlDetail = "takeover_trial_swapchain_ready_fallback_layered";
    status_.detail = "takeover_trial_swapchain_ready_fallback_layered";
    WriteAutoDisableMarker("takeover_trial_swapchain_ready_fallback_layered");
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
    compositionSwapChain_.Reset();
    probeHwnd_ = nullptr;
    takeoverAttempted_ = false;
    const TakeoverControlResult control = ResolveTakeoverControl();
    status_.takeoverEnabled = control.enabled;
    status_.takeoverControl = control.source;
    status_.takeoverControlDetail = control.detail;

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

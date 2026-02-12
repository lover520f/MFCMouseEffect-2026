#include "pch.h"

#include "D3D11DCompPresenter.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dcomp.lib")

namespace mousefx::gpu {

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
    return true;
}

void D3D11DCompPresenter::Shutdown() {
    std::lock_guard<std::mutex> lock(mutex_);
    dcompDevice_.Reset();
    dxgiDevice_.Reset();
    d3d11Context_.Reset();
    d3d11Device_.Reset();
    status_ = D3D11DCompPresenterStatus{};
    status_.detail = "shutdown";
}

D3D11DCompPresenterStatus D3D11DCompPresenter::GetStatus() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return status_;
}

} // namespace mousefx::gpu

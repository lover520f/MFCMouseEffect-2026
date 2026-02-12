#pragma once

#include <d3d11.h>
#include <dcomp.h>
#include <dxgi.h>
#include <wrl/client.h>

#include <mutex>
#include <string>

namespace mousefx::gpu {

struct D3D11DCompPresenterStatus {
    bool initialized = false;
    bool d3d11DeviceReady = false;
    bool dcompDeviceReady = false;
    std::string detail = "not_initialized";
};

// Stage-1 scaffold for Windows-native GPU final-present host.
// This class intentionally focuses on device bring-up only.
class D3D11DCompPresenter final {
public:
    D3D11DCompPresenter() = default;
    ~D3D11DCompPresenter() = default;

    bool Initialize();
    void Shutdown();
    D3D11DCompPresenterStatus GetStatus() const;

private:
    mutable std::mutex mutex_{};
    D3D11DCompPresenterStatus status_{};
    Microsoft::WRL::ComPtr<ID3D11Device> d3d11Device_{};
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11Context_{};
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice_{};
    Microsoft::WRL::ComPtr<IDCompositionDevice> dcompDevice_{};
};

} // namespace mousefx::gpu

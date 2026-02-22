#pragma once

#include <d3d11.h>
#include <wrl/client.h>

#include <cstdint>
#include <string>

namespace mousefx {

class FluxFieldGpuV2ComputeEngine final {
public:
    struct Snapshot {
        bool active = false;
        std::string reason = "uninitialized";
        uint64_t tickCount = 0;
        uint32_t lastPasses = 0;
    };

    FluxFieldGpuV2ComputeEngine() = default;
    ~FluxFieldGpuV2ComputeEngine();

    FluxFieldGpuV2ComputeEngine(const FluxFieldGpuV2ComputeEngine&) = delete;
    FluxFieldGpuV2ComputeEngine& operator=(const FluxFieldGpuV2ComputeEngine&) = delete;

    bool Start();
    void Shutdown();
    bool IsActive() const;
    void Tick(uint64_t elapsedMs, uint32_t holdMs);
    Snapshot GetSnapshot() const;

private:
    bool CreateWorkResources();

    void* d3d11Module_ = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Device> device_{};
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_{};
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texA_{};
    Microsoft::WRL::ComPtr<ID3D11Texture2D> texB_{};
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uavA_{};
    Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> uavB_{};

    bool started_ = false;
    bool active_ = false;
    std::string reason_ = "uninitialized";
    uint64_t tickCount_ = 0;
    uint32_t lastPasses_ = 0;
};

} // namespace mousefx

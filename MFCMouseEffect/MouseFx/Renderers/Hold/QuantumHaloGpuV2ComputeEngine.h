#pragma once

#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>

#include <cstdint>
#include <string>

namespace mousefx {

class QuantumHaloGpuV2ComputeEngine final {
public:
    struct Snapshot {
        bool active = false;
        std::string reason = "uninitialized";
        uint64_t tickCount = 0;
        uint32_t lastPasses = 0;
    };

    QuantumHaloGpuV2ComputeEngine() = default;
    ~QuantumHaloGpuV2ComputeEngine() {
        Shutdown();
    }

    QuantumHaloGpuV2ComputeEngine(const QuantumHaloGpuV2ComputeEngine&) = delete;
    QuantumHaloGpuV2ComputeEngine& operator=(const QuantumHaloGpuV2ComputeEngine&) = delete;

    bool Start() {
        if (started_) return active_;
        started_ = true;
        active_ = false;
        reason_ = "unknown";

        d3d11Module_ = LoadLibraryW(L"d3d11.dll");
        if (!d3d11Module_) {
            reason_ = "load_d3d11_dll_failed";
            return false;
        }

        auto* createDevice = reinterpret_cast<PFN_D3D11_CREATE_DEVICE>(
            GetProcAddress(d3d11Module_, "D3D11CreateDevice"));
        if (!createDevice) {
            reason_ = "resolve_d3d11_create_device_failed";
            return false;
        }

        const D3D_FEATURE_LEVEL levels[] = {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
        };
        D3D_FEATURE_LEVEL created = D3D_FEATURE_LEVEL_10_0;
        const UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
        const HRESULT hr = createDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            flags,
            levels,
            static_cast<UINT>(sizeof(levels) / sizeof(levels[0])),
            D3D11_SDK_VERSION,
            device_.GetAddressOf(),
            &created,
            context_.GetAddressOf());
        if (FAILED(hr) || !device_ || !context_) {
            reason_ = "create_hardware_device_failed";
            return false;
        }

        if (!CreateWorkResources()) {
            reason_ = "create_work_resources_failed";
            return false;
        }

        active_ = true;
        reason_ = "hardware_d3d11_uav_ready";
        return true;
    }

    void Shutdown() {
        active_ = false;
        started_ = false;
        uavA_.Reset();
        uavB_.Reset();
        texA_.Reset();
        texB_.Reset();
        context_.Reset();
        device_.Reset();
        if (d3d11Module_) {
            FreeLibrary(d3d11Module_);
            d3d11Module_ = nullptr;
        }
        tickCount_ = 0;
        lastPasses_ = 0;
    }

    bool IsActive() const {
        return active_;
    }

    void Tick(uint64_t elapsedMs, uint32_t holdMs) {
        if (!active_ || !context_ || !uavA_ || !uavB_ || !texA_ || !texB_) return;

        // Keep compute workload lightweight; visual output is handled by D3D11+DComp presenter.
        uint32_t passes = 1u;
        if (holdMs > 0u) {
            passes = 2u;
        }
        if ((elapsedMs / 500u) % 3u == 0u) {
            ++passes;
        }

        const float t = static_cast<float>(elapsedMs % 12000u) / 12000.0f;
        for (uint32_t i = 0; i < passes; ++i) {
            const float phase = t + static_cast<float>(i) * 0.041f;
            float color[4] = {
                0.10f + 0.66f * (1.0f - phase),
                0.12f + 0.42f * phase,
                0.28f + 0.58f * phase,
                1.0f
            };
            context_->ClearUnorderedAccessViewFloat((i & 1u) ? uavA_.Get() : uavB_.Get(), color);
        }

        ++tickCount_;
        lastPasses_ = passes;
    }

    Snapshot GetSnapshot() const {
        Snapshot s{};
        s.active = active_;
        s.reason = reason_;
        s.tickCount = tickCount_;
        s.lastPasses = lastPasses_;
        return s;
    }

private:
    bool CreateWorkResources() {
        D3D11_TEXTURE2D_DESC desc{};
        desc.Width = 256;
        desc.Height = 256;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;

        if (FAILED(device_->CreateTexture2D(&desc, nullptr, texA_.GetAddressOf()))) return false;
        if (FAILED(device_->CreateTexture2D(&desc, nullptr, texB_.GetAddressOf()))) return false;

        D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc{};
        uavDesc.Format = desc.Format;
        uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
        uavDesc.Texture2D.MipSlice = 0;
        if (FAILED(device_->CreateUnorderedAccessView(texA_.Get(), &uavDesc, uavA_.GetAddressOf()))) return false;
        if (FAILED(device_->CreateUnorderedAccessView(texB_.Get(), &uavDesc, uavB_.GetAddressOf()))) return false;

        return true;
    }

    HMODULE d3d11Module_ = nullptr;
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

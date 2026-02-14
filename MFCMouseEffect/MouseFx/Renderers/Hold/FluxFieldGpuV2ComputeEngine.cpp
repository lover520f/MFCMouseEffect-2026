#include "pch.h"
#include "FluxFieldGpuV2ComputeEngine.h"

namespace mousefx {

FluxFieldGpuV2ComputeEngine::~FluxFieldGpuV2ComputeEngine() {
    Shutdown();
}

bool FluxFieldGpuV2ComputeEngine::Start() {
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

void FluxFieldGpuV2ComputeEngine::Shutdown() {
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

bool FluxFieldGpuV2ComputeEngine::IsActive() const {
    return active_;
}

void FluxFieldGpuV2ComputeEngine::Tick(uint64_t elapsedMs, uint32_t holdMs) {
    if (!active_ || !context_ || !uavA_ || !uavB_ || !texA_ || !texB_) return;

    uint32_t passes = 8u;
    if (holdMs > 0) {
        passes += (holdMs / 120u);
    } else {
        passes += static_cast<uint32_t>((elapsedMs / 200u) % 6u);
    }
    if (passes > 30u) passes = 30u;

    const float t = static_cast<float>(elapsedMs % 10000u) / 10000.0f;
    for (uint32_t i = 0; i < passes; ++i) {
        const float phase = t + (static_cast<float>(i) * 0.037f);
        float colorA[4] = {
            0.08f + 0.72f * phase,
            0.20f + 0.60f * (1.0f - phase),
            0.35f + 0.45f * phase,
            1.0f
        };
        float colorB[4] = {
            0.12f + 0.62f * (1.0f - phase),
            0.16f + 0.55f * phase,
            0.28f + 0.50f * (1.0f - phase),
            1.0f
        };
        context_->ClearUnorderedAccessViewFloat((i & 1u) ? uavA_.Get() : uavB_.Get(), colorA);
        context_->ClearUnorderedAccessViewFloat((i & 1u) ? uavB_.Get() : uavA_.Get(), colorB);
        context_->CopyResource((i & 1u) ? texB_.Get() : texA_.Get(), (i & 1u) ? texA_.Get() : texB_.Get());
    }

    ++tickCount_;
    lastPasses_ = passes;
    if ((tickCount_ & 1ull) == 0ull) {
        context_->Flush();
    }
}

FluxFieldGpuV2ComputeEngine::Snapshot FluxFieldGpuV2ComputeEngine::GetSnapshot() const {
    Snapshot s{};
    s.active = active_;
    s.reason = reason_;
    s.tickCount = tickCount_;
    s.lastPasses = lastPasses_;
    return s;
}

bool FluxFieldGpuV2ComputeEngine::CreateWorkResources() {
    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = 1024;
    desc.Height = 1024;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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

} // namespace mousefx

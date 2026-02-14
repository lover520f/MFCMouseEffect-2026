#pragma once

#include "MouseFx/Styles/RippleStyle.h"

#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include <cstdint>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace mousefx {

class QuantumHaloGpuV2ShaderPipeline final {
public:
    QuantumHaloGpuV2ShaderPipeline() = default;
    ~QuantumHaloGpuV2ShaderPipeline();

    QuantumHaloGpuV2ShaderPipeline(const QuantumHaloGpuV2ShaderPipeline&) = delete;
    QuantumHaloGpuV2ShaderPipeline& operator=(const QuantumHaloGpuV2ShaderPipeline&) = delete;

    bool Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Shutdown();
    bool BindSwapChain(IDXGISwapChain1* swapChain, UINT w, UINT h);
    bool Render(uint64_t elapsedMs, uint32_t holdMs, int sizePx, const RippleStyle& style);
    const std::string& LastErrorReason() const;

private:
    struct alignas(16) ShaderConstants {
        float resolutionX = 0.0f;
        float resolutionY = 0.0f;
        float centerX = 0.0f;
        float centerY = 0.0f;
        float timeSec = 0.0f;
        float progress = 0.0f;
        float baseAlpha = 0.0f;
        float baseRadius = 0.0f;
        float strokeWidth = 0.0f;
        float _pad0 = 0.0f;
        float primaryR = 0.0f;
        float primaryG = 0.0f;
        float primaryB = 0.0f;
        float _pad1 = 0.0f;
        float cyanR = 0.0f;
        float cyanG = 0.0f;
        float cyanB = 0.0f;
        float _pad2 = 0.0f;
    };

    void SetError(const std::string& reason);
    bool EnsureShaders();

    std::string lastErrorReason_{};
    UINT targetW_ = 0;
    UINT targetH_ = 0;

    Microsoft::WRL::ComPtr<ID3D11Device> device_{};
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_{};
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain_{};
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv_{};
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader_{};
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader_{};
    Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer_{};
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> rasterizer_{};
};

} // namespace mousefx

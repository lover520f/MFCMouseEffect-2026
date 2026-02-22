#pragma once

#include "MouseFx/Renderers/Hold/QuantumHaloGpuV2ShaderPipeline.h"
#include "MouseFx/Styles/RippleStyle.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <dcomp.h>
#include <wrl/client.h>

#include <cstdint>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dcomp.lib")

namespace mousefx {

class QuantumHaloGpuV2Presenter final {
public:
    QuantumHaloGpuV2Presenter() = default;
    ~QuantumHaloGpuV2Presenter();

    QuantumHaloGpuV2Presenter(const QuantumHaloGpuV2Presenter&) = delete;
    QuantumHaloGpuV2Presenter& operator=(const QuantumHaloGpuV2Presenter&) = delete;

    bool Start();
    void Shutdown();
    bool IsReady() const;
    const std::string& LastErrorReason() const;

    bool RenderFrame(
        int cursorScreenX,
        int cursorScreenY,
        int sizePx,
        float t,
        uint64_t elapsedMs,
        uint32_t holdMs,
        const RippleStyle& style);

private:
    static constexpr wchar_t kWindowClassName[] = L"MouseFxQuantumHaloGpuV2OverlayWindow";

    static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool EnsureWindow();
    bool EnsureDevice();
    bool EnsureSwapChainTarget(UINT w, UINT h);
    bool PositionWindow(int cursorX, int cursorY, int w, int h);

    static bool TryGpuSubmitSeh(
        QuantumHaloGpuV2ShaderPipeline* pipeline,
        IDXGISwapChain1* swapChain,
        IDCompositionDevice* dcompDevice,
        uint64_t elapsedMs,
        uint32_t holdMs,
        int clampedSize,
        const RippleStyle* style,
        unsigned long* outSehCode,
        bool* outPipelineOk,
        HRESULT* outPresentHr,
        HRESULT* outCommitHr);

    static void PickBestHardwareAdapter(Microsoft::WRL::ComPtr<IDXGIAdapter1>& outAdapter);

    void SetError(const std::string& reason);
    void MarkFailure();

    bool started_ = false;
    bool ready_ = false;
    int failureCount_ = 0;
    std::string lastErrorReason_{};

    HWND hwnd_ = nullptr;
    UINT targetW_ = 0;
    UINT targetH_ = 0;
    uint64_t lastPresentedElapsedMs_ = UINT64_MAX;
    uint32_t lastPresentedHoldMs_ = UINT32_MAX;

    QuantumHaloGpuV2ShaderPipeline pipeline_{};
    Microsoft::WRL::ComPtr<ID3D11Device> device_{};
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> context_{};
    Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain_{};
    Microsoft::WRL::ComPtr<IDCompositionDevice> dcompDevice_{};
    Microsoft::WRL::ComPtr<IDCompositionTarget> dcompTarget_{};
    Microsoft::WRL::ComPtr<IDCompositionVisual> dcompVisual_{};
};

} // namespace mousefx

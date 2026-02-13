#pragma once

#include <d3d11.h>
#include <dcomp.h>
#include <dxgi.h>
#include <wrl/client.h>

#include <cstdint>
#include <mutex>
#include <string>

namespace mousefx::gpu {

struct D3D11DCompPresenterStatus {
    bool initialized = false;
    bool d3d11DeviceReady = false;
    bool dcompDeviceReady = false;
    bool dcompTargetReady = false;
    bool compositionSwapChainReady = false;
    bool visibleTrialEnabled = false;
    bool visibleTrialReady = false;
    bool rearmProcessed = false;
    bool controlOnFilePresent = false;
    bool controlOffFilePresent = false;
    bool controlAutoOffFilePresent = false;
    bool controlVisibleTrialFilePresent = false;
    bool controlOnceFilePresent = false;
    bool controlOnceFileConsumed = false;
    bool takeoverEnabled = false;
    bool takeoverEligible = false;
    bool takeoverActive = false;
    uint32_t takeoverAttempts = 0;
    uint32_t takeoverFallbacks = 0;
    uint32_t trialFrameSubmitAttempts = 0;
    uint32_t trialFrameSubmitSuccess = 0;
    uint32_t trialFrameSubmitFailure = 0;
    uint32_t trialFrameSubmitSkippedDisabled = 0;
    uint32_t trialFrameSubmitSkippedNotReady = 0;
    uint64_t lastTrialTickMs = 0;
    std::string lastTrialResult = "none";
    std::string takeoverControl = "default_off";
    std::string takeoverControlDetail = "";
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
    bool ShouldAttemptTakeover() const;
    void RecordTakeoverNotAttempted(const char* reason);
    bool IsTrialFrameUploadEnabled() const;
    bool SubmitTrialFrameBGRAIfEnabled(const void* pixels, int width, int height, int strideBytes);
    void SetVisibleTrialHwnd(HWND hwnd);
    bool TryActivateTakeoverPath();
    bool SubmitTrialFrameBGRA(const void* pixels, int width, int height, int strideBytes);

private:
    static const wchar_t* ProbeClassName();
    static bool EnsureProbeClassRegistered();
    bool CreateProbeWindowAndTarget();
    bool CreateProbeCompositionSwapChain();
    bool TryPrepareVisibleTrialTarget();
    bool SubmitTrialFrameBGRAUnlocked(const void* pixels, int width, int height, int strideBytes);
    void DestroyProbeWindowAndTarget();
    static LRESULT CALLBACK ProbeWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    mutable std::mutex mutex_{};
    D3D11DCompPresenterStatus status_{};
    Microsoft::WRL::ComPtr<ID3D11Device> d3d11Device_{};
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> d3d11Context_{};
    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice_{};
    Microsoft::WRL::ComPtr<IDCompositionDevice> dcompDevice_{};
    Microsoft::WRL::ComPtr<IDCompositionTarget> dcompTarget_{};
    Microsoft::WRL::ComPtr<IDCompositionVisual> dcompRootVisual_{};
    Microsoft::WRL::ComPtr<IDXGISwapChain1> compositionSwapChain_{};
    Microsoft::WRL::ComPtr<IDCompositionTarget> visibleTrialTarget_{};
    Microsoft::WRL::ComPtr<IDCompositionVisual> visibleTrialRootVisual_{};
    int compositionWidth_ = 0;
    int compositionHeight_ = 0;
    HWND probeHwnd_ = nullptr;
    HWND visibleTrialHwnd_ = nullptr;
    bool takeoverAttempted_ = false;
};

} // namespace mousefx::gpu

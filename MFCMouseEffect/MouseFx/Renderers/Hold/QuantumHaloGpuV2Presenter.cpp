#include "pch.h"
#include "QuantumHaloGpuV2Presenter.h"
#include "MouseFx/Utils/MathUtils.h"

#include <sstream>

namespace mousefx {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string Hex32(unsigned long v) {
    std::ostringstream ss;
    ss << "0x" << std::uppercase << std::hex << v;
    return ss.str();
}

static std::string HexHr(HRESULT hr) {
    return Hex32(static_cast<unsigned long>(hr));
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

QuantumHaloGpuV2Presenter::~QuantumHaloGpuV2Presenter() {
    Shutdown();
}

bool QuantumHaloGpuV2Presenter::Start() {
    if (started_) return ready_;
    started_ = true;
    ready_ = false;
    failureCount_ = 0;
    lastErrorReason_.clear();

    if (!EnsureWindow()) {
        MarkFailure();
        return false;
    }
    if (!EnsureDevice()) {
        MarkFailure();
        return false;
    }
    ready_ = true;
    return true;
}

void QuantumHaloGpuV2Presenter::Shutdown() {
    ready_ = false;
    started_ = false;
    pipeline_.Shutdown();
    swapChain_.Reset();
    dcompVisual_.Reset();
    dcompTarget_.Reset();
    dcompDevice_.Reset();
    context_.Reset();
    device_.Reset();
    if (hwnd_) {
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
}

bool QuantumHaloGpuV2Presenter::IsReady() const {
    return ready_;
}

const std::string& QuantumHaloGpuV2Presenter::LastErrorReason() const {
    return lastErrorReason_;
}

// ---------------------------------------------------------------------------
// Render
// ---------------------------------------------------------------------------

bool QuantumHaloGpuV2Presenter::RenderFrame(
    int cursorScreenX,
    int cursorScreenY,
    int sizePx,
    float t,
    uint64_t elapsedMs,
    uint32_t holdMs,
    const RippleStyle& style) {
    (void)t;
    if (!ready_) return false;
    if (!hwnd_ || !device_ || !context_ || !swapChain_) {
        if (!EnsureSwapChainTarget(ClampInt(sizePx, 96, 640), ClampInt(sizePx, 96, 640))) {
            if (lastErrorReason_.empty()) SetError("render_prereq_missing");
            MarkFailure();
            return false;
        }
    }

    const int clampedSize = ClampInt(sizePx, 96, 640);
    if (!EnsureSwapChainTarget(clampedSize, clampedSize)) {
        if (lastErrorReason_.empty()) SetError("ensure_swapchain_target_failed");
        MarkFailure();
        return false;
    }
    if (!PositionWindow(cursorScreenX, cursorScreenY, clampedSize, clampedSize)) {
        if (lastErrorReason_.empty()) {
            SetError("position_window_failed_" + Hex32(GetLastError()));
        }
        MarkFailure();
        return false;
    }

    const bool shouldPresent = (lastPresentedElapsedMs_ != elapsedMs) || (lastPresentedHoldMs_ != holdMs);
    if (!shouldPresent) {
        return true;
    }

    unsigned long sehCode = 0ul;
    bool pipelineOk = false;
    HRESULT presentHr = S_OK;
    HRESULT commitHr = S_OK;
    const bool submitOk = TryGpuSubmitSeh(
        &pipeline_,
        swapChain_.Get(),
        dcompDevice_.Get(),
        elapsedMs,
        holdMs,
        clampedSize,
        &style,
        &sehCode,
        &pipelineOk,
        &presentHr,
        &commitHr);
    if (!submitOk) {
        SetError("gpu_driver_seh_" + Hex32(sehCode));
        MarkFailure();
        return false;
    }
    if (!pipelineOk) {
        SetError("pipeline_render_failed_" + pipeline_.LastErrorReason());
        MarkFailure();
        return false;
    }
    if (FAILED(presentHr)) {
        SetError("present_failed_" + HexHr(presentHr));
        MarkFailure();
        return false;
    }
    if (FAILED(commitHr)) {
        SetError("dcomp_commit_failed_" + HexHr(commitHr));
        MarkFailure();
        return false;
    }

    lastPresentedElapsedMs_ = elapsedMs;
    lastPresentedHoldMs_ = holdMs;
    return true;
}

// ---------------------------------------------------------------------------
// Window
// ---------------------------------------------------------------------------

LRESULT CALLBACK QuantumHaloGpuV2Presenter::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    QuantumHaloGpuV2Presenter* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<QuantumHaloGpuV2Presenter*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<QuantumHaloGpuV2Presenter*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (self) return self->WndProc(hwnd, msg, wParam, lParam);
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT QuantumHaloGpuV2Presenter::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_NCHITTEST:
            return HTTRANSPARENT;
        case WM_ERASEBKGND:
            return 1;
        case WM_DESTROY:
            if (hwnd_ == hwnd) hwnd_ = nullptr;
            break;
        default:
            break;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool QuantumHaloGpuV2Presenter::EnsureWindow() {
    if (hwnd_) return true;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = &QuantumHaloGpuV2Presenter::StaticWndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = kWindowClassName;
    if (!RegisterClassExW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        SetError("register_class_failed_" + Hex32(GetLastError()));
        return false;
    }

    const DWORD ex = WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TRANSPARENT;
    hwnd_ = CreateWindowExW(
        ex,
        kWindowClassName,
        L"",
        WS_POPUP,
        0, 0, 64, 64,
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        this);
    if (!hwnd_) {
        SetError("create_window_failed_" + Hex32(GetLastError()));
        return false;
    }

    ShowWindow(hwnd_, SW_SHOWNA);
    return true;
}

// ---------------------------------------------------------------------------
// Device & SwapChain
// ---------------------------------------------------------------------------

bool QuantumHaloGpuV2Presenter::EnsureDevice() {
    if (device_ && context_ && dcompDevice_ && dcompTarget_ && dcompVisual_) {
        return true;
    }

    UINT flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
    D3D_FEATURE_LEVEL level = D3D_FEATURE_LEVEL_11_0;
    const D3D_FEATURE_LEVEL levels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    Microsoft::WRL::ComPtr<IDXGIAdapter1> hardwareAdapter;
    PickBestHardwareAdapter(hardwareAdapter);

    HRESULT hr = E_FAIL;
    if (hardwareAdapter) {
        hr = D3D11CreateDevice(
            hardwareAdapter.Get(),
            D3D_DRIVER_TYPE_UNKNOWN,
            nullptr,
            flags,
            levels,
            static_cast<UINT>(sizeof(levels) / sizeof(levels[0])),
            D3D11_SDK_VERSION,
            device_.GetAddressOf(),
            &level,
            context_.GetAddressOf());
    }
    if (FAILED(hr) || !device_ || !context_) {
        device_.Reset();
        context_.Reset();
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            flags,
            levels,
            static_cast<UINT>(sizeof(levels) / sizeof(levels[0])),
            D3D11_SDK_VERSION,
            device_.GetAddressOf(),
            &level,
            context_.GetAddressOf());
    }
    if (FAILED(hr) || !device_ || !context_) {
        SetError("d3d11_create_device_failed_" + HexHr(hr));
        return false;
    }

    if (!pipeline_.Initialize(device_.Get(), context_.Get())) {
        SetError("pipeline_init_failed_" + pipeline_.LastErrorReason());
        return false;
    }

    Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
    hr = device_.As(&dxgiDevice);
    if (FAILED(hr) || !dxgiDevice) {
        SetError("as_dxgi_device_failed_" + HexHr(hr));
        return false;
    }

    hr = DCompositionCreateDevice(
        dxgiDevice.Get(),
        __uuidof(IDCompositionDevice),
        reinterpret_cast<void**>(dcompDevice_.GetAddressOf()));
    if (FAILED(hr) || !dcompDevice_) {
        SetError("dcomp_create_device_failed_" + HexHr(hr));
        return false;
    }
    hr = dcompDevice_->CreateTargetForHwnd(hwnd_, TRUE, dcompTarget_.GetAddressOf());
    if (FAILED(hr) || !dcompTarget_) {
        SetError("dcomp_create_target_failed_" + HexHr(hr));
        return false;
    }
    hr = dcompDevice_->CreateVisual(dcompVisual_.GetAddressOf());
    if (FAILED(hr) || !dcompVisual_) {
        SetError("dcomp_create_visual_failed_" + HexHr(hr));
        return false;
    }
    return true;
}

bool QuantumHaloGpuV2Presenter::EnsureSwapChainTarget(UINT w, UINT h) {
    if (!device_ || !context_ || !dcompDevice_ || !dcompTarget_ || !dcompVisual_) return false;
    if (w == 0 || h == 0) return false;

    HRESULT hr = S_OK;
    if (!swapChain_) {
        Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
        Microsoft::WRL::ComPtr<IDXGIAdapter> adapter;
        Microsoft::WRL::ComPtr<IDXGIFactory2> factory2;
        hr = device_.As(&dxgiDevice);
        if (FAILED(hr) || !dxgiDevice) {
            SetError("swapchain_as_dxgi_device_failed_" + HexHr(hr));
            return false;
        }
        hr = dxgiDevice->GetAdapter(adapter.GetAddressOf());
        if (FAILED(hr) || !adapter) {
            SetError("swapchain_get_adapter_failed_" + HexHr(hr));
            return false;
        }
        hr = adapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(factory2.GetAddressOf()));
        if (FAILED(hr) || !factory2) {
            SetError("swapchain_get_factory2_failed_" + HexHr(hr));
            return false;
        }

        DXGI_SWAP_CHAIN_DESC1 desc{};
        desc.Width = w;
        desc.Height = h;
        desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_SHADER_INPUT;
        desc.BufferCount = 2;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
        desc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;
        desc.Scaling = DXGI_SCALING_STRETCH;

        hr = factory2->CreateSwapChainForComposition(device_.Get(), &desc, nullptr, swapChain_.GetAddressOf());
        if (FAILED(hr) || !swapChain_) {
            SetError("create_swapchain_for_composition_failed_" + HexHr(hr));
            return false;
        }

        hr = dcompVisual_->SetContent(swapChain_.Get());
        if (FAILED(hr)) {
            SetError("dcomp_set_content_failed_" + HexHr(hr));
            return false;
        }
        hr = dcompTarget_->SetRoot(dcompVisual_.Get());
        if (FAILED(hr)) {
            SetError("dcomp_set_root_failed_" + HexHr(hr));
            return false;
        }
        hr = dcompDevice_->Commit();
        if (FAILED(hr)) {
            SetError("dcomp_commit_initial_failed_" + HexHr(hr));
            return false;
        }
    } else if (w != targetW_ || h != targetH_) {
        hr = swapChain_->ResizeBuffers(2, w, h, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
        if (FAILED(hr)) {
            SetError("swapchain_resize_failed_" + HexHr(hr));
            return false;
        }
    }

    if (!pipeline_.BindSwapChain(swapChain_.Get(), w, h)) {
        SetError("pipeline_bind_swapchain_failed_" + pipeline_.LastErrorReason());
        return false;
    }

    targetW_ = w;
    targetH_ = h;
    return true;
}

bool QuantumHaloGpuV2Presenter::PositionWindow(int cursorX, int cursorY, int w, int h) {
    if (!hwnd_) return false;
    const int left = cursorX - w / 2;
    const int top = cursorY - h / 2;
    const BOOL ok = SetWindowPos(
        hwnd_,
        HWND_TOPMOST,
        left,
        top,
        w,
        h,
        SWP_NOACTIVATE | SWP_SHOWWINDOW | SWP_NOOWNERZORDER);
    return ok == TRUE;
}

// ---------------------------------------------------------------------------
// SEH & Adapter
// ---------------------------------------------------------------------------

bool QuantumHaloGpuV2Presenter::TryGpuSubmitSeh(
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
    HRESULT* outCommitHr) {
    if (outSehCode) *outSehCode = 0ul;
    if (outPipelineOk) *outPipelineOk = false;
    if (outPresentHr) *outPresentHr = E_FAIL;
    if (outCommitHr) *outCommitHr = E_FAIL;
    if (!pipeline || !swapChain || !dcompDevice || !style) {
        return true;
    }

    __try {
        const bool pipelineOk = pipeline->Render(elapsedMs, holdMs, clampedSize, *style);
        if (outPipelineOk) *outPipelineOk = pipelineOk;
        if (!pipelineOk) {
            if (outPresentHr) *outPresentHr = S_OK;
            if (outCommitHr) *outCommitHr = S_OK;
            return true;
        }

        const HRESULT presentHr = swapChain->Present(0, 0);
        const HRESULT commitHr = dcompDevice->Commit();
        if (outPresentHr) *outPresentHr = presentHr;
        if (outCommitHr) *outCommitHr = commitHr;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        if (outSehCode) *outSehCode = static_cast<unsigned long>(GetExceptionCode());
        return false;
    }
}

void QuantumHaloGpuV2Presenter::PickBestHardwareAdapter(Microsoft::WRL::ComPtr<IDXGIAdapter1>& outAdapter) {
    outAdapter.Reset();
    Microsoft::WRL::ComPtr<IDXGIFactory1> factory;
    if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(factory.GetAddressOf()))) || !factory) {
        return;
    }

    SIZE_T bestMem = 0;
    for (UINT i = 0;; ++i) {
        Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
        if (factory->EnumAdapters1(i, adapter.GetAddressOf()) == DXGI_ERROR_NOT_FOUND) {
            break;
        }
        if (!adapter) {
            continue;
        }

        DXGI_ADAPTER_DESC1 desc{};
        if (FAILED(adapter->GetDesc1(&desc))) {
            continue;
        }
        if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) {
            continue;
        }
        if (desc.DedicatedVideoMemory >= bestMem) {
            bestMem = desc.DedicatedVideoMemory;
            outAdapter = adapter;
        }
    }
}

void QuantumHaloGpuV2Presenter::SetError(const std::string& reason) {
    lastErrorReason_ = reason;
}

void QuantumHaloGpuV2Presenter::MarkFailure() {
    ++failureCount_;
    if (failureCount_ >= 3) {
        ready_ = false;
    }
}

} // namespace mousefx

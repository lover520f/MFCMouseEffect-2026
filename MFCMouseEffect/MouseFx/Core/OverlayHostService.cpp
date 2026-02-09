#include "pch.h"

#include "OverlayHostService.h"

#include "MouseFx/Gpu/DawnOverlayBridge.h"
#include "MouseFx/Gpu/DawnRuntime.h"
#include "MouseFx/Gpu/GpuHardwareProbe.h"
#include "MouseFx/Interfaces/IRippleRenderer.h"
#include "MouseFx/Interfaces/ITrailRenderer.h"
#include "MouseFx/Layers/ParticleTrailOverlayLayer.h"
#include "MouseFx/Layers/RippleOverlayLayer.h"
#include "MouseFx/Layers/TextOverlayLayer.h"
#include "MouseFx/Layers/TrailOverlayLayer.h"
#include "MouseFx/Windows/OverlayHostWindow.h"

namespace mousefx {

std::string OverlayHostService::NormalizeRenderBackend(std::string backend) {
    for (char& c : backend) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    if (backend == "dawn") return "dawn";
    if (backend == "cpu") return "cpu";
    return "auto";
}

std::string OverlayHostService::NormalizeGpuBridgeMode(std::string mode) {
    for (char& c : mode) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    if (mode == "compositor") return "compositor";
    return "host_compat";
}

OverlayHostService& OverlayHostService::Instance() {
    static OverlayHostService instance;
    return instance;
}

void OverlayHostService::SetRenderBackendPreference(const std::string& backend) {
    const std::string normalized = NormalizeRenderBackend(backend);
    if (requestedBackend_ == normalized) return;
    requestedBackend_ = normalized;
    RefreshGpuRuntimeProbe();
    Shutdown();
}

std::string OverlayHostService::GetRenderBackendPreference() const {
    return requestedBackend_;
}

std::string OverlayHostService::GetActiveRenderBackend() const {
    return activeBackend_;
}

std::string OverlayHostService::GetRenderBackendDetail() const {
    return backendDetail_;
}

std::string OverlayHostService::GetRenderPipelineMode() const {
    return pipelineMode_;
}

void OverlayHostService::SetGpuBridgeModeRequest(const std::string& mode) {
    const std::string normalized = NormalizeGpuBridgeMode(mode);
    requestedBridgeMode_ = normalized;
    gpu::SetRequestedBridgeMode(normalized);
    RefreshGpuRuntimeProbe();
}

std::string OverlayHostService::GetGpuBridgeModeRequest() const {
    return requestedBridgeMode_;
}

bool OverlayHostService::HasGpuHardware() const {
    return gpu::HasDesktopDisplayAdapter();
}

bool OverlayHostService::IsGpuBackendAvailable(const std::string& backend) const {
    const std::string normalized = NormalizeRenderBackend(backend);
    if (normalized == "dawn") {
        return gpu::IsDawnCompiled();
    }
    if (normalized == "cpu") return true;
    return false;
}

void OverlayHostService::RefreshGpuRuntimeProbe() {
    gpu::ResetDawnRuntimeProbe();
    if (requestedBackend_ == "dawn" || requestedBackend_ == "auto") {
        const gpu::DawnRuntimeInitResult dawn = gpu::TryInitializeDawnRuntime();
        backendDetail_ = dawn.detail;
    }
}

std::string OverlayHostService::ProbeDawnRuntimeNow(bool refreshProbe) {
    if (refreshProbe) {
        gpu::ResetDawnRuntimeProbe();
    }
    const gpu::DawnRuntimeInitResult dawn = gpu::TryInitializeDawnRuntime();
    backendDetail_ = dawn.detail;
    return dawn.detail;
}

uint64_t OverlayHostService::GetLastGpuCommandFrameTickMs() const {
    if (!host_) return 0;
    return host_->GetLastGpuCommandFrameTickMs();
}

uint32_t OverlayHostService::GetLastGpuCommandCount() const {
    if (!host_) return 0;
    return host_->GetLastGpuCommandCount();
}

uint32_t OverlayHostService::GetLastGpuTrailCommandCount() const {
    if (!host_) return 0;
    return host_->GetLastGpuTrailCommandCount();
}

uint32_t OverlayHostService::GetLastGpuRippleCommandCount() const {
    if (!host_) return 0;
    return host_->GetLastGpuRippleCommandCount();
}

uint32_t OverlayHostService::GetLastGpuParticleCommandCount() const {
    if (!host_) return 0;
    return host_->GetLastGpuParticleCommandCount();
}

bool OverlayHostService::Initialize() {
    if (host_) return true;

    const std::string pref = NormalizeRenderBackend(requestedBackend_);
    if (pref == "dawn") {
        const gpu::DawnRuntimeInitResult dawn = gpu::TryInitializeDawnRuntime();
        if (dawn.ok) {
            activeBackend_ = dawn.backend;
            backendDetail_ = dawn.detail;
            const gpu::DawnOverlayBridgeStatus bridge = gpu::GetDawnOverlayBridgeStatus();
            if (bridge.mode == "compositor" && bridge.compositorApisReady) {
                pipelineMode_ = "dawn_compositor";
            } else {
                pipelineMode_ = "dawn_host_compat_layered";
            }
        } else {
            activeBackend_ = "cpu";
            backendDetail_ = dawn.detail;
            pipelineMode_ = "cpu_layered";
        }
    } else if (pref == "auto") {
        const gpu::DawnRuntimeInitResult dawn = gpu::TryInitializeDawnRuntime();
        if (dawn.ok) {
            activeBackend_ = dawn.backend;
            backendDetail_ = dawn.detail;
            const gpu::DawnOverlayBridgeStatus bridge = gpu::GetDawnOverlayBridgeStatus();
            if (bridge.mode == "compositor" && bridge.compositorApisReady) {
                pipelineMode_ = "dawn_compositor";
            } else {
                pipelineMode_ = "dawn_host_compat_layered";
            }
        } else {
            activeBackend_ = "cpu";
            backendDetail_ = dawn.detail;
            pipelineMode_ = "cpu_layered";
        }
    } else {
        activeBackend_ = "cpu";
        backendDetail_ = "cpu_forced";
        pipelineMode_ = "cpu_layered";
    }

    host_ = std::make_unique<OverlayHostWindow>();
    if (!host_->Create()) {
        host_.reset();
        return false;
    }
    return true;
}

void OverlayHostService::Shutdown() {
    if (!host_) return;
    rippleLayer_ = nullptr;
    textLayer_ = nullptr;
    host_->Shutdown();
    host_.reset();
    activeBackend_ = "cpu";
    backendDetail_ = "cpu_default";
    pipelineMode_ = "cpu_layered";
}

IOverlayLayer* OverlayHostService::AttachLayer(std::unique_ptr<IOverlayLayer> layer) {
    if (!layer) return nullptr;
    if (!Initialize()) return nullptr;
    return host_->AddLayer(std::move(layer));
}

TrailOverlayLayer* OverlayHostService::AttachTrailLayer(std::unique_ptr<ITrailRenderer> renderer, int durationMs, int maxPoints, bool isChromatic) {
    if (!renderer) return nullptr;
    auto layer = std::make_unique<TrailOverlayLayer>(
        std::move(renderer),
        durationMs,
        maxPoints,
        Gdiplus::Color(220, 100, 255, 218),
        isChromatic);
    return static_cast<TrailOverlayLayer*>(AttachLayer(std::move(layer)));
}

ParticleTrailOverlayLayer* OverlayHostService::AttachParticleTrailLayer(bool isChromatic) {
    auto layer = std::make_unique<ParticleTrailOverlayLayer>(isChromatic);
    return static_cast<ParticleTrailOverlayLayer*>(AttachLayer(std::move(layer)));
}

uint64_t OverlayHostService::ShowRipple(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params) {
    RippleOverlayLayer* layer = EnsureRippleLayer();
    if (!layer) return 0;
    return layer->ShowRipple(ev, style, std::move(renderer), params);
}

uint64_t OverlayHostService::ShowContinuousRipple(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params) {
    RippleOverlayLayer* layer = EnsureRippleLayer();
    if (!layer) return 0;
    return layer->ShowContinuous(ev, style, std::move(renderer), params);
}

void OverlayHostService::UpdateRipplePosition(uint64_t id, const POINT& pt) {
    if (!rippleLayer_) return;
    rippleLayer_->UpdatePosition(id, pt);
}

void OverlayHostService::StopRipple(uint64_t id) {
    if (!rippleLayer_) return;
    rippleLayer_->Stop(id);
}

bool OverlayHostService::IsRippleActive(uint64_t id) const {
    if (!rippleLayer_) return false;
    return rippleLayer_->IsActive(id);
}

void OverlayHostService::UpdateRippleHoldElapsed(uint64_t id, uint32_t holdMs) {
    if (!rippleLayer_) return;
    rippleLayer_->SendHoldElapsed(id, holdMs);
}

void OverlayHostService::UpdateRippleHoldThreshold(uint64_t id, uint32_t thresholdMs) {
    if (!rippleLayer_) return;
    rippleLayer_->SendHoldThreshold(id, thresholdMs);
}

void OverlayHostService::SendRippleCommand(uint64_t id, const std::string& cmd, const std::string& args) {
    if (!rippleLayer_) return;
    rippleLayer_->SendCommand(id, cmd, args);
}

void OverlayHostService::BroadcastRippleCommand(const std::string& cmd, const std::string& args) {
    if (!rippleLayer_) return;
    rippleLayer_->BroadcastCommand(cmd, args);
}

bool OverlayHostService::ShowText(const POINT& pt, const std::wstring& text, Argb color, const TextConfig& config) {
    TextOverlayLayer* layer = EnsureTextLayer();
    if (!layer) return false;
    layer->ShowText(pt, text, color, config);
    return true;
}

void OverlayHostService::DetachLayer(IOverlayLayer* layer) {
    if (!host_ || !layer) return;
    if (rippleLayer_ == layer) {
        rippleLayer_ = nullptr;
    }
    if (textLayer_ == layer) {
        textLayer_ = nullptr;
    }
    host_->RemoveLayer(layer);
}

RippleOverlayLayer* OverlayHostService::EnsureRippleLayer() {
    if (rippleLayer_) return rippleLayer_;
    auto layer = std::make_unique<RippleOverlayLayer>();
    rippleLayer_ = static_cast<RippleOverlayLayer*>(AttachLayer(std::move(layer)));
    return rippleLayer_;
}

TextOverlayLayer* OverlayHostService::EnsureTextLayer() {
    if (textLayer_) return textLayer_;
    auto layer = std::make_unique<TextOverlayLayer>();
    textLayer_ = static_cast<TextOverlayLayer*>(AttachLayer(std::move(layer)));
    return textLayer_;
}

} // namespace mousefx

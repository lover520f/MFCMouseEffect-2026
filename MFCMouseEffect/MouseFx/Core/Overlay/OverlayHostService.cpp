#include "pch.h"

#include "OverlayHostService.h"

#include "MouseFx/Interfaces/IRippleRenderer.h"
#include "MouseFx/Interfaces/ITrailRenderer.h"
#include "MouseFx/Layers/ParticleTrailOverlayLayer.h"
#include "MouseFx/Layers/RippleOverlayLayer.h"
#include "MouseFx/Layers/TextOverlayLayer.h"
#include "MouseFx/Windows/TextWindowPool.h"
#include "MouseFx/Layers/TrailOverlayLayer.h"
#include "Platform/PlatformOverlayServicesFactory.h"
#include "Settings/EmojiUtils.h"

namespace {

bool HasEmojiStarter(const std::wstring& text) {
    for (size_t i = 0; i < text.size();) {
        const uint32_t cp = settings::NextCodePointUtf16(text, &i);
        if (cp == 0) {
            break;
        }
        if (settings::IsEmojiCodePoint(cp)) {
            return true;
        }
    }
    return false;
}

mousefx::TextWindowPool& SharedTextWindowPool() {
    static mousefx::TextWindowPool pool;
    return pool;
}

} // namespace

namespace mousefx {

OverlayHostService& OverlayHostService::Instance() {
    static OverlayHostService instance;
    return instance;
}

bool OverlayHostService::Initialize() {
    if (hostBackend_) return true;
    hostBackend_ = platform::CreateOverlayHostBackend();
    if (!hostBackend_) {
        return false;
    }
    if (!hostBackend_->Create()) {
        hostBackend_.reset();
        return false;
    }
    return true;
}

void OverlayHostService::Shutdown() {
    if (!hostBackend_) return;
    rippleLayer_ = nullptr;
    textLayer_ = nullptr;
    SharedTextWindowPool().Shutdown();
    hostBackend_->Shutdown();
    hostBackend_.reset();
}

IOverlayLayer* OverlayHostService::AttachLayer(std::unique_ptr<IOverlayLayer> layer) {
    if (!layer) return nullptr;
    if (!Initialize()) return nullptr;
    return hostBackend_->AddLayer(std::move(layer));
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

void OverlayHostService::UpdateRipplePosition(uint64_t id, const ScreenPoint& pt) {
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

void OverlayHostService::SendRippleCommand(uint64_t id, const std::string& cmd, const std::string& args) {
    if (!rippleLayer_) return;
    rippleLayer_->SendCommand(id, cmd, args);
}

void OverlayHostService::BroadcastRippleCommand(const std::string& cmd, const std::string& args) {
    if (!rippleLayer_) return;
    rippleLayer_->BroadcastCommand(cmd, args);
}

bool OverlayHostService::ShowText(const ScreenPoint& pt, const std::wstring& text, Argb color, const TextConfig& config) {
    if (HasEmojiStarter(text)) {
        TextWindowPool& pool = SharedTextWindowPool();
        if (!pool.Initialize(8)) {
            return false;
        }
        pool.ShowText(pt, text, color, config);
        return true;
    }
    TextOverlayLayer* layer = EnsureTextLayer();
    if (!layer) return false;
    layer->ShowText(pt, text, color, config);
    return true;
}

void OverlayHostService::DetachLayer(IOverlayLayer* layer) {
    if (!hostBackend_ || !layer) return;
    if (rippleLayer_ == layer) {
        rippleLayer_ = nullptr;
    }
    if (textLayer_ == layer) {
        textLayer_ = nullptr;
    }
    hostBackend_->RemoveLayer(layer);
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

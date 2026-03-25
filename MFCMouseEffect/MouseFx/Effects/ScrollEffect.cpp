#include "pch.h"
#include "ScrollEffect.h"
#include "MouseFx/Effects/ScrollEffectCommandAdapter.h"
#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Utils/TimeUtils.h"
#include "MouseFx/Renderers/RendererRegistry.h"

// Ensure standard renderers are linked
#include "MouseFx/Renderers/Scroll/ChevronRenderer.h"
#include "MouseFx/Renderers/Scroll/HelixRenderer.h"
#include "MouseFx/Renderers/Scroll/TwinkleRenderer.h"

namespace mousefx {

ScrollEffect::ScrollEffect(const EffectConfig& config, const std::string& rendererName)
    : currentRendererName_(NormalizeScrollEffectType(rendererName)) {
    style_ = GetThemePalette(config.theme).scroll;
    sizeScalePercent_ = std::clamp(config.effectSizeScales.scroll, 50, 200);
    computeProfile_ = BuildScrollEffectProfileFromStyle(
        style_,
        sizeScalePercent_);
    isChromatic_ = (ToLowerAscii(config.theme) == "chromatic");
}

ScrollEffect::~ScrollEffect() {
    Shutdown();
}

bool ScrollEffect::Initialize() {
    return true;
}

void ScrollEffect::Shutdown() {
}

void ScrollEffect::OnCommand(const std::string& cmd, const std::string& args) {
    if (cmd == "type") {
        currentRendererName_ = NormalizeScrollEffectType(args);
        lastEmitTickMs_ = 0;
        pendingDelta_ = 0;
        activeRippleIds_.clear();
    }
}

void ScrollEffect::PruneInactiveRipples(size_t maxActive) {
    if (activeRippleIds_.empty()) return;
    auto& host = OverlayHostService::Instance();

    while (!activeRippleIds_.empty() && !host.IsRippleActive(activeRippleIds_.front())) {
        activeRippleIds_.pop_front();
    }
    while (activeRippleIds_.size() > maxActive) {
        const uint64_t id = activeRippleIds_.front();
        activeRippleIds_.pop_front();
        host.StopRipple(id);
    }
}

void ScrollEffect::OnScroll(const ScrollEvent& event) {
    const ScrollEffectInputShaperProfile shaper = ResolveScrollInputShaperProfile(currentRendererName_);
    pendingDelta_ += event.delta;

    const uint64_t now = NowMs();
    if (lastEmitTickMs_ != 0 && (now - lastEmitTickMs_) < shaper.emitIntervalMs) {
        PruneInactiveRipples(shaper.maxActiveRipples);
        return;
    }

    int effectiveDelta = event.delta;
    lastEmitTickMs_ = now;
    if (pendingDelta_ != 0) {
        effectiveDelta = pendingDelta_;
        pendingDelta_ = 0;
    }

    const RippleStyle runtimeStyle = isChromatic_ ? MakeRandomStyle(style_) : style_;
    const ScrollEffectProfile runtimeProfile = isChromatic_
        ? BuildScrollEffectProfileFromStyle(runtimeStyle, sizeScalePercent_)
        : computeProfile_;
    const ScrollEffectRenderCommand command = ComputeScrollEffectRenderCommand(
        event.pt,
        event.horizontal,
        effectiveDelta,
        currentRendererName_,
        runtimeProfile);
    if (!command.emit) {
        PruneInactiveRipples(shaper.maxActiveRipples);
        return;
    }

    auto renderer = RendererRegistry::Instance().Create(scroll_effect_adapter::ResolveRendererName(command));
    if (!renderer) {
        return;
    }

    const ClickEvent renderEvent = scroll_effect_adapter::BuildClickEventFromCommand(event, command);
    const RenderParams params = scroll_effect_adapter::BuildRenderParamsFromCommand(command);
    const RippleStyle renderStyle = scroll_effect_adapter::BuildRippleStyleFromCommand(runtimeStyle, command);
    renderer->SetParams(params);

    const uint64_t rippleId = OverlayHostService::Instance().ShowRipple(
        renderEvent,
        renderStyle,
        std::move(renderer),
        params);
    if (rippleId != 0) {
        activeRippleIds_.push_back(rippleId);
    }
    PruneInactiveRipples(shaper.maxActiveRipples);
}

} // namespace mousefx

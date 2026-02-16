#include "pch.h"
#include "ScrollEffect.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Utils/TimeUtils.h"
#include <algorithm>
#include "MouseFx/Renderers/RendererRegistry.h"

// Ensure standard renderers are linked
#include "MouseFx/Renderers/Scroll/ChevronRenderer.h"
#include "MouseFx/Renderers/Scroll/HelixRenderer.h"

namespace mousefx {

static float Clamp01(float v) {
    return std::max(0.0f, std::min(1.0f, v));
}


ScrollEffect::ScrollEffect(const std::string& themeName, const std::string& rendererName) 
    : currentRendererName_(rendererName) {
    style_ = GetThemePalette(themeName).scroll;
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
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
        currentRendererName_ = args;
        lastEmitTickMs_ = 0;
        pendingDelta_ = 0;
        activeRippleIds_.clear();
    }
}

bool ScrollEffect::IsHelixRenderer() const {
    return ToLowerAscii(currentRendererName_) == "helix";
}

ScrollEffect::InputShaperProfile ScrollEffect::GetInputShaperProfile() const {
    InputShaperProfile profile{};
    if (IsHelixRenderer()) {
        profile.emitIntervalMs = kHelixEmitIntervalMs;
        profile.maxActiveRipples = kHelixMaxActiveRipples;
        profile.maxDurationMs = kHelixMaxDurationMs;
    }
    return profile;
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
    const InputShaperProfile shaper = GetInputShaperProfile();
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

    auto renderer = RendererRegistry::Instance().Create(currentRendererName_);
    if (!renderer) {
        if (currentRendererName_ != "none") {
             renderer = RendererRegistry::Instance().Create("arrow");
        }
    }
    
    if (!renderer) return;

    ClickEvent ev{};
    ev.pt = event.pt;
    ev.button = MouseButton::Left;

    RenderParams params;
    const float base = (effectiveDelta >= 0) ? -3.1415926f / 2.0f : 3.1415926f / 2.0f;
    if (event.horizontal) {
        params.directionRad = (effectiveDelta >= 0) ? 0.0f : 3.1415926f;
    } else {
        params.directionRad = base;
    }
    const float strength = (float)std::abs(effectiveDelta) / 120.0f;
    params.intensity = Clamp01(0.6f + strength * 0.6f);
    params.loop = false;
    renderer->SetParams(params);

    RippleStyle finalStyle = style_;
    if (isChromatic_) {
        finalStyle = MakeRandomStyle(style_);
    }
    finalStyle.durationMs = std::min<uint32_t>(finalStyle.durationMs, shaper.maxDurationMs);

    const uint64_t rippleId = OverlayHostService::Instance().ShowRipple(ev, finalStyle, std::move(renderer), params);
    if (rippleId != 0) {
        activeRippleIds_.push_back(rippleId);
        PruneInactiveRipples(shaper.maxActiveRipples);
    }
}

} // namespace mousefx

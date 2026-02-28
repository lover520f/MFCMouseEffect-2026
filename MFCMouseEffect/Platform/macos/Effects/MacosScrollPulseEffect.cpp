#include "pch.h"

#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "Platform/macos/Effects/MacosScrollPulseEffect.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayRenderer.h"

#include <chrono>
#include <utility>

namespace mousefx {
MacosScrollPulseEffect::MacosScrollPulseEffect(
    std::string effectType,
    std::string themeName,
    macos_effect_profile::ScrollRenderProfile renderProfile)
    : effectType_(std::move(effectType)),
      themeName_(std::move(themeName)),
      renderProfile_(renderProfile) {
    effectType_ = NormalizeScrollEffectType(effectType_);
}

MacosScrollPulseEffect::~MacosScrollPulseEffect() {
    Shutdown();
}

bool MacosScrollPulseEffect::Initialize() {
    initialized_ = true;
    lastEmitTickMs_ = 0;
    pendingDelta_ = 0;
    return true;
}

void MacosScrollPulseEffect::Shutdown() {
    initialized_ = false;
    lastEmitTickMs_ = 0;
    pendingDelta_ = 0;
    macos_scroll_pulse::CloseAllScrollPulseWindows();
}

void MacosScrollPulseEffect::OnScroll(const ScrollEvent& event) {
    if (!initialized_ || event.delta == 0) {
        return;
    }

    const ScrollEffectInputShaperProfile shaper = ResolveScrollInputShaperProfile(effectType_);
    pendingDelta_ += event.delta;
    const uint64_t now = CurrentTickMs();
    if (lastEmitTickMs_ != 0 && (now - lastEmitTickMs_) < shaper.emitIntervalMs) {
        return;
    }
    lastEmitTickMs_ = now;
    int effectiveDelta = event.delta;
    if (pendingDelta_ != 0) {
        effectiveDelta = pendingDelta_;
        pendingDelta_ = 0;
    }

    const ScrollEffectRenderCommand command = ComputeScrollEffectRenderCommand(
        ScreenToOverlayPoint(event.pt),
        event.horizontal,
        effectiveDelta,
        effectType_,
        macos_effect_compute_profile::BuildScrollProfile(renderProfile_));
    macos_scroll_pulse::ShowScrollPulseOverlay(command, themeName_);
}

uint64_t MacosScrollPulseEffect::CurrentTickMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

} // namespace mousefx

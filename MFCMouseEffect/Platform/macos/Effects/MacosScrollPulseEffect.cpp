#include "pch.h"

#include "MouseFx/Core/Effects/ScrollEffectCompute.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Utils/StringUtils.h"
#include "Platform/macos/Effects/MacosScrollPulseEffect.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "Platform/macos/Effects/MacosScrollPulseOverlayRenderer.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <utility>

namespace mousefx {
MacosScrollPulseEffect::MacosScrollPulseEffect(
    std::string effectType,
    std::string themeName,
    int sizeScalePercent)
    : effectType_(std::move(effectType)),
      themeName_(std::move(themeName)),
      sizeScalePercent_(std::clamp(sizeScalePercent, 50, 200)) {
    effectType_ = NormalizeScrollEffectType(effectType_);
    style_ = GetThemePalette(themeName_).scroll;
    computeProfile_ = BuildScrollEffectProfileFromStyle(style_, sizeScalePercent_);
    isChromatic_ = (ToLowerAscii(themeName_) == "chromatic");
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

    const RippleStyle runtimeStyle = isChromatic_ ? MakeRandomStyle(style_) : style_;
    const ScrollEffectProfile runtimeProfile = isChromatic_
        ? BuildScrollEffectProfileFromStyle(runtimeStyle, sizeScalePercent_)
        : computeProfile_;
    const ScrollEffectRenderCommand command = ComputeScrollEffectRenderCommand(
        ScreenToOverlayPoint(event.pt),
        event.horizontal,
        effectiveDelta,
        effectType_,
        runtimeProfile);
    macos_scroll_pulse::ShowScrollPulseOverlay(command, themeName_);
}

uint64_t MacosScrollPulseEffect::CurrentTickMs() {
    using namespace std::chrono;
    return duration_cast<milliseconds>(steady_clock::now().time_since_epoch()).count();
}

} // namespace mousefx

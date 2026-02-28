#include "pch.h"

#include "Platform/macos/Effects/MacosClickPulseEffect.h"
#include "MouseFx/Core/Effects/ClickEffectCompute.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"
#include "Platform/macos/Effects/MacosClickPulseOverlayRenderer.h"
#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"

#include <utility>

namespace mousefx {

MacosClickPulseEffect::MacosClickPulseEffect(
    std::string effectType,
    std::string themeName,
    macos_effect_profile::ClickRenderProfile renderProfile)
    : effectType_(std::move(effectType)),
      themeName_(std::move(themeName)),
      renderProfile_(renderProfile) {
    effectType_ = NormalizeClickEffectType(effectType_);
}

MacosClickPulseEffect::~MacosClickPulseEffect() {
    Shutdown();
}

bool MacosClickPulseEffect::Initialize() {
    initialized_ = true;
    return true;
}

void MacosClickPulseEffect::Shutdown() {
    initialized_ = false;
    macos_click_pulse::CloseAllClickPulseWindows();
}

void MacosClickPulseEffect::OnClick(const ClickEvent& event) {
    if (!initialized_) {
        return;
    }
    const ClickEffectRenderCommand command = ComputeClickEffectRenderCommand(
        ScreenToOverlayPoint(event.pt),
        event.button,
        effectType_,
        macos_effect_compute_profile::BuildClickProfile(renderProfile_));
    macos_click_pulse::ShowClickPulseOverlay(command, themeName_);
}

} // namespace mousefx

#include "pch.h"

#include "MouseFx/Core/Effects/HoverEffectCompute.h"
#include "Platform/macos/Effects/MacosHoverPulseEffect.h"
#include "Platform/macos/Effects/MacosEffectComputeProfileAdapter.h"

#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "Platform/macos/Effects/MacosHoverPulseOverlayRenderer.h"

#include <utility>

namespace mousefx {
MacosHoverPulseEffect::MacosHoverPulseEffect(
    std::string effectType,
    std::string themeName,
    macos_effect_profile::HoverRenderProfile renderProfile)
    : effectType_(std::move(effectType)),
      themeName_(std::move(themeName)),
      renderProfile_(renderProfile) {
    effectType_ = NormalizeHoverEffectType(effectType_);
}

MacosHoverPulseEffect::~MacosHoverPulseEffect() {
    Shutdown();
}

bool MacosHoverPulseEffect::Initialize() {
    initialized_ = true;
    return true;
}

void MacosHoverPulseEffect::Shutdown() {
    initialized_ = false;
    macos_hover_pulse::CloseHoverPulseOverlay();
}

void MacosHoverPulseEffect::OnHoverStart(const ScreenPoint& pt) {
    if (!initialized_) {
        return;
    }
    const HoverEffectRenderCommand command =
        ComputeHoverEffectRenderCommand(
            ScreenToOverlayPoint(pt),
            effectType_,
            macos_effect_compute_profile::BuildHoverProfile(renderProfile_));
    macos_hover_pulse::ShowHoverPulseOverlay(command, themeName_);
}

void MacosHoverPulseEffect::OnHoverEnd() {
    macos_hover_pulse::CloseHoverPulseOverlay();
}

} // namespace mousefx

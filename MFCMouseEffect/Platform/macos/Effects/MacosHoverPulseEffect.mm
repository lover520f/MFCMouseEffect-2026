#include "pch.h"

#include "Platform/macos/Effects/MacosHoverPulseEffect.h"

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
    if (effectType_.empty()) {
        effectType_ = "glow";
    }
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
    macos_hover_pulse::ShowHoverPulseOverlay(ScreenToOverlayPoint(pt), effectType_, themeName_, renderProfile_);
}

void MacosHoverPulseEffect::OnHoverEnd() {
    macos_hover_pulse::CloseHoverPulseOverlay();
}

} // namespace mousefx

#include "pch.h"

#include "Platform/macos/Effects/MacosHoldPulseOverlayStyle.Internal.h"
#include "Platform/macos/Effects/MacosHoldPulseOverlayStyle.h"
#include "MouseFx/Effects/HoldRouteCatalog.h"
#include "MouseFx/Utils/StringUtils.h"

#if defined(__APPLE__)
#include <cmath>
#include <utility>
#endif

namespace mousefx::macos_hold_pulse::detail {

#if defined(__APPLE__)
std::string NormalizeHoldType(const std::string& effectType) {
    const std::string value = ToLowerAscii(effectType);
    if (value.empty()) {
        return "charge";
    }
    return value;
}

bool ContainsHoldToken(const std::string& holdType, const char* token) {
    return holdType.find(token) != std::string::npos;
}

HoldStyle ResolveHoldStyle(const std::string& holdType) {
    if (ContainsHoldToken(holdType, "hold_quantum_halo_gpu_v2") ||
        ContainsHoldToken(holdType, "hold_neon3d_gpu_v2") ||
        ContainsHoldToken(holdType, "quantum_halo")) {
        return HoldStyle::QuantumHalo;
    }
    if (ContainsHoldToken(holdType, mousefx::hold_route::kTypeFluxFieldGpuV2) ||
        ContainsHoldToken(holdType, mousefx::hold_route::kTypeFluxFieldCpu) ||
        ContainsHoldToken(holdType, "fluxfield") ||
        ContainsHoldToken(holdType, "flux_field")) {
        return HoldStyle::FluxField;
    }
    if (ContainsHoldToken(holdType, "scifi3d")) {
        return HoldStyle::Hologram;
    }
    if (holdType.find("lightning") != std::string::npos) {
        return HoldStyle::Lightning;
    }
    if (holdType.find("hex") != std::string::npos) {
        return HoldStyle::Hex;
    }
    if (holdType.find("hologram") != std::string::npos) {
        return HoldStyle::Hologram;
    }
    if (holdType.find("tech") != std::string::npos) {
        return HoldStyle::TechRing;
    }
    if (holdType.find("neon") != std::string::npos) {
        return HoldStyle::Neon;
    }
    return HoldStyle::Charge;
}

NSColor* ArgbToNsColor(uint32_t argb) {
    const CGFloat alpha = static_cast<CGFloat>((argb >> 24) & 0xFFu) / 255.0;
    const CGFloat red = static_cast<CGFloat>((argb >> 16) & 0xFFu) / 255.0;
    const CGFloat green = static_cast<CGFloat>((argb >> 8) & 0xFFu) / 255.0;
    const CGFloat blue = static_cast<CGFloat>(argb & 0xFFu) / 255.0;
    return [NSColor colorWithCalibratedRed:red green:green blue:blue alpha:alpha];
}

NSColor* HoldBaseColor(
    MouseButton button,
    HoldStyle style,
    const macos_effect_profile::HoldRenderProfile& profile) {
    if (style == HoldStyle::Lightning) {
        return ArgbToNsColor(profile.colors.lightningStrokeArgb);
    }
    if (style == HoldStyle::Hex) {
        return ArgbToNsColor(profile.colors.hexStrokeArgb);
    }
    if (style == HoldStyle::Hologram) {
        return ArgbToNsColor(profile.colors.hologramStrokeArgb);
    }
    if (style == HoldStyle::QuantumHalo) {
        return ArgbToNsColor(profile.colors.quantumHaloStrokeArgb);
    }
    if (style == HoldStyle::FluxField) {
        return ArgbToNsColor(profile.colors.fluxFieldStrokeArgb);
    }
    if (style == HoldStyle::TechRing || style == HoldStyle::Neon) {
        return ArgbToNsColor(profile.colors.techNeonStrokeArgb);
    }
    if (button == MouseButton::Right) {
        return ArgbToNsColor(profile.colors.rightBaseStrokeArgb);
    }
    if (button == MouseButton::Middle) {
        return ArgbToNsColor(profile.colors.middleBaseStrokeArgb);
    }
    return ArgbToNsColor(profile.colors.leftBaseStrokeArgb);
}

void ConfigureHoldAccentLayer(CAShapeLayer* accent, CGRect bounds, HoldStyle holdStyle, NSColor* baseColor) {
    if (ConfigureSpecialHoldAccentLayer(accent, bounds, holdStyle, baseColor)) {
        return;
    }

    CGPathRef path = CGPathCreateWithEllipseInRect(CGRectInset(bounds, 44.0, 44.0), nullptr);
    accent.path = path;
    CGPathRelease(path);
    accent.fillColor = [NSColor clearColor].CGColor;
    accent.strokeColor = [[baseColor colorWithAlphaComponent:0.85] CGColor];
    accent.lineWidth = 1.4;
    accent.lineDashPattern = @[@6, @6];
}
#endif

} // namespace mousefx::macos_hold_pulse::detail

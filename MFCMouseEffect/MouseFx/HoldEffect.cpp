#include "pch.h"
#include "HoldEffect.h"
#include "ThemeStyle.h"
#include "RenderStrategies.h"

namespace mousefx {

HoldEffect::HoldEffect(const std::string& themeName, Mode mode) : mode_(mode) {
    style_ = GetThemePalette(themeName).hold;
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
}

HoldEffect::~HoldEffect() {
    Shutdown();
}

bool HoldEffect::Initialize() {
    // Small pool for hold indicators
    return pool_.Initialize(5);
}

void HoldEffect::Shutdown() {
    pool_.Shutdown();
}

void HoldEffect::OnHoldStart(const POINT& pt, int button) {
    if (holdButton_ != 0) return; // Already holding?

    holdPoint_ = pt;
    holdButton_ = button;
    
    ClickEvent ev{};
    ev.pt = pt;
    ev.button = MouseButton::Left;

    RenderParams params;
    params.loop = false;
    params.intensity = 1.0f;

    std::unique_ptr<IRippleRenderer> renderer;
    switch (mode_) {
    case Mode::Charge:    renderer = std::make_unique<ChargeRenderer>(); break;
    case Mode::Lightning: renderer = std::make_unique<LightningRenderer>(); break;
    case Mode::Hex:       renderer = std::make_unique<HexRenderer>(); break;
    case Mode::TechRing:  renderer = std::make_unique<HologramRenderer>(); break; // Tech Ring = Complex Gyro
    case Mode::Hologram:  renderer = std::make_unique<TechHudRenderer>(); break;  // Hologram  = Simple HUD
    default:              renderer = std::make_unique<ChargeRenderer>(); break;
    }

    RippleStyle finalStyle = style_;
    if (isChromatic_) {
        // For Hold effect, maybe we want it to change color over time?
        // But for v1, just random start color is enough.
        finalStyle = MakeRandomStyle(style_);
    }

    currentRipple_ = pool_.ShowContinuous(ev, finalStyle, std::move(renderer), params);
}

void HoldEffect::OnHoldUpdate(const POINT& pt, DWORD durationMs) {
    holdPoint_ = pt;
    if (currentRipple_) {
        currentRipple_->UpdatePosition(pt);
    }
    (void)durationMs;
}

void HoldEffect::OnHoldEnd() {
    if (currentRipple_) {
        currentRipple_->Stop();
        currentRipple_ = nullptr;
    }
    holdButton_ = 0;
}

} // namespace mousefx

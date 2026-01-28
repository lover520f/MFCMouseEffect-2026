#include "pch.h"
#include "HoldEffect.h"
#include "ThemeStyle.h"

namespace mousefx {

HoldEffect::HoldEffect(const std::string& themeName, Mode mode) : mode_(mode) {
    style_ = GetThemePalette(themeName).hold;
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

    RippleWindow::RenderParams params;
    params.loop = false;
    params.intensity = 1.0f;

    auto drawMode = RippleWindow::DrawMode::ChargeRing;
    if (mode_ == Mode::Lightning) drawMode = RippleWindow::DrawMode::LightningSingularity;
    else if (mode_ == Mode::Hex)  drawMode = RippleWindow::DrawMode::HexForceField;

    currentRipple_ = pool_.ShowContinuous(ev, style_, drawMode, params);
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

#include "pch.h"
#include "HoldEffect.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Renderers/RendererRegistry.h"
// Include all renderers so they are linked/registered? 
// No, reliance on static registration means we need to link the object files or include headers somewhere.
// In a single project, headers included in .cpps are enough if that cpp is compiled.
// For now, let's include headers here to ensure they are compiled (as separate files).
// Or we can include them in AppController or a "RegistryInit.cpp".
// Safest is to include them here or in a central place.
#include "MouseFx/Renderers/Hold/ChargeRenderer.h"
#include "MouseFx/Renderers/Hold/LightningRenderer.h"
#include "MouseFx/Renderers/Hold/HexRenderer.h"
#include "MouseFx/Renderers/Hold/TechRingRenderer.h"
#include "MouseFx/Renderers/Hold/HologramHudRenderer.h"

namespace mousefx {

HoldEffect::HoldEffect(const std::string& themeName, const std::string& type) : type_(type) {
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

    std::unique_ptr<IRippleRenderer> renderer = RendererRegistry::Instance().Create(type_);
    if (!renderer) {
        // Fallback to charge if not found
        renderer = RendererRegistry::Instance().Create("charge");
    }

    RippleStyle finalStyle = style_;
    if (isChromatic_) {
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

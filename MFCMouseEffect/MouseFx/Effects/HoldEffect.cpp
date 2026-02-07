#include "pch.h"
#include "HoldEffect.h"
#include "MouseFx/Core/OverlayHostService.h"
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
#include "MouseFx/Renderers/Hold/HoldNeon3DRenderer.h"

namespace mousefx {

HoldEffect::HoldEffect(const std::string& themeName, const std::string& type) : type_(type) {
    style_ = GetThemePalette(themeName).hold;
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
}

HoldEffect::~HoldEffect() {
    Shutdown();
}

bool HoldEffect::Initialize() {
    return true;
}

void HoldEffect::Shutdown() {
    OnHoldEnd();
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

    currentRippleId_ = OverlayHostService::Instance().ShowContinuousRipple(
        ev, finalStyle, std::move(renderer), params);
    if (currentRippleId_ == 0) {
        if (!pool_.Initialize(5)) return;
        std::unique_ptr<IRippleRenderer> fallbackRenderer = RendererRegistry::Instance().Create(type_);
        if (!fallbackRenderer) {
            fallbackRenderer = RendererRegistry::Instance().Create("charge");
        }
        currentRipple_ = pool_.ShowContinuous(ev, finalStyle, std::move(fallbackRenderer), params);
    } else {
        currentRipple_ = nullptr;
        char buf[32]{};
        snprintf(buf, sizeof(buf), "%u", finalStyle.durationMs);
        OverlayHostService::Instance().SendRippleCommand(currentRippleId_, "threshold_ms", buf);
    }
    if (currentRipple_) {
        char buf[32]{};
        snprintf(buf, sizeof(buf), "%u", finalStyle.durationMs);
        currentRipple_->SendCommand("threshold_ms", buf);
    }
}

void HoldEffect::OnHoldUpdate(const POINT& pt, DWORD durationMs) {
    holdPoint_ = pt;
    if (currentRippleId_ != 0) {
        OverlayHostService::Instance().UpdateRipplePosition(currentRippleId_, pt);
        char buf[32]{};
        snprintf(buf, sizeof(buf), "%u", (uint32_t)durationMs);
        OverlayHostService::Instance().SendRippleCommand(currentRippleId_, "hold_ms", buf);
    }
    if (currentRipple_) {
        currentRipple_->UpdatePosition(pt);
        char buf[32]{};
        snprintf(buf, sizeof(buf), "%u", (uint32_t)durationMs);
        currentRipple_->SendCommand("hold_ms", buf);
    }
}

void HoldEffect::OnHoldEnd() {
    if (currentRippleId_ != 0) {
        OverlayHostService::Instance().StopRipple(currentRippleId_);
        currentRippleId_ = 0;
    }
    if (currentRipple_) {
        currentRipple_->Stop();
        currentRipple_ = nullptr;
    }
    holdButton_ = 0;
}

void HoldEffect::OnCommand(const std::string& cmd, const std::string& args) {
    OverlayHostService::Instance().BroadcastRippleCommand(cmd, args);
    pool_.BroadcastCommand(cmd, args);
}

} // namespace mousefx

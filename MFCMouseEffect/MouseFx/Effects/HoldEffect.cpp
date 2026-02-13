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
#include "MouseFx/Renderers/Hold/HoldNeon3DGpuV2Renderer.h"
#include "MouseFx/Renderers/Hold/FluxFieldHudCpuRenderer.h"
#include "MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h"
#include <cmath>

namespace mousefx {

HoldEffect::HoldEffect(const std::string& themeName, const std::string& type, const std::string& followMode)
    : type_(type), followMode_(ParseFollowMode(followMode)) {
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
}

void HoldEffect::OnHoldStart(const POINT& pt, int button) {
    if (holdButton_ != 0) return; // Already holding?

    holdPoint_ = pt;
    holdButton_ = button;
    hasSmoothedPoint_ = false;
    hasLastSentPoint_ = false;
    lastHoldCommandMs_ = 0;
    lastEfficientPosMs_ = 0;
    
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
    if (currentRippleId_ != 0) {
        char buf[32]{};
        snprintf(buf, sizeof(buf), "%u", finalStyle.durationMs);
        OverlayHostService::Instance().SendRippleCommand(currentRippleId_, "threshold_ms", buf);
    }
}

void HoldEffect::OnHoldUpdate(const POINT& pt, DWORD durationMs) {
    holdPoint_ = pt;
    if (currentRippleId_ == 0) return;

    const uint64_t nowMs = GetTickCount64();
    POINT outPt = pt;
    bool shouldUpdatePos = false;

    switch (followMode_) {
        case FollowMode::Precise:
            shouldUpdatePos = true;
            break;
        case FollowMode::Smooth: {
            const float alpha = 0.35f;
            if (!hasSmoothedPoint_) {
                smoothedX_ = (float)pt.x;
                smoothedY_ = (float)pt.y;
                hasSmoothedPoint_ = true;
            } else {
                smoothedX_ += ((float)pt.x - smoothedX_) * alpha;
                smoothedY_ += ((float)pt.y - smoothedY_) * alpha;
            }
            outPt.x = (LONG)std::lround(smoothedX_);
            outPt.y = (LONG)std::lround(smoothedY_);
            shouldUpdatePos = true;
            break;
        }
        case FollowMode::Efficient:
            if (nowMs - lastEfficientPosMs_ >= 20) {
                lastEfficientPosMs_ = nowMs;
                shouldUpdatePos = true;
            }
            break;
    }

    if (shouldUpdatePos) {
        if (!hasLastSentPoint_ || !IsSamePoint(lastSentPoint_, outPt)) {
            OverlayHostService::Instance().UpdateRipplePosition(currentRippleId_, outPt);
            lastSentPoint_ = outPt;
            hasLastSentPoint_ = true;
        }
    }

    uint64_t cmdIntervalMs = 0;
    if (followMode_ == FollowMode::Smooth) cmdIntervalMs = 8;
    if (followMode_ == FollowMode::Efficient) cmdIntervalMs = 20;
    if (cmdIntervalMs == 0 || nowMs - lastHoldCommandMs_ >= cmdIntervalMs) {
        lastHoldCommandMs_ = nowMs;
        char buf[32]{};
        snprintf(buf, sizeof(buf), "%u", (uint32_t)durationMs);
        OverlayHostService::Instance().SendRippleCommand(currentRippleId_, "hold_ms", buf);
    }
}

void HoldEffect::OnHoldEnd() {
    if (currentRippleId_ != 0) {
        OverlayHostService::Instance().StopRipple(currentRippleId_);
        currentRippleId_ = 0;
    }
    holdButton_ = 0;
    hasSmoothedPoint_ = false;
    hasLastSentPoint_ = false;
    lastHoldCommandMs_ = 0;
    lastEfficientPosMs_ = 0;
}

void HoldEffect::OnCommand(const std::string& cmd, const std::string& args) {
    OverlayHostService::Instance().BroadcastRippleCommand(cmd, args);
}

HoldEffect::FollowMode HoldEffect::ParseFollowMode(const std::string& mode) {
    std::string value = ToLowerAscii(mode);
    if (value == "precise") return FollowMode::Precise;
    if (value == "efficient") return FollowMode::Efficient;
    return FollowMode::Smooth;
}

bool HoldEffect::IsSamePoint(const POINT& a, const POINT& b) {
    return a.x == b.x && a.y == b.y;
}

} // namespace mousefx

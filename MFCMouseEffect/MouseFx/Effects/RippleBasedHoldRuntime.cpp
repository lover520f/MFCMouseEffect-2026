#include "pch.h"
#include "RippleBasedHoldRuntime.h"

#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Renderers/RendererRegistry.h"
#include "MouseFx/Renderers/HoldRuntimeRegistry.h"
#include "MouseFx/Styles/ThemeStyle.h"

// Include hold renderer headers to ensure static registration occurs.
#include "MouseFx/Renderers/Hold/ChargeRenderer.h"
#include "MouseFx/Renderers/Hold/LightningRenderer.h"
#include "MouseFx/Renderers/Hold/HexRenderer.h"
#include "MouseFx/Renderers/Hold/TechRingRenderer.h"
#include "MouseFx/Renderers/Hold/HologramHudRenderer.h"
#include "MouseFx/Renderers/Hold/HoldNeon3DRenderer.h"
#include "MouseFx/Renderers/Hold/HoldQuantumHaloGpuV2Renderer.h"
#include "MouseFx/Renderers/Hold/FluxFieldHudCpuRenderer.h"
#include "MouseFx/Renderers/Hold/FluxFieldHudGpuV2Renderer.h"

#include <cstdio>

namespace mousefx {

RippleBasedHoldRuntime::RippleBasedHoldRuntime(
    const std::string& rendererName,
    bool isGpuV2Route,
    bool isChromatic)
    : rendererName_(rendererName),
      isGpuV2Route_(isGpuV2Route),
      isChromatic_(isChromatic) {
}

bool RippleBasedHoldRuntime::Start(const RippleStyle& style, const POINT& pt) {
    if (rippleId_ != 0) return false; // Already running

    ClickEvent ev{};
    ev.pt = pt;
    ev.button = MouseButton::Left;

    RenderParams params;
    params.loop = false;
    params.intensity = 1.0f;

    std::unique_ptr<IRippleRenderer> renderer = RendererRegistry::Instance().Create(rendererName_);
    if (!renderer) {
        renderer = RendererRegistry::Instance().Create("charge");
    }

    RippleStyle finalStyle = style;
    if (isChromatic_) {
        finalStyle = MakeRandomStyle(style);
    }

    rippleId_ = OverlayHostService::Instance().ShowContinuousRipple(
        ev, finalStyle, std::move(renderer), params);
    if (rippleId_ != 0) {
        char buf[32]{};
        snprintf(buf, sizeof(buf), "%u", finalStyle.durationMs);
        OverlayHostService::Instance().SendRippleCommand(rippleId_, "threshold_ms", buf);
        if (isGpuV2Route_) {
            SendHoldStateCommand(0, pt);
        }
    }
    hasLastSentPoint_ = false;
    return rippleId_ != 0;
}

void RippleBasedHoldRuntime::Update(uint32_t holdMs, const POINT& pt) {
    if (rippleId_ == 0) return;

    if (!hasLastSentPoint_ || lastSentPoint_.x != pt.x || lastSentPoint_.y != pt.y) {
        OverlayHostService::Instance().UpdateRipplePosition(rippleId_, pt);
        lastSentPoint_ = pt;
        hasLastSentPoint_ = true;
    }

    char buf[32]{};
    snprintf(buf, sizeof(buf), "%u", holdMs);
    OverlayHostService::Instance().SendRippleCommand(rippleId_, "hold_ms", buf);
    if (isGpuV2Route_) {
        const POINT statePt = hasLastSentPoint_ ? lastSentPoint_ : pt;
        SendHoldStateCommand(holdMs, statePt);
    }
}

void RippleBasedHoldRuntime::Stop() {
    if (rippleId_ == 0) return;
    if (isGpuV2Route_) {
        const POINT finalPt = hasLastSentPoint_ ? lastSentPoint_ : POINT{};
        SendHoldEndCommand(finalPt);
    }
    OverlayHostService::Instance().StopRipple(rippleId_);
    rippleId_ = 0;
    hasLastSentPoint_ = false;
}

bool RippleBasedHoldRuntime::IsRunning() const {
    return rippleId_ != 0;
}

void RippleBasedHoldRuntime::SendHoldStateCommand(uint32_t holdMs, const POINT& pt) const {
    if (rippleId_ == 0) return;
    char buf[96]{};
    snprintf(buf, sizeof(buf), "%u,%ld,%ld", holdMs, (long)pt.x, (long)pt.y);
    OverlayHostService::Instance().SendRippleCommand(rippleId_, "hold_state", buf);
}

void RippleBasedHoldRuntime::SendHoldEndCommand(const POINT& pt) const {
    if (rippleId_ == 0) return;
    char buf[64]{};
    snprintf(buf, sizeof(buf), "%ld,%ld", (long)pt.x, (long)pt.y);
    OverlayHostService::Instance().SendRippleCommand(rippleId_, "hold_end", buf);
}

} // namespace mousefx

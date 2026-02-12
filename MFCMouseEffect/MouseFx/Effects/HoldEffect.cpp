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
#include <cmath>

namespace mousefx {
namespace {

bool IsHeavyHoldRendererType(const std::string& type) {
    const std::string t = ToLowerAscii(type);
    return t == "neon3d" || t == "hold_neon3d" || t == "hologram_hud";
}

bool IsNeon3DHoldType(const std::string& type) {
    const std::string t = ToLowerAscii(type);
    return t == "neon3d" || t == "hold_neon3d";
}

Argb ZeroAlpha(Argb c) {
    c.value &= 0x00FFFFFFu;
    return c;
}

void PrewarmHoldRenderer(const std::string& type, const RippleStyle& baseStyle) {
    if (!IsHeavyHoldRendererType(type)) return;

    std::unique_ptr<IRippleRenderer> renderer = RendererRegistry::Instance().Create(type);
    if (!renderer) return;

    RippleStyle warmStyle = baseStyle;
    warmStyle.durationMs = 24;
    warmStyle.windowSize = std::max(96, std::min(baseStyle.windowSize, 192));
    warmStyle.startRadius = std::max(6.0f, std::min(baseStyle.startRadius, 16.0f));
    warmStyle.endRadius = std::max(warmStyle.startRadius + 6.0f, std::min(baseStyle.endRadius, 28.0f));
    warmStyle.strokeWidth = std::max(1.0f, std::min(baseStyle.strokeWidth, 2.0f));
    warmStyle.fill = ZeroAlpha(baseStyle.fill);
    warmStyle.stroke = ZeroAlpha(baseStyle.stroke);
    warmStyle.glow = ZeroAlpha(baseStyle.glow);

    Gdiplus::Bitmap bmp(warmStyle.windowSize, warmStyle.windowSize, PixelFormat32bppARGB);
    Gdiplus::Graphics g(&bmp);
    g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);

    renderer->Start(warmStyle);
    renderer->Render(g, 0.0f, 0, warmStyle.windowSize, warmStyle);
    renderer->SetHoldElapsedMs(std::max<uint32_t>(1, warmStyle.durationMs / 2));
    renderer->SetHoldDurationMs(std::max<uint32_t>(1, warmStyle.durationMs));
    renderer->Render(g, 0.5f, std::max<uint64_t>(1, warmStyle.durationMs / 2), warmStyle.windowSize, warmStyle);
}

} // namespace

HoldEffect::HoldEffect(const std::string& themeName, const std::string& type, const std::string& followMode)
    : type_(type), followMode_(ParseFollowMode(followMode)) {
    style_ = GetThemePalette(themeName).hold;
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
}

HoldEffect::~HoldEffect() {
    Shutdown();
}

bool HoldEffect::Initialize() {
    // Prewarm heavy hold renderers at startup so the first real hold interaction is smoother.
    PrewarmHoldRenderer(type_, style_);
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
    hasLastRawPoint_ = false;
    lastRawPointTickMs_ = 0;
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
        OverlayHostService::Instance().UpdateRippleHoldThreshold(currentRippleId_, finalStyle.durationMs);
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
            float alpha = IsNeon3DHoldType(type_) ? 0.58f : 0.35f;
            const bool layeredFinalPresent = !OverlayHostService::Instance().IsGpuPresentActive();
            if (latencyPriorityActive_ && IsNeon3DHoldType(type_)) {
                alpha = 0.82f;
            }
            float speedPxPerMs = 0.0f;
            bool hasSpeedSample = false;
            if (hasLastRawPoint_ && nowMs > lastRawPointTickMs_) {
                const float dx = static_cast<float>(pt.x - lastRawPoint_.x);
                const float dy = static_cast<float>(pt.y - lastRawPoint_.y);
                const float dtMs = static_cast<float>(nowMs - lastRawPointTickMs_);
                if (dtMs > 0.0f) {
                    speedPxPerMs = std::sqrt(dx * dx + dy * dy) / dtMs;
                    hasSpeedSample = true;
                    if (IsNeon3DHoldType(type_)) {
                        // Layered CPU final-present path needs stronger follow to keep cursor sync.
                        if (layeredFinalPresent) {
                            if (latencyPriorityActive_) {
                                if (speedPxPerMs >= 2.0f) alpha = 0.96f;
                                else if (speedPxPerMs >= 1.0f) alpha = 0.92f;
                                else if (speedPxPerMs >= 0.5f) alpha = 0.86f;
                                else alpha = 0.80f;
                            } else {
                                if (speedPxPerMs >= 2.2f) alpha = 0.94f;
                                else if (speedPxPerMs >= 1.2f) alpha = 0.88f;
                                else if (speedPxPerMs >= 0.6f) alpha = 0.78f;
                                else alpha = 0.72f;
                            }
                        } else {
                            if (speedPxPerMs >= 1.8f) alpha = 0.86f;
                            else if (speedPxPerMs >= 0.9f) alpha = 0.74f;
                        }
                    }
                }
            }
            lastRawPoint_ = pt;
            hasLastRawPoint_ = true;
            lastRawPointTickMs_ = nowMs;

            bool directFollow = false;
            if (IsNeon3DHoldType(type_) && layeredFinalPresent) {
                if (hasSpeedSample && speedPxPerMs >= (latencyPriorityActive_ ? 1.8f : 1.2f)) {
                    directFollow = true;
                } else if (hasSmoothedPoint_) {
                    const float dx = std::fabs(static_cast<float>(pt.x) - smoothedX_);
                    const float dy = std::fabs(static_cast<float>(pt.y) - smoothedY_);
                    if ((dx + dy) >= 14.0f) {
                        // Large cursor jumps should snap to cursor to avoid visible tail lag.
                        directFollow = true;
                    }
                }
            }
            if (!hasSmoothedPoint_) {
                smoothedX_ = (float)pt.x;
                smoothedY_ = (float)pt.y;
                hasSmoothedPoint_ = true;
            } else if (directFollow) {
                smoothedX_ = (float)pt.x;
                smoothedY_ = (float)pt.y;
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
    if (followMode_ == FollowMode::Smooth) {
        if (latencyPriorityActive_ && IsNeon3DHoldType(type_)) {
            cmdIntervalMs = 2;
        } else {
            cmdIntervalMs = IsNeon3DHoldType(type_) ? 4 : 8;
        }
    }
    if (followMode_ == FollowMode::Efficient) cmdIntervalMs = 20;
    if (cmdIntervalMs == 0 || nowMs - lastHoldCommandMs_ >= cmdIntervalMs) {
        lastHoldCommandMs_ = nowMs;
        OverlayHostService::Instance().UpdateRippleHoldElapsed(currentRippleId_, (uint32_t)durationMs);
    }
}

void HoldEffect::OnHoldEnd() {
    if (currentRippleId_ != 0) {
        OverlayHostService::Instance().StopRipple(currentRippleId_);
        currentRippleId_ = 0;
    }
    holdButton_ = 0;
    hasSmoothedPoint_ = false;
    hasLastRawPoint_ = false;
    lastRawPointTickMs_ = 0;
    hasLastSentPoint_ = false;
    lastHoldCommandMs_ = 0;
    lastEfficientPosMs_ = 0;
}

void HoldEffect::OnCommand(const std::string& cmd, const std::string& args) {
    if (cmd == "latency_priority") {
        const std::string v = ToLowerAscii(args);
        latencyPriorityActive_ = (v == "on" || v == "1" || v == "true");
        return;
    }
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

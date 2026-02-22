#include "pch.h"

#include "RippleOverlayLayer.h"
#include "MouseFx/Core/Overlay/OverlayCoordSpace.h"
#include "MouseFx/Core/Protocol/InputTypesWin32.h"
#include "MouseFx/Utils/TimeUtils.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace mousefx {


uint64_t RippleOverlayLayer::ShowRipple(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params) {
    if (!renderer) return 0;

    RippleInstance instance{};
    instance.id = nextId_++;
    instance.ev = ev;
    instance.style = style;
    instance.params = params;
    instance.renderer = std::move(renderer);
    instance.startTick = NowMs();
    instance.rawElapsedMs = 0;
    instance.elapsedMs = 0;
    instance.t = 0.0f;
    instance.renderReady = (instance.params.startDelayMs == 0u);
    instance.active = true;
    instance.continuous = false;

    instance.renderer->SetParams(instance.params);
    instance.renderer->Start(instance.style);

    instances_.push_back(std::move(instance));
    return instances_.back().id;
}

uint64_t RippleOverlayLayer::ShowContinuous(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params) {
    if (!renderer) return 0;

    RippleInstance instance{};
    instance.id = nextId_++;
    instance.ev = ev;
    instance.style = style;
    instance.params = params;
    instance.renderer = std::move(renderer);
    instance.startTick = NowMs();
    instance.rawElapsedMs = 0;
    instance.elapsedMs = 0;
    instance.t = 0.0f;
    instance.renderReady = (instance.params.startDelayMs == 0u);
    instance.active = true;
    instance.continuous = true;

    instance.renderer->SetParams(instance.params);
    instance.renderer->Start(instance.style);

    instances_.push_back(std::move(instance));
    return instances_.back().id;
}

void RippleOverlayLayer::UpdatePosition(uint64_t id, const POINT& pt) {
    RippleInstance* instance = FindById(id);
    if (!instance) return;
    instance->ev.pt = ToScreenPoint(pt);
}

void RippleOverlayLayer::Stop(uint64_t id) {
    RippleInstance* instance = FindById(id);
    if (!instance) return;
    instance->active = false;
}

bool RippleOverlayLayer::IsActive(uint64_t id) const {
    const RippleInstance* instance = FindById(id);
    return instance && instance->active;
}

void RippleOverlayLayer::SendCommand(uint64_t id, const std::string& cmd, const std::string& args) {
    RippleInstance* instance = FindById(id);
    if (!instance || !instance->renderer) return;
    instance->renderer->OnCommand(cmd, args);
}

void RippleOverlayLayer::BroadcastCommand(const std::string& cmd, const std::string& args) {
    for (auto& instance : instances_) {
        if (instance.active && instance.renderer) {
            instance.renderer->OnCommand(cmd, args);
        }
    }
}

void RippleOverlayLayer::Update(uint64_t nowMs) {
    for (auto& instance : instances_) {
        if (!instance.active || !instance.renderer) continue;
        instance.rawElapsedMs = (nowMs >= instance.startTick) ? (nowMs - instance.startTick) : 0;
        if (instance.rawElapsedMs < static_cast<uint64_t>(instance.params.startDelayMs)) {
            instance.renderReady = false;
            instance.elapsedMs = 0;
            instance.t = 0.0f;
            continue;
        }

        instance.renderReady = true;
        instance.elapsedMs = instance.rawElapsedMs - static_cast<uint64_t>(instance.params.startDelayMs);
        const uint32_t durationMs = (instance.style.durationMs == 0) ? 1u : instance.style.durationMs;
        float t = (float)instance.elapsedMs / (float)durationMs;

        if (instance.continuous) {
            if (instance.params.loop) {
                if (t > 1.0f) {
                    const uint64_t delayMs = static_cast<uint64_t>(instance.params.startDelayMs);
                    instance.startTick = (nowMs >= delayMs) ? (nowMs - delayMs) : 0;
                    instance.rawElapsedMs = delayMs;
                    instance.elapsedMs = 0;
                    t = 0.0f;
                    instance.renderer->Start(instance.style);
                }
            } else {
                if (t > 1.0f) t = 1.0f;
            }
        } else {
            if (t >= 1.0f) {
                instance.active = false;
                continue;
            }
        }

        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;
        instance.t = t;
    }

    instances_.erase(
        std::remove_if(
            instances_.begin(),
            instances_.end(),
            [](const RippleInstance& instance) { return !instance.active; }),
        instances_.end());
}

void RippleOverlayLayer::Render(Gdiplus::Graphics& graphics) {
    for (auto& instance : instances_) {
        if (!instance.active || !instance.renderer || !instance.renderReady) continue;

        int sizePx = instance.style.windowSize;
        if (sizePx < 64) sizePx = 64;
        if (sizePx > 512) sizePx = 512;

        const POINT centerPt = ResolveRenderCenter(instance);
        const int left = centerPt.x - (sizePx / 2);
        const int top = centerPt.y - (sizePx / 2);

        const Gdiplus::GraphicsState state = graphics.Save();
        graphics.TranslateTransform((Gdiplus::REAL)left, (Gdiplus::REAL)top);
        instance.renderer->Render(graphics, instance.t, instance.elapsedMs, sizePx, instance.style);
        graphics.Restore(state);
    }
}

POINT RippleOverlayLayer::ResolveRenderCenter(const RippleInstance& instance) const {
    POINT screenPt = ToNativePoint(instance.ev.pt);
    if (instance.params.useKinematics) {
        const double tSec = static_cast<double>(instance.elapsedMs) / 1000.0;
        const double dx =
            static_cast<double>(instance.params.velocityX) * tSec +
            0.5 * static_cast<double>(instance.params.accelerationX) * tSec * tSec;
        const double dy =
            static_cast<double>(instance.params.velocityY) * tSec +
            0.5 * static_cast<double>(instance.params.accelerationY) * tSec * tSec;

        const long long x = static_cast<long long>(screenPt.x) + static_cast<long long>(std::llround(dx));
        const long long y = static_cast<long long>(screenPt.y) + static_cast<long long>(std::llround(dy));

        const long long minL = static_cast<long long>((std::numeric_limits<LONG>::min)());
        const long long maxL = static_cast<long long>((std::numeric_limits<LONG>::max)());
        screenPt.x = static_cast<LONG>((std::min)((std::max)(x, minL), maxL));
        screenPt.y = static_cast<LONG>((std::min)((std::max)(y, minL), maxL));
    }
    return ScreenToOverlayPoint(screenPt);
}

RippleOverlayLayer::RippleInstance* RippleOverlayLayer::FindById(uint64_t id) {
    if (id == 0) return nullptr;
    for (auto& instance : instances_) {
        if (instance.id == id) return &instance;
    }
    return nullptr;
}

const RippleOverlayLayer::RippleInstance* RippleOverlayLayer::FindById(uint64_t id) const {
    if (id == 0) return nullptr;
    for (const auto& instance : instances_) {
        if (instance.id == id) return &instance;
    }
    return nullptr;
}

} // namespace mousefx

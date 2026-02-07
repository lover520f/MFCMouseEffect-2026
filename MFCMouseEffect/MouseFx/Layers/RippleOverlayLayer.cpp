#include "pch.h"

#include "RippleOverlayLayer.h"

#include <algorithm>

namespace mousefx {

uint64_t RippleOverlayLayer::NowMs() {
    return GetTickCount64();
}

uint64_t RippleOverlayLayer::ShowRipple(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params) {
    if (!renderer) return 0;

    RippleInstance instance{};
    instance.id = nextId_++;
    instance.ev = ev;
    instance.style = style;
    instance.params = params;
    instance.renderer = std::move(renderer);
    instance.startTick = NowMs();
    instance.elapsedMs = 0;
    instance.t = 0.0f;
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
    instance.elapsedMs = 0;
    instance.t = 0.0f;
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
    instance->ev.pt = pt;
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
        instance.elapsedMs = (nowMs >= instance.startTick) ? (nowMs - instance.startTick) : 0;
        const uint32_t durationMs = (instance.style.durationMs == 0) ? 1u : instance.style.durationMs;
        float t = (float)instance.elapsedMs / (float)durationMs;

        if (instance.continuous) {
            if (instance.params.loop) {
                if (t > 1.0f) {
                    instance.startTick = nowMs;
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
    const int virtualX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    const int virtualY = GetSystemMetrics(SM_YVIRTUALSCREEN);

    for (auto& instance : instances_) {
        if (!instance.active || !instance.renderer) continue;

        int sizePx = instance.style.windowSize;
        if (sizePx < 64) sizePx = 64;
        if (sizePx > 512) sizePx = 512;

        const int left = (int)instance.ev.pt.x - (sizePx / 2) - virtualX;
        const int top = (int)instance.ev.pt.y - (sizePx / 2) - virtualY;

        const Gdiplus::GraphicsState state = graphics.Save();
        graphics.TranslateTransform((Gdiplus::REAL)left, (Gdiplus::REAL)top);
        instance.renderer->Render(graphics, instance.t, instance.elapsedMs, sizePx, instance.style);
        graphics.Restore(state);
    }
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

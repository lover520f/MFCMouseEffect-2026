// RippleWindowPool.cpp

#include "pch.h"
#include "RippleWindowPool.h"
#include <algorithm>

namespace mousefx {

bool RippleWindowPool::Initialize(size_t count) {
    if (!windows_.empty()) return true;
    count = std::max<size_t>(2, std::min<size_t>(16, count));
    windows_.reserve(count);
    for (size_t i = 0; i < count; i++) {
        auto w = std::make_unique<RippleWindow>();
        if (!w->Create()) {
            return false;
        }
        windows_.push_back(std::move(w));
    }
    return true;
}

void RippleWindowPool::Shutdown() {
    windows_.clear();
}

void RippleWindowPool::ShowRipple(const ClickEvent& ev) {
    for (auto& w : windows_) {
        if (!w->IsActive()) {
            w->StartAt(ev, nullptr);
            return;
        }
    }
}

void RippleWindowPool::ShowRipple(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params) {
    for (auto& w : windows_) {
        if (!w->IsActive()) {
            w->StartAt(ev, style, std::move(renderer), params);
            return;
        }
    }
}

RippleWindow* RippleWindowPool::ShowContinuous(const ClickEvent& ev) {
    for (auto& w : windows_) {
        if (!w->IsActive()) {
            w->StartContinuous(ev, nullptr);
            return w.get();
        }
    }
    return nullptr;
}

RippleWindow* RippleWindowPool::ShowContinuous(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params) {
    for (auto& w : windows_) {
        if (!w->IsActive()) {
            w->StartContinuous(ev, style, std::move(renderer), params);
            return w.get();
        }
    }
    return nullptr;
}

void RippleWindowPool::BroadcastCommand(const std::string& cmd, const std::string& args) {
    for (auto& w : windows_) {
        if (w->IsActive()) {
            w->SendCommand(cmd, args);
        }
    }
}

} // namespace mousefx

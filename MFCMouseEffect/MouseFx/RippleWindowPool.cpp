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
    if (windows_.empty()) {
        // Lazy init (works even if caller forgot).
        if (!Initialize(8)) return;
    }

    // Prefer an inactive window; otherwise recycle the oldest one.
    RippleWindow* best = nullptr;
    uint64_t bestTick = UINT64_MAX;

    for (auto& w : windows_) {
        if (!w->IsActive()) {
            best = w.get();
            break;
        }
        if (w->StartTick() < bestTick) {
            bestTick = w->StartTick();
            best = w.get();
        }
    }

    if (best) {
        best->StartAt(ev);
    }
}

RippleWindow* RippleWindowPool::ShowContinuous(const ClickEvent& ev) {
    if (windows_.empty()) {
        if (!Initialize(8)) return nullptr;
    }

    RippleWindow* best = nullptr;
    uint64_t bestTick = UINT64_MAX;

    for (auto& w : windows_) {
        if (!w->IsActive()) {
            best = w.get();
            break;
        }
        if (w->StartTick() < bestTick) {
            bestTick = w->StartTick();
            best = w.get();
        }
    }

    if (best) {
        best->StartContinuous(ev);
    }
    return best;
}

void RippleWindowPool::SetDrawMode(RippleWindow::DrawMode mode) {
    for (auto& w : windows_) {
        w->SetDrawMode(mode);
    }
}

} // namespace mousefx


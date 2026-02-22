#include "pch.h"
#include "TextWindowPool.h"
#include <algorithm>

namespace mousefx {

bool TextWindowPool::Initialize(size_t count) {
    if (!windows_.empty()) return true;
    count = std::max<size_t>(2, std::min<size_t>(32, count));
    windows_.reserve(count);
    for (size_t i = 0; i < count; i++) {
        auto w = std::make_unique<TextWindow>();
        if (!w->Create()) return false;
        windows_.push_back(std::move(w));
    }
    return true;
}

void TextWindowPool::Shutdown() {
    windows_.clear();
}

void TextWindowPool::ShowText(const ScreenPoint& pt, const std::wstring& text, Argb color, const TextConfig& config) {
    if (windows_.empty()) {
        if (!Initialize(10)) return;
    }

    TextWindow* best = nullptr;
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
        best->StartAt(pt, text, color, config);
    }
}

} // namespace mousefx

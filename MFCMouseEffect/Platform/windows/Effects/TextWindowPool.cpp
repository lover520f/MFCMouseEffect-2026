#include "pch.h"
#include "TextWindowPool.h"
#include "MouseFx/Core/Effects/TextEffectCompute.h"
#include "Settings/EmojiUtils.h"

#include <algorithm>
#include <random>

namespace mousefx {
namespace {

TextEffectRandomSamples BuildRandomSamples() {
    static thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_int_distribution<int> driftDist(-50, 49);
    std::uniform_int_distribution<int> swayFreqDist(0, 199);
    std::uniform_int_distribution<int> swayAmpDist(0, 99);
    TextEffectRandomSamples samples{};
    samples.driftX = driftDist(generator);
    samples.swayFreqCenti = swayFreqDist(generator);
    samples.swayAmpDeci = swayAmpDist(generator);
    return samples;
}

} // namespace

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
    const TextEffectRenderCommand command = ComputeTextEffectRenderCommand(
        text,
        color,
        config,
        settings::HasEmojiStarter(text),
        BuildRandomSamples());
    ShowTextComputed(pt, command);
}

void TextWindowPool::ShowTextComputed(const ScreenPoint& anchorPoint, const TextEffectRenderCommand& command) {
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
        best->StartAtComputed(anchorPoint, command);
    }
}

} // namespace mousefx

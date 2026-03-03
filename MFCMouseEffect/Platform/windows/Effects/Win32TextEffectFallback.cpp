#include "pch.h"

#include "Platform/windows/Effects/Win32TextEffectFallback.h"
#include "MouseFx/Core/Effects/TextEffectCompute.h"
#include "Settings/EmojiUtils.h"

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

bool Win32TextEffectFallback::EnsureInitialized(size_t count) {
    return pool_.Initialize(count);
}

void Win32TextEffectFallback::Shutdown() {
    pool_.Shutdown();
}

void Win32TextEffectFallback::ShowText(const ScreenPoint& pt, const std::wstring& text, Argb color, const TextConfig& config) {
    const TextEffectRenderCommand command = ComputeTextEffectRenderCommand(
        text,
        color,
        config,
        settings::HasEmojiStarter(text),
        BuildRandomSamples());
    ShowTextComputed(pt, command);
}

void Win32TextEffectFallback::ShowTextComputed(
    const ScreenPoint& anchorPoint,
    const TextEffectRenderCommand& command) {
    pool_.ShowTextComputed(anchorPoint, command);
}

} // namespace mousefx

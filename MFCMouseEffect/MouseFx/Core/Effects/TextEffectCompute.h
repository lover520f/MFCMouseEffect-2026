#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"

#include <cstdint>
#include <string>

namespace mousefx {

struct TextEffectRandomSamples {
    int driftX = 0;
    int swayFreqCenti = 100;
    int swayAmpDeci = 50;
};

struct TextEffectRenderCommand {
    std::wstring text{};
    uint32_t argb = 0xFFFFFFFFu;
    std::wstring fontFamily{};
    bool emojiText = false;
    int durationMs = 800;
    double floatDistancePx = 60.0;
    double baseFontSizePx = 16.0;
    double panelSizePx = 200.0;
    double driftX = 0.0;
    double swayFreq = 1.0;
    double swayAmp = 5.0;
};

struct TextEffectRenderFrame {
    double offsetXPx = 0.0;
    double offsetYUpPx = 0.0;
    double scale = 1.0;
    double alpha = 1.0;
    double rotationDeg = 0.0;
    double fontSizePx = 16.0;
};

TextEffectRenderCommand ComputeTextEffectRenderCommand(
    const std::wstring& text,
    Argb color,
    const TextConfig& config,
    bool emojiText,
    const TextEffectRandomSamples& randomSamples);

TextEffectRenderFrame ComputeTextEffectRenderFrame(
    const TextEffectRenderCommand& command,
    double progress01);

} // namespace mousefx


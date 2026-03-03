#include "pch.h"

#include "MouseFx/Core/Effects/TextEffectCompute.h"

#include <algorithm>
#include <cmath>

namespace mousefx {
namespace {

double Clamp01(double value) {
    return std::clamp(value, 0.0, 1.0);
}

double EaseOutCubic(double value) {
    const double u = 1.0 - Clamp01(value);
    return 1.0 - (u * u * u);
}

double ResolvePanelSize(double baseFontSizePx) {
    return std::max(200.0, baseFontSizePx * 8.0);
}

double ResolveFontSizePx(const TextConfig& config) {
    return std::max(6.0, static_cast<double>(config.fontSize) * (96.0 / 72.0));
}

double ResolveFloatDistancePx(const TextConfig& config) {
    return static_cast<double>(std::max(config.floatDistance, 0));
}

int ResolveDurationMs(const TextConfig& config) {
    return std::max(config.durationMs, 1);
}

double ResolveScale(double progress) {
    if (progress < 0.3) {
        return 0.8 + (progress / 0.3) * 0.4;
    }
    return 1.2 - ((progress - 0.3) / 0.7) * 0.2;
}

double ResolveAlpha(double progress) {
    if (progress < 0.15) {
        return Clamp01(progress / 0.15);
    }
    if (progress > 0.6) {
        return Clamp01(1.0 - (progress - 0.6) / 0.4);
    }
    return 1.0;
}

} // namespace

TextEffectRenderCommand ComputeTextEffectRenderCommand(
    const std::wstring& text,
    Argb color,
    const TextConfig& config,
    bool emojiText,
    const TextEffectRandomSamples& randomSamples) {
    TextEffectRenderCommand command{};
    command.text = text;
    command.argb = color.value;
    command.fontFamily = config.fontFamily;
    command.emojiText = emojiText;
    command.durationMs = ResolveDurationMs(config);
    command.floatDistancePx = ResolveFloatDistancePx(config);
    command.baseFontSizePx = ResolveFontSizePx(config);
    command.panelSizePx = ResolvePanelSize(command.baseFontSizePx);
    command.driftX = static_cast<double>(randomSamples.driftX);
    command.swayFreq = 1.0 + static_cast<double>(randomSamples.swayFreqCenti) / 100.0;
    command.swayAmp = 5.0 + static_cast<double>(randomSamples.swayAmpDeci) / 10.0;
    return command;
}

TextEffectRenderFrame ComputeTextEffectRenderFrame(
    const TextEffectRenderCommand& command,
    double progress01) {
    const double t = Clamp01(progress01);
    const double eased = EaseOutCubic(t);
    const double yOffset = eased * command.floatDistancePx;
    const double xOffset = (t * command.driftX) + std::sin(t * 3.1415926 * command.swayFreq) * command.swayAmp;
    const double scale = ResolveScale(t);
    const double alpha = ResolveAlpha(t);

    TextEffectRenderFrame frame{};
    frame.offsetXPx = xOffset;
    frame.offsetYUpPx = yOffset;
    frame.scale = scale;
    frame.alpha = alpha;
    frame.rotationDeg = xOffset * 0.2;
    frame.fontSizePx = std::max(6.0, command.baseFontSizePx * scale);
    return frame;
}

} // namespace mousefx


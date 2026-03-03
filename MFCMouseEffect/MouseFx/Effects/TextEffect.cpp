#include "pch.h"
#include "TextEffect.h"
#include "MouseFx/Core/Effects/TextEffectCompute.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Core/Diagnostics/TextEffectRuntimeDiagnostics.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "Platform/PlatformEffectFallbackFactory.h"
#include "Settings/EmojiUtils.h"
#include <random>

namespace mousefx {
namespace {

int RandomRange(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

std::wstring ResolveDefaultLabel(MouseButton button) {
    switch (button) {
    case MouseButton::Right:
        return L"RIGHT";
    case MouseButton::Middle:
        return L"MIDDLE";
    case MouseButton::Left:
    default:
        return L"LEFT";
    }
}

std::wstring ResolveClickText(const TextConfig& config, MouseButton button) {
    if (!config.texts.empty()) {
        return config.texts[RandomRange(0, static_cast<int>(config.texts.size()) - 1)];
    }
    return ResolveDefaultLabel(button);
}

TextEffectRandomSamples BuildRandomSamples() {
    TextEffectRandomSamples samples{};
    samples.driftX = RandomRange(-50, 49);
    samples.swayFreqCenti = RandomRange(0, 199);
    samples.swayAmpDeci = RandomRange(0, 99);
    return samples;
}

} // namespace

TextEffect::TextEffect(const TextConfig& config, const std::string& themeName)
    : config_(config),
      fallback_(platform::CreateTextEffectFallback()) {
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
}

TextEffect::~TextEffect() {
    Shutdown();
}

bool TextEffect::Initialize() {
    bool hasEmojiText = false;
    for (const auto& item : config_.texts) {
        if (settings::HasEmojiStarter(item)) {
            hasEmojiText = true;
            break;
        }
    }
    if (hasEmojiText) {
        if (!fallback_ || !fallback_->EnsureInitialized(8)) return false;
    }
    (void)OverlayHostService::Instance().Initialize();
    return true;
}

void TextEffect::Shutdown() {
    if (fallback_) {
        fallback_->Shutdown();
    }
}

void TextEffect::OnClick(const ClickEvent& event) {
    const std::wstring text = ResolveClickText(config_, event.button);
    if (text.empty()) {
        return;
    }

    diagnostics::RecordTextEffectClick(event.pt, text);
    
    Argb color = { 0xFFFF69B4 }; // Default
    if (isChromatic_) {
        // Random vibrant color
        color = MakeRandomColor();
    } else if (!config_.colors.empty()) {
        color = config_.colors[RandomRange(0, (int)config_.colors.size() - 1)];
    }
    const bool emojiText = settings::HasEmojiStarter(text);
    const TextEffectRenderCommand command = ComputeTextEffectRenderCommand(
        text,
        color,
        config_,
        emojiText,
        BuildRandomSamples());

    if (emojiText) {
        if (!fallback_ || !fallback_->EnsureInitialized(8)) return;
        fallback_->ShowTextComputed(event.pt, command);
        return;
    }

#if defined(__APPLE__)
    if (fallback_ && fallback_->EnsureInitialized(8)) {
        fallback_->ShowTextComputed(event.pt, command);
        return;
    }
#endif

    if (OverlayHostService::Instance().ShowText(event.pt, text, color, config_)) {
        return;
    }

    if (!fallback_ || !fallback_->EnsureInitialized(8)) return;
    fallback_->ShowTextComputed(event.pt, command);
}

} // namespace mousefx

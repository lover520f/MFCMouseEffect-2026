#include "pch.h"
#include "TextEffect.h"
#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "Platform/PlatformEffectFallbackFactory.h"
#include "Settings/EmojiUtils.h"
#include <random>

namespace mousefx {

static int RandomRange(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}


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
    if (config_.texts.empty()) return;

    // Pick random text and color
    const std::wstring& text = config_.texts[RandomRange(0, (int)config_.texts.size() - 1)];
    
    Argb color = { 0xFFFF69B4 }; // Default
    if (isChromatic_) {
        // Random vibrant color
        color = MakeRandomColor();
    } else if (!config_.colors.empty()) {
        color = config_.colors[RandomRange(0, (int)config_.colors.size() - 1)];
    }

    if (settings::HasEmojiStarter(text)) {
        if (!fallback_ || !fallback_->EnsureInitialized(8)) return;
        fallback_->ShowText(event.pt, text, color, config_);
        return;
    }

    if (OverlayHostService::Instance().ShowText(event.pt, text, color, config_)) {
        return;
    }

    if (!fallback_ || !fallback_->EnsureInitialized(8)) return;
    fallback_->ShowText(event.pt, text, color, config_);
}

} // namespace mousefx

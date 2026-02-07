#include "pch.h"
#include "TextEffect.h"
#include "MouseFx/Core/OverlayHostService.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include <random>

namespace mousefx {

static int RandomRange(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

static uint32_t NextCodePoint(const std::wstring& text, size_t* index) {
    if (!index || *index >= text.size()) return 0;
    wchar_t lead = text[*index];
    (*index)++;
    if (lead >= 0xD800 && lead <= 0xDBFF && *index < text.size()) {
        wchar_t trail = text[*index];
        if (trail >= 0xDC00 && trail <= 0xDFFF) {
            (*index)++;
            return (((uint32_t)lead - 0xD800) << 10) + ((uint32_t)trail - 0xDC00) + 0x10000;
        }
    }
    return (uint32_t)lead;
}

static bool IsEmojiCodePoint(uint32_t cp) {
    if (cp >= 0x1F300 && cp <= 0x1F5FF) return true;
    if (cp >= 0x1F600 && cp <= 0x1F64F) return true;
    if (cp >= 0x1F680 && cp <= 0x1F6FF) return true;
    if (cp >= 0x1F900 && cp <= 0x1F9FF) return true;
    if (cp >= 0x1FA70 && cp <= 0x1FAFF) return true;
    if (cp >= 0x2600 && cp <= 0x27BF) return true;
    if (cp >= 0x1F1E6 && cp <= 0x1F1FF) return true;
    return false;
}

static bool HasEmojiStarter(const std::wstring& text) {
    for (size_t i = 0; i < text.size();) {
        const uint32_t cp = NextCodePoint(text, &i);
        if (cp == 0) break;
        if (IsEmojiCodePoint(cp)) return true;
    }
    return false;
}

TextEffect::TextEffect(const TextConfig& config, const std::string& themeName) : config_(config) {
    isChromatic_ = (ToLowerAscii(themeName) == "chromatic");
}

TextEffect::~TextEffect() {
    Shutdown();
}

bool TextEffect::Initialize() {
    return true;
}

void TextEffect::Shutdown() {
    pool_.Shutdown();
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

    // Keep emoji support parity with legacy D2D text path.
    if (HasEmojiStarter(text)) {
        if (!pool_.Initialize(15)) return;
        pool_.ShowText(event.pt, text, color, config_);
        return;
    }

    if (OverlayHostService::Instance().ShowText(event.pt, text, color, config_)) {
        return;
    }

    if (!pool_.Initialize(15)) return;
    pool_.ShowText(event.pt, text, color, config_);
}

} // namespace mousefx

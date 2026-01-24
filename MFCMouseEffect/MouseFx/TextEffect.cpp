#include "pch.h"
#include "TextEffect.h"
#include <random>

namespace mousefx {

static int RandomRange(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

TextEffect::TextEffect(const TextConfig& config) : config_(config) {}

TextEffect::~TextEffect() {
    Shutdown();
}

bool TextEffect::Initialize() {
    return pool_.Initialize(15);
}

void TextEffect::Shutdown() {
    pool_.Shutdown();
}

void TextEffect::OnClick(const ClickEvent& event) {
    if (config_.texts.empty()) return;

    // Pick random text and color
    const std::wstring& text = config_.texts[RandomRange(0, (int)config_.texts.size() - 1)];
    
    Argb color = { 0xFFFF69B4 }; // Default
    if (!config_.colors.empty()) {
        color = config_.colors[RandomRange(0, (int)config_.colors.size() - 1)];
    }

    pool_.ShowText(event.pt, text, color, config_);
}

} // namespace mousefx

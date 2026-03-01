#pragma once

#include "MouseFx/Core/Config/EffectConfig.h"

#include <array>

namespace mousefx::wasm::render_values {

inline bool HasVisibleAlpha(uint32_t colorArgb) {
    return ((colorArgb >> 24) & 0xFFu) != 0u;
}

inline std::wstring ResolveTextById(const TextConfig& config, uint32_t textId) {
    if (!config.texts.empty()) {
        const size_t index = static_cast<size_t>(textId % static_cast<uint32_t>(config.texts.size()));
        return config.texts[index];
    }
    static const std::array<std::wstring, 3> kFallbackTexts = {
        L"WASM",
        L"MouseFx",
        L"Click",
    };
    const size_t index = static_cast<size_t>(textId % static_cast<uint32_t>(kFallbackTexts.size()));
    return kFallbackTexts[index];
}

inline uint32_t ResolveTextColorArgb(const TextConfig& config, uint32_t textId, uint32_t commandColorArgb) {
    if (HasVisibleAlpha(commandColorArgb)) {
        return commandColorArgb;
    }
    if (!config.colors.empty()) {
        const size_t index = static_cast<size_t>(textId % static_cast<uint32_t>(config.colors.size()));
        return config.colors[index].value;
    }
    return 0xFFFFFFFFu;
}

inline uint32_t ResolveImageTintArgb(const IconConfig& config, uint32_t commandTintArgb) {
    if (HasVisibleAlpha(commandTintArgb)) {
        return commandTintArgb;
    }
    return config.fillColor.value;
}

} // namespace mousefx::wasm::render_values

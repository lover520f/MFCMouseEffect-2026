#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmCommandRenderResolvers.h"

#include "MouseFx/Core/Wasm/WasmPluginImageAssetCatalog.h"

#include <string>

namespace mousefx::platform::macos::wasm_render_resolver {

namespace {

constexpr uint32_t kFallbackWhiteArgb = 0xFFFFFFFFu;

} // namespace

bool HasVisibleAlpha(uint32_t argb) {
    return ((argb >> 24) & 0xFFu) != 0u;
}

std::wstring ResolveImageAssetPath(
    const std::wstring& activeManifestPath,
    uint32_t imageId) {
    std::wstring imagePath;
    std::string ignoredError;
    if (activeManifestPath.empty()) {
        return {};
    }
    if (!mousefx::wasm::WasmPluginImageAssetCatalog::ResolveImageAssetPath(
            activeManifestPath,
            imageId,
            &imagePath,
            &ignoredError)) {
        return {};
    }
    return imagePath;
}

std::wstring ResolveTextById(const mousefx::EffectConfig& config, uint32_t textId) {
    if (!config.textClick.texts.empty()) {
        const size_t idx = static_cast<size_t>(textId % static_cast<uint32_t>(config.textClick.texts.size()));
        return config.textClick.texts[idx];
    }
    static const std::wstring kFallbackTexts[] = {L"WASM", L"MouseFx", L"Click"};
    const size_t idx = static_cast<size_t>(textId % static_cast<uint32_t>(std::size(kFallbackTexts)));
    return kFallbackTexts[idx];
}

uint32_t ResolveTextColorArgb(const mousefx::EffectConfig& config, uint32_t textId, uint32_t commandColorArgb) {
    if (HasVisibleAlpha(commandColorArgb)) {
        return commandColorArgb;
    }
    if (!config.textClick.colors.empty()) {
        const size_t idx = static_cast<size_t>(textId % static_cast<uint32_t>(config.textClick.colors.size()));
        return config.textClick.colors[idx].value;
    }
    return kFallbackWhiteArgb;
}

uint32_t ResolveImageTintArgb(const mousefx::EffectConfig& config, uint32_t commandTintArgb) {
    if (HasVisibleAlpha(commandTintArgb)) {
        return commandTintArgb;
    }
    return config.icon.fillColor.value;
}

} // namespace mousefx::platform::macos::wasm_render_resolver

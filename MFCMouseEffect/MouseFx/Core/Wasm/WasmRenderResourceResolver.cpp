#include "pch.h"

#include "WasmRenderResourceResolver.h"

#include "WasmImageFileRenderer.h"
#include "WasmPluginImageAssetCatalog.h"
#include "MouseFx/Renderers/RendererRegistry.h"

#include <array>

namespace mousefx::wasm {

namespace {

bool HasVisibleAlpha(uint32_t colorArgb) {
    return ((colorArgb >> 24) & 0xFFu) != 0;
}

const std::array<std::wstring, 3>& BuiltinFallbackTexts() {
    static const std::array<std::wstring, 3> kTexts = {
        L"WASM",
        L"MouseFx",
        L"Click",
    };
    return kTexts;
}

const std::array<const char*, 2>& BuiltinImageRendererKeys() {
    static const std::array<const char*, 2> kKeys = {
        "star",
        "ripple",
    };
    return kKeys;
}

} // namespace

std::wstring WasmRenderResourceResolver::ResolveTextById(const EffectConfig& config, uint32_t textId) {
    if (!config.textClick.texts.empty()) {
        const size_t index = static_cast<size_t>(textId % static_cast<uint32_t>(config.textClick.texts.size()));
        return config.textClick.texts[index];
    }

    const auto& fallback = BuiltinFallbackTexts();
    const size_t index = static_cast<size_t>(textId % static_cast<uint32_t>(fallback.size()));
    return fallback[index];
}

Argb WasmRenderResourceResolver::ResolveTextColor(
    const EffectConfig& config,
    uint32_t textId,
    uint32_t commandColorArgb) {
    if (HasVisibleAlpha(commandColorArgb)) {
        return Argb{commandColorArgb};
    }
    if (!config.textClick.colors.empty()) {
        const size_t index = static_cast<size_t>(textId % static_cast<uint32_t>(config.textClick.colors.size()));
        return config.textClick.colors[index];
    }
    return Argb{0xFFFFFFFFu};
}

std::unique_ptr<IRippleRenderer> WasmRenderResourceResolver::CreateImageRendererById(
    uint32_t imageId,
    const std::wstring& manifestPath,
    uint32_t commandTintArgb,
    bool applyTint,
    float alphaScale,
    std::string* outRendererKey) {
    std::wstring imagePath;
    if (WasmPluginImageAssetCatalog::ResolveImageAssetPath(manifestPath, imageId, &imagePath, nullptr)) {
        auto renderer = std::make_unique<WasmImageFileRenderer>(
            imagePath,
            commandTintArgb,
            applyTint,
            alphaScale);
        if (outRendererKey) {
            *outRendererKey = "plugin:image_file";
        }
        return renderer;
    }

    const auto& keys = BuiltinImageRendererKeys();
    const char* preferred = keys[static_cast<size_t>(imageId % static_cast<uint32_t>(keys.size()))];

    auto renderer = RendererRegistry::Instance().Create(preferred);
    if (renderer) {
        if (outRendererKey) {
            *outRendererKey = preferred;
        }
        return renderer;
    }

    for (const char* key : keys) {
        renderer = RendererRegistry::Instance().Create(key);
        if (!renderer) {
            continue;
        }
        if (outRendererKey) {
            *outRendererKey = key;
        }
        return renderer;
    }

    if (outRendererKey) {
        outRendererKey->clear();
    }
    return nullptr;
}

Argb WasmRenderResourceResolver::ResolveImageTint(
    const EffectConfig& config,
    uint32_t imageId,
    uint32_t tintArgb) {
    (void)imageId;
    if (HasVisibleAlpha(tintArgb)) {
        return Argb{tintArgb};
    }
    return config.icon.fillColor;
}

} // namespace mousefx::wasm

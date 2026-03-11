#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmCommandRenderResolvers.h"

#include "MouseFx/Core/Wasm/WasmDynamicTextLabelScope.h"
#include "MouseFx/Core/Wasm/WasmPluginAbi.h"
#include "MouseFx/Core/Wasm/WasmRenderValueResolver.h"
#include "MouseFx/Core/Wasm/WasmPluginImageAssetCatalog.h"

namespace mousefx::platform::macos::wasm_render_resolver {

bool HasVisibleAlpha(uint32_t argb) {
    return mousefx::wasm::render_values::HasVisibleAlpha(argb);
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
    if (textId == mousefx::wasm::kTextIdEventLabel) {
        const std::wstring dynamicLabel = mousefx::wasm::ResolveWasmDynamicTextLabel();
        if (!dynamicLabel.empty()) {
            return dynamicLabel;
        }
    }
    return mousefx::wasm::render_values::ResolveTextById(config.textClick, textId);
}

uint32_t ResolveTextColorArgb(const mousefx::EffectConfig& config, uint32_t textId, uint32_t commandColorArgb) {
    return mousefx::wasm::render_values::ResolveTextColorArgb(config.textClick, textId, commandColorArgb);
}

uint32_t ResolveImageTintArgb(const mousefx::EffectConfig& config, uint32_t commandTintArgb) {
    return mousefx::wasm::render_values::ResolveImageTintArgb(config.icon, commandTintArgb);
}

} // namespace mousefx::platform::macos::wasm_render_resolver

#pragma once

#include <cstdint>
#include <memory>
#include <string>

#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Interfaces/IRippleRenderer.h"

namespace mousefx::wasm {

class WasmRenderResourceResolver final {
public:
    static std::wstring ResolveTextById(const EffectConfig& config, uint32_t textId);
    static Argb ResolveTextColor(const EffectConfig& config, uint32_t textId, uint32_t commandColorArgb);
    static std::unique_ptr<IRippleRenderer> CreateImageRendererById(
        uint32_t imageId,
        const std::wstring& manifestPath,
        uint32_t commandTintArgb,
        bool applyTint,
        float alphaScale,
        std::string* outRendererKey);
    static Argb ResolveImageTint(const EffectConfig& config, uint32_t imageId, uint32_t tintArgb);
};

} // namespace mousefx::wasm

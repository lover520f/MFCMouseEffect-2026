#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Internal.h"

#include "Platform/macos/Wasm/MacosWasmCommandRenderResolvers.h"
#include "Platform/macos/Wasm/MacosWasmTransientOverlay.h"
#include "MouseFx/Core/Wasm/WasmPluginAbi.h"

#include <cmath>
#include <cstring>

namespace mousefx::platform::macos::wasm_render_dispatch {

bool HandleSpawnImageCommand(
    const uint8_t* raw,
    const mousefx::EffectConfig& config,
    const std::wstring& activeManifestPath,
    mousefx::wasm::CommandExecutionResult* outResult,
    ThrottleCounters* outThrottleCounters) {
    mousefx::wasm::SpawnImageCommandV1 cmd{};
    std::memcpy(&cmd, raw, sizeof(cmd));
    WasmImageOverlayRequest request{};
    request.screenPt.x = static_cast<int32_t>(std::lround(cmd.x));
    request.screenPt.y = static_cast<int32_t>(std::lround(cmd.y));
    request.assetPath = wasm_render_resolver::ResolveImageAssetPath(activeManifestPath, cmd.imageId);
    request.tintArgb = wasm_render_resolver::ResolveImageTintArgb(config, cmd.tintRgba);
    request.scale = cmd.scale;
    request.alpha = cmd.alpha;
    request.lifeMs = cmd.lifeMs;
    request.delayMs = cmd.delayMs;
    request.velocityX = cmd.vx;
    request.velocityY = cmd.vy;
    request.accelerationX = cmd.ax;
    request.accelerationY = cmd.ay;
    request.rotationRad = cmd.rotation;
    request.applyTint = wasm_render_resolver::HasVisibleAlpha(cmd.tintRgba);
    const WasmOverlayRenderResult renderResult = ShowWasmImageOverlay(request);
    if (renderResult == WasmOverlayRenderResult::Rendered) {
        outResult->executedImageCommands += 1;
        outResult->renderedAny = true;
    } else if (!AccountThrottle(renderResult, false, outResult, outThrottleCounters)) {
        outResult->droppedCommands += 1;
        outResult->lastError = "failed to render spawn_image command";
    }
    return true;
}

bool HandleSpawnImageAffineCommand(
    const uint8_t* raw,
    const mousefx::EffectConfig& config,
    const std::wstring& activeManifestPath,
    mousefx::wasm::CommandExecutionResult* outResult,
    ThrottleCounters* outThrottleCounters) {
    mousefx::wasm::SpawnImageAffineCommandV1 cmd{};
    std::memcpy(&cmd, raw, sizeof(cmd));
    WasmImageOverlayRequest request{};
    request.screenPt.x = static_cast<int32_t>(std::lround(cmd.base.x + cmd.affineDx));
    request.screenPt.y = static_cast<int32_t>(std::lround(cmd.base.y + cmd.affineDy));
    request.assetPath = wasm_render_resolver::ResolveImageAssetPath(activeManifestPath, cmd.base.imageId);
    request.tintArgb = wasm_render_resolver::ResolveImageTintArgb(config, cmd.base.tintRgba);
    request.scale = cmd.base.scale;
    request.alpha = cmd.base.alpha;
    request.lifeMs = cmd.base.lifeMs;
    request.delayMs = cmd.base.delayMs;
    request.velocityX = cmd.base.vx;
    request.velocityY = cmd.base.vy;
    request.accelerationX = cmd.base.ax;
    request.accelerationY = cmd.base.ay;
    request.rotationRad = cmd.base.rotation;
    request.applyTint = wasm_render_resolver::HasVisibleAlpha(cmd.base.tintRgba);
    const WasmOverlayRenderResult renderResult = ShowWasmImageOverlay(request);
    if (renderResult == WasmOverlayRenderResult::Rendered) {
        outResult->executedImageCommands += 1;
        outResult->renderedAny = true;
    } else if (!AccountThrottle(renderResult, false, outResult, outThrottleCounters)) {
        outResult->droppedCommands += 1;
        outResult->lastError = "failed to render spawn_image_affine command";
    }
    return true;
}

} // namespace mousefx::platform::macos::wasm_render_dispatch

#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Internal.h"

#include "Platform/macos/Wasm/MacosWasmCommandRenderResolvers.h"
#include "Platform/macos/Wasm/MacosWasmTransientOverlay.h"
#include "MouseFx/Core/Wasm/WasmPluginAbi.h"

#include <cmath>
#include <cstring>

namespace mousefx::platform::macos::wasm_render_dispatch {

bool HandleSpawnTextCommand(
    const uint8_t* raw,
    const mousefx::EffectConfig& config,
    mousefx::wasm::CommandExecutionResult* outResult,
    ThrottleCounters* outThrottleCounters) {
    mousefx::wasm::SpawnTextCommandV1 cmd{};
    std::memcpy(&cmd, raw, sizeof(cmd));
    const ScreenPoint pt{
        static_cast<int32_t>(std::lround(cmd.x)),
        static_cast<int32_t>(std::lround(cmd.y)),
    };
    const std::wstring text = wasm_render_resolver::ResolveTextById(config, cmd.textId);
    const uint32_t color = wasm_render_resolver::ResolveTextColorArgb(config, cmd.textId, cmd.colorRgba);
    const WasmOverlayRenderResult renderResult =
        ShowWasmTextOverlay(pt, text, color, cmd.scale, cmd.lifeMs);
    if (renderResult == WasmOverlayRenderResult::Rendered) {
        outResult->executedTextCommands += 1;
        outResult->renderedAny = true;
    } else if (!AccountThrottle(renderResult, true, outResult, outThrottleCounters)) {
        outResult->droppedCommands += 1;
        outResult->lastError = "failed to render spawn_text command";
    }
    return true;
}

} // namespace mousefx::platform::macos::wasm_render_dispatch

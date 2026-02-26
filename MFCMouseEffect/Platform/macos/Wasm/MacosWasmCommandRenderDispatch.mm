#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmCommandRenderDispatch.h"
#include "Platform/macos/Wasm/MacosWasmCommandRenderDispatch.Internal.h"

#include <cstring>

namespace mousefx::platform::macos::wasm_render_dispatch {

bool AccountThrottle(
    WasmOverlayRenderResult renderResult,
    bool isText,
    mousefx::wasm::CommandExecutionResult* outResult,
    ThrottleCounters* outThrottleCounters) {
    if (!outResult || !outThrottleCounters) {
        return false;
    }

    if (renderResult == WasmOverlayRenderResult::ThrottledByCapacity) {
        outThrottleCounters->byCapacity += 1;
    } else if (renderResult == WasmOverlayRenderResult::ThrottledByInterval) {
        outThrottleCounters->byInterval += 1;
    } else {
        return false;
    }

    if (isText) {
        outThrottleCounters->text += 1;
    } else {
        outThrottleCounters->image += 1;
    }
    outResult->droppedCommands += 1;
    return true;
}

bool ExecuteParsedCommand(
    const mousefx::wasm::CommandRecord& record,
    const uint8_t* commandBuffer,
    size_t commandBytes,
    const mousefx::EffectConfig& config,
    const std::wstring& activeManifestPath,
    mousefx::wasm::CommandExecutionResult* outResult,
    ThrottleCounters* outThrottleCounters) {
    if (!commandBuffer || !outResult || !outThrottleCounters) {
        return false;
    }

    if (record.offsetBytes + record.sizeBytes > commandBytes) {
        outResult->droppedCommands += 1;
        return true;
    }

    const uint8_t* raw = commandBuffer + record.offsetBytes;
    switch (record.kind) {
    case mousefx::wasm::CommandKind::SpawnText:
        return HandleSpawnTextCommand(raw, config, outResult, outThrottleCounters);
    case mousefx::wasm::CommandKind::SpawnImage:
        return HandleSpawnImageCommand(raw, config, activeManifestPath, outResult, outThrottleCounters);
    case mousefx::wasm::CommandKind::SpawnImageAffine:
        return HandleSpawnImageAffineCommand(raw, config, activeManifestPath, outResult, outThrottleCounters);
    default:
        outResult->droppedCommands += 1;
        return true;
    }
}

void ApplyThrottleCounters(const ThrottleCounters& counters, mousefx::wasm::CommandExecutionResult* outResult) {
    if (!outResult) {
        return;
    }
    outResult->throttledCommands = counters.text + counters.image;
    outResult->throttledByCapacityCommands = counters.byCapacity;
    outResult->throttledByIntervalCommands = counters.byInterval;
}

} // namespace mousefx::platform::macos::wasm_render_dispatch

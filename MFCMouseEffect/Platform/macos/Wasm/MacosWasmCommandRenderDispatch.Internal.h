#pragma once

#include "Platform/macos/Wasm/MacosWasmCommandRenderDispatch.h"
#include "Platform/macos/Wasm/MacosWasmTransientOverlay.h"

namespace mousefx::platform::macos::wasm_render_dispatch {

bool AccountThrottle(
    WasmOverlayRenderResult renderResult,
    bool isText,
    mousefx::wasm::CommandExecutionResult* outResult,
    ThrottleCounters* outThrottleCounters);

bool HandleSpawnTextCommand(
    const uint8_t* raw,
    const mousefx::EffectConfig& config,
    mousefx::wasm::CommandExecutionResult* outResult,
    ThrottleCounters* outThrottleCounters);

bool HandleSpawnImageCommand(
    const uint8_t* raw,
    const mousefx::EffectConfig& config,
    const std::wstring& activeManifestPath,
    mousefx::wasm::CommandExecutionResult* outResult,
    ThrottleCounters* outThrottleCounters);

bool HandleSpawnImageAffineCommand(
    const uint8_t* raw,
    const mousefx::EffectConfig& config,
    const std::wstring& activeManifestPath,
    mousefx::wasm::CommandExecutionResult* outResult,
    ThrottleCounters* outThrottleCounters);

} // namespace mousefx::platform::macos::wasm_render_dispatch

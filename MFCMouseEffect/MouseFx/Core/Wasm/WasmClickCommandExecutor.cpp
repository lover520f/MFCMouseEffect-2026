#include "pch.h"

#include "WasmClickCommandExecutor.h"

#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "WasmCommandBufferParser.h"
#include "WasmPluginAbi.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace mousefx::wasm {

namespace {

std::wstring ResolveTextById(const TextConfig& config, uint32_t textId) {
    if (config.texts.empty()) {
        return L"WASM";
    }
    const size_t index = static_cast<size_t>(textId % static_cast<uint32_t>(config.texts.size()));
    return config.texts[index];
}

TextConfig BuildTextConfig(const TextConfig& base, const SpawnTextCommandV1& cmd) {
    TextConfig cfg = base;
    if (cmd.lifeMs > 0) {
        cfg.durationMs = std::clamp<int>(static_cast<int>(cmd.lifeMs), 80, 8000);
    }

    const float lifeSeconds = std::max(0.08f, static_cast<float>(cfg.durationMs) / 1000.0f);
    const float predictedDy = (cmd.vy * lifeSeconds) + (0.5f * cmd.ay * lifeSeconds * lifeSeconds);
    const float fallbackDy = std::abs(cmd.vy) * 0.55f;
    const float distance = std::max(std::abs(predictedDy), fallbackDy);
    cfg.floatDistance = std::clamp<int>(static_cast<int>(std::lround(distance)), 16, 420);

    if (cmd.scale > 0.0f) {
        const float scaledSize = cfg.fontSize * cmd.scale;
        cfg.fontSize = std::clamp(scaledSize, 6.0f, 90.0f);
    }
    return cfg;
}

void ExecuteSpawnText(
    const SpawnTextCommandV1& cmd,
    const EffectConfig& config,
    CommandExecutionResult* outResult) {
    if (!outResult) {
        return;
    }
    const TextConfig textCfg = BuildTextConfig(config.textClick, cmd);
    const POINT pt{
        static_cast<LONG>(std::lround(cmd.x)),
        static_cast<LONG>(std::lround(cmd.y)),
    };
    const std::wstring text = ResolveTextById(config.textClick, cmd.textId);
    const Argb color{cmd.colorRgba};

    if (!OverlayHostService::Instance().ShowText(pt, text, color, textCfg)) {
        outResult->lastError = "failed to render spawn_text command";
        outResult->droppedCommands += 1;
        return;
    }
    outResult->executedTextCommands += 1;
    outResult->renderedAny = true;
}

} // namespace

CommandExecutionResult WasmClickCommandExecutor::Execute(
    const uint8_t* commandBuffer,
    size_t commandBytes,
    const EffectConfig& config) {
    CommandExecutionResult result{};
    if (!commandBuffer || commandBytes == 0) {
        return result;
    }

    const CommandParseResult parsed = WasmCommandBufferParser::Parse(
        commandBuffer, commandBytes, 4096u);
    result.parsedCommands = static_cast<uint32_t>(parsed.commands.size());
    if (parsed.error != CommandParseError::None) {
        result.lastError = std::string("command parse failed: ") + CommandParseErrorToString(parsed.error);
        result.droppedCommands = result.parsedCommands;
        return result;
    }

    for (const auto& record : parsed.commands) {
        if (record.offsetBytes + record.sizeBytes > commandBytes) {
            result.droppedCommands += 1;
            continue;
        }

        const uint8_t* raw = commandBuffer + record.offsetBytes;
        switch (record.kind) {
        case CommandKind::SpawnText: {
            SpawnTextCommandV1 cmd{};
            std::memcpy(&cmd, raw, sizeof(cmd));
            ExecuteSpawnText(cmd, config, &result);
            break;
        }
        default:
            result.droppedCommands += 1;
            break;
        }
    }

    return result;
}

} // namespace mousefx::wasm

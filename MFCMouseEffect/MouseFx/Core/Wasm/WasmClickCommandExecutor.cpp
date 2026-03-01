#include "pch.h"

#include "WasmClickCommandExecutor.h"

#include "MouseFx/Core/Overlay/OverlayHostService.h"
#include "WasmCommandBufferParser.h"
#include "WasmImageCommandConfig.h"
#include "WasmImageRuntimeConfig.h"
#include "WasmPluginAbi.h"
#include "WasmRenderResourceResolver.h"
#include "WasmTextCommandConfig.h"

#include <algorithm>
#include <cmath>
#include <cstring>

namespace mousefx::wasm {

namespace {

void ExecuteSpawnText(
    const SpawnTextCommandV1& cmd,
    const EffectConfig& config,
    CommandExecutionResult* outResult) {
    if (!outResult) {
        return;
    }
    const TextConfig textCfg = BuildSpawnTextConfig(config.textClick, cmd);
    const ScreenPoint pt{
        static_cast<LONG>(std::lround(cmd.x)),
        static_cast<LONG>(std::lround(cmd.y)),
    };
    const std::wstring text = WasmRenderResourceResolver::ResolveTextById(config, cmd.textId);
    const Argb color = WasmRenderResourceResolver::ResolveTextColor(config, cmd.textId, cmd.colorRgba);

    if (!OverlayHostService::Instance().ShowText(pt, text, color, textCfg)) {
        outResult->lastError = "failed to render spawn_text command";
        outResult->droppedCommands += 1;
        return;
    }
    outResult->executedTextCommands += 1;
    outResult->renderedAny = true;
}

RippleStyle BuildImageStyle(const EffectConfig& config, const SpawnImageCommandV1& cmd) {
    RippleStyle style{};
    style.durationMs = ResolveSpawnImageLifeMs(cmd.lifeMs, config.icon.durationMs);
    const float scale = ResolveSpawnImageScale(cmd.scale);
    style.startRadius = std::max(2.0f, config.icon.startRadius * scale);
    style.endRadius = std::max(style.startRadius + 2.0f, config.icon.endRadius * scale);
    style.strokeWidth = std::max(0.8f, config.icon.strokeWidth);

    const float diameter = style.endRadius * 2.0f;
    style.windowSize = std::clamp<int>(static_cast<int>(std::ceil(diameter + 32.0f)), 64, 640);

    const Argb tint = WasmRenderResourceResolver::ResolveImageTint(config, cmd.imageId, cmd.tintRgba);
    style.fill = tint;
    style.stroke = tint;
    style.glow = Argb{(tint.value & 0x00FFFFFFu) | 0x44000000u};
    return style;
}

void ExecuteSpawnImage(
    const SpawnImageCommandV1& cmd,
    const EffectConfig& config,
    const std::wstring& activeManifestPath,
    CommandExecutionResult* outResult) {
    if (!outResult) {
        return;
    }

    ClickEvent ev{};
    ev.button = MouseButton::Left;
    ev.pt.x = static_cast<LONG>(std::lround(cmd.x));
    ev.pt.y = static_cast<LONG>(std::lround(cmd.y));

    RenderParams renderParams{};
    renderParams.loop = false;
    renderParams.intensity = ResolveSpawnImageAlpha(cmd.alpha);
    renderParams.directionRad = cmd.rotation;
    renderParams.velocityX = cmd.vx;
    renderParams.velocityY = cmd.vy;
    renderParams.accelerationX = cmd.ax;
    renderParams.accelerationY = cmd.ay;
    renderParams.useKinematics = (std::abs(cmd.vx) > 0.001f) || (std::abs(cmd.vy) > 0.001f) ||
                                 (std::abs(cmd.ax) > 0.001f) || (std::abs(cmd.ay) > 0.001f);
    renderParams.startDelayMs = ResolveSpawnImageDelayMs(cmd.delayMs);

    const RippleStyle style = BuildImageStyle(config, cmd);
    const bool applyTint = ResolveSpawnImageApplyTint(cmd.tintRgba);
    std::string rendererKey;
    std::unique_ptr<IRippleRenderer> renderer =
        WasmRenderResourceResolver::CreateImageRendererById(
            cmd.imageId,
            activeManifestPath,
            cmd.tintRgba,
            applyTint,
            renderParams.intensity,
            &rendererKey);
    if (!renderer) {
        outResult->lastError = "cannot resolve image renderer";
        outResult->droppedCommands += 1;
        return;
    }

    const uint64_t id = OverlayHostService::Instance().ShowRipple(
        ev, style, std::move(renderer), renderParams);
    if (id == 0) {
        outResult->lastError = "failed to render spawn_image command";
        outResult->droppedCommands += 1;
        return;
    }

    outResult->executedImageCommands += 1;
    outResult->renderedAny = true;
}

} // namespace

CommandExecutionResult WasmClickCommandExecutor::Execute(
    const uint8_t* commandBuffer,
    size_t commandBytes,
    const EffectConfig& config,
    const std::wstring& activeManifestPath) {
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
        case CommandKind::SpawnImage: {
            SpawnImageCommandV1 cmd{};
            std::memcpy(&cmd, raw, sizeof(cmd));
            ExecuteSpawnImage(ResolveSpawnImageCommand(cmd), config, activeManifestPath, &result);
            break;
        }
        case CommandKind::SpawnImageAffine: {
            SpawnImageAffineCommandV1 cmd{};
            std::memcpy(&cmd, raw, sizeof(cmd));
            ExecuteSpawnImage(ResolveSpawnImageCommand(cmd), config, activeManifestPath, &result);
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

#include "pch.h"

#include "Platform/macos/Wasm/MacosWasmCommandRenderer.h"

#include "Platform/macos/Wasm/MacosWasmCommandRenderDispatch.h"
#include "Platform/macos/Wasm/MacosWasmTransientOverlay.h"
#include "MouseFx/Core/Wasm/WasmCommandBufferParser.h"

#include <string>

namespace mousefx::platform::macos {

namespace {

constexpr uint32_t kParseCommandLimit = 4096u;

class MacosWasmCommandRenderer final : public mousefx::wasm::IWasmCommandRenderer {
public:
    ~MacosWasmCommandRenderer() override {
        CloseAllWasmOverlays();
    }

    bool SupportsRendering() const override {
        return true;
    }

    mousefx::wasm::CommandExecutionResult Execute(
        const uint8_t* commandBuffer,
        size_t commandBytes,
        const mousefx::EffectConfig& config,
        const std::wstring& activeManifestPath) override {
        mousefx::wasm::CommandExecutionResult result{};
        if (!commandBuffer || commandBytes == 0) {
            return result;
        }

        const mousefx::wasm::CommandParseResult parsed = mousefx::wasm::WasmCommandBufferParser::Parse(
            commandBuffer,
            commandBytes,
            kParseCommandLimit);
        result.parsedCommands = static_cast<uint32_t>(parsed.commands.size());
        if (parsed.error != mousefx::wasm::CommandParseError::None) {
            result.lastError = std::string("command parse failed: ") +
                mousefx::wasm::CommandParseErrorToString(parsed.error);
            result.droppedCommands = result.parsedCommands;
            return result;
        }

        wasm_render_dispatch::ThrottleCounters throttleCounters{};
        for (const auto& record : parsed.commands) {
            if (!wasm_render_dispatch::ExecuteParsedCommand(
                    record,
                    commandBuffer,
                    commandBytes,
                    config,
                    activeManifestPath,
                    &result,
                    &throttleCounters)) {
                result.droppedCommands += 1;
            }
        }

        wasm_render_dispatch::ApplyThrottleCounters(throttleCounters, &result);
        return result;
    }
};

} // namespace

std::unique_ptr<mousefx::wasm::IWasmCommandRenderer> CreateMacosWasmCommandRenderer() {
    return std::make_unique<MacosWasmCommandRenderer>();
}

} // namespace mousefx::platform::macos

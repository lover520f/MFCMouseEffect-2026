#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

#include "MouseFx/Core/Config/EffectConfig.h"

namespace mousefx::wasm {

struct CommandExecutionResult final {
    uint32_t parsedCommands = 0;
    uint32_t executedTextCommands = 0;
    uint32_t executedImageCommands = 0;
    uint32_t droppedCommands = 0;
    bool renderedAny = false;
    std::string lastError{};
};

class WasmClickCommandExecutor final {
public:
    static CommandExecutionResult Execute(
        const uint8_t* commandBuffer,
        size_t commandBytes,
        const EffectConfig& config);
};

} // namespace mousefx::wasm

#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "WasmPluginAbi.h"

namespace mousefx::wasm {

enum class CommandParseError : uint8_t {
    None = 0,
    TruncatedHeader,
    InvalidCommandSize,
    TruncatedCommand,
    UnsupportedCommandKind,
    CommandLimitExceeded,
};

struct CommandRecord final {
    CommandKind kind{CommandKind::SpawnText};
    uint32_t offsetBytes = 0;
    uint32_t sizeBytes = 0;
};

struct CommandParseResult final {
    std::vector<CommandRecord> commands{};
    CommandParseError error{CommandParseError::None};
    uint32_t consumedBytes = 0;
    uint32_t droppedBytes = 0;
};

const char* CommandParseErrorToString(CommandParseError error);

class WasmCommandBufferParser final {
public:
    static CommandParseResult Parse(const uint8_t* buffer, size_t bufferBytes, uint32_t maxCommands);
};

} // namespace mousefx::wasm


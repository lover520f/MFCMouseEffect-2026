#include "pch.h"

#include "WasmCommandBufferParser.h"

#include "WasmBinaryCodec.h"

namespace mousefx::wasm {

namespace {

bool IsSupportedKind(CommandKind kind, uint16_t commandSizeBytes) {
    switch (kind) {
    case CommandKind::SpawnText:
        return commandSizeBytes >= sizeof(SpawnTextCommandV1);
    case CommandKind::SpawnImage:
        return commandSizeBytes >= sizeof(SpawnImageCommandV1);
    case CommandKind::SpawnImageAffine:
        return commandSizeBytes >= sizeof(SpawnImageAffineCommandV1);
    default:
        return false;
    }
}

} // namespace

const char* CommandParseErrorToString(CommandParseError error) {
    switch (error) {
    case CommandParseError::None:
        return "none";
    case CommandParseError::TruncatedHeader:
        return "truncated_header";
    case CommandParseError::InvalidCommandSize:
        return "invalid_command_size";
    case CommandParseError::TruncatedCommand:
        return "truncated_command";
    case CommandParseError::UnsupportedCommandKind:
        return "unsupported_command_kind";
    case CommandParseError::CommandLimitExceeded:
        return "command_limit_exceeded";
    default:
        return "unknown";
    }
}

CommandParseResult WasmCommandBufferParser::Parse(const uint8_t* buffer, size_t bufferBytes, uint32_t maxCommands) {
    CommandParseResult result{};
    if (!buffer || bufferBytes == 0) {
        return result;
    }

    size_t offset = 0;
    while (offset < bufferBytes) {
        if (result.commands.size() >= static_cast<size_t>(maxCommands)) {
            result.error = CommandParseError::CommandLimitExceeded;
            result.consumedBytes = static_cast<uint32_t>(offset);
            result.droppedBytes = static_cast<uint32_t>(bufferBytes - offset);
            return result;
        }

        CommandHeaderV1 header{};
        if (!TryReadCommandHeaderV1(buffer, bufferBytes, offset, &header)) {
            result.error = CommandParseError::TruncatedHeader;
            result.consumedBytes = static_cast<uint32_t>(offset);
            result.droppedBytes = static_cast<uint32_t>(bufferBytes - offset);
            return result;
        }
        if (header.sizeBytes < sizeof(CommandHeaderV1)) {
            result.error = CommandParseError::InvalidCommandSize;
            result.consumedBytes = static_cast<uint32_t>(offset);
            result.droppedBytes = static_cast<uint32_t>(bufferBytes - offset);
            return result;
        }

        const size_t nextOffset = offset + static_cast<size_t>(header.sizeBytes);
        if (nextOffset > bufferBytes) {
            result.error = CommandParseError::TruncatedCommand;
            result.consumedBytes = static_cast<uint32_t>(offset);
            result.droppedBytes = static_cast<uint32_t>(bufferBytes - offset);
            return result;
        }

        const CommandKind kind = static_cast<CommandKind>(header.kind);
        if (!IsSupportedKind(kind, header.sizeBytes)) {
            result.error = CommandParseError::UnsupportedCommandKind;
            result.consumedBytes = static_cast<uint32_t>(offset);
            result.droppedBytes = static_cast<uint32_t>(bufferBytes - offset);
            return result;
        }

        result.commands.push_back(CommandRecord{
            kind,
            static_cast<uint32_t>(offset),
            static_cast<uint32_t>(header.sizeBytes),
        });
        offset = nextOffset;
    }

    result.consumedBytes = static_cast<uint32_t>(offset);
    return result;
}

} // namespace mousefx::wasm


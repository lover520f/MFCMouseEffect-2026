#include "pch.h"

#include "WasmBinaryCodec.h"

#include <cstring>

namespace mousefx::wasm {

std::array<uint8_t, sizeof(ClickInputV1)> SerializeClickInputV1(const ClickInputV1& input) {
    std::array<uint8_t, sizeof(ClickInputV1)> bytes{};
    std::memcpy(bytes.data(), &input, sizeof(input));
    return bytes;
}

bool TryReadCommandHeaderV1(
    const uint8_t* buffer,
    size_t bufferBytes,
    size_t offset,
    CommandHeaderV1* outHeader) {
    if (!buffer || !outHeader) {
        return false;
    }
    if (offset > bufferBytes) {
        return false;
    }
    const size_t remain = bufferBytes - offset;
    if (remain < sizeof(CommandHeaderV1)) {
        return false;
    }

    std::memcpy(outHeader, buffer + offset, sizeof(CommandHeaderV1));
    return true;
}

} // namespace mousefx::wasm


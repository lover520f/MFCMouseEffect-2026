#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

#include "WasmPluginAbi.h"

namespace mousefx::wasm {

std::array<uint8_t, sizeof(EventInputV1)> SerializeEventInputV1(const EventInputV1& input);
bool TryReadCommandHeaderV1(
    const uint8_t* buffer,
    size_t bufferBytes,
    size_t offset,
    CommandHeaderV1* outHeader);

} // namespace mousefx::wasm


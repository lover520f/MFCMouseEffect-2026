#pragma once

#include <cstdint>
#include <string>

namespace mousefx::wasm::wasm3_runtime_detail {

constexpr uint32_t kRuntimeStackBytes = 256u * 1024u;
constexpr uint32_t kLinearMemoryLimitBytes = 4u * 1024u * 1024u;
constexpr uint32_t kWasmPageBytes = 64u * 1024u;
constexpr uint32_t kScratchAlignment = 16u;

inline uint32_t AlignUp(uint32_t value, uint32_t alignment) {
    if (alignment == 0) {
        return value;
    }
    const uint32_t remainder = value % alignment;
    return remainder == 0 ? value : (value + alignment - remainder);
}

inline std::string BuildWasm3Error(const char* prefix, const char* wasm3Result) {
    std::string out;
    if (prefix && prefix[0] != '\0') {
        out.append(prefix);
        out.append(": ");
    }
    out.append(wasm3Result ? wasm3Result : "unknown wasm3 error");
    return out;
}

inline void SetOutError(std::string* outError, const std::string& value) {
    if (outError) {
        *outError = value;
    }
}

} // namespace mousefx::wasm::wasm3_runtime_detail

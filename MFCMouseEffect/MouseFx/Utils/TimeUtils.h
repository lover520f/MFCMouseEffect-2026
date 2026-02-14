#pragma once

#include <windows.h>
#include <cstdint>

namespace mousefx {

// Returns the current time in milliseconds (monotonic clock).
inline uint64_t NowMs() {
    return ::GetTickCount64();
}

} // namespace mousefx

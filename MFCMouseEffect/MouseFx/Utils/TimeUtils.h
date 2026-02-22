#pragma once

#include <chrono>
#include <cstdint>

namespace mousefx {

// Returns the current time in milliseconds (monotonic clock).
inline uint64_t NowMs() {
    using Clock = std::chrono::steady_clock;
    using Ms = std::chrono::milliseconds;
    return static_cast<uint64_t>(
        std::chrono::duration_cast<Ms>(Clock::now().time_since_epoch()).count());
}

} // namespace mousefx

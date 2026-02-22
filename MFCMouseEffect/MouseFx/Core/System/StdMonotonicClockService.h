#pragma once

#include "MouseFx/Core/System/IMonotonicClockService.h"

#include <chrono>

namespace mousefx {

// Standard C++ monotonic clock fallback.
class StdMonotonicClockService final : public IMonotonicClockService {
public:
    uint64_t NowMs() const override {
        using namespace std::chrono;
        const auto now = steady_clock::now().time_since_epoch();
        return static_cast<uint64_t>(duration_cast<milliseconds>(now).count());
    }
};

} // namespace mousefx

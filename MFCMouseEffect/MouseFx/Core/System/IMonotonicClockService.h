#pragma once

#include <cstdint>

namespace mousefx {

// Cross-platform monotonic clock abstraction (milliseconds).
class IMonotonicClockService {
public:
    virtual ~IMonotonicClockService() = default;

    virtual uint64_t NowMs() const = 0;
};

} // namespace mousefx

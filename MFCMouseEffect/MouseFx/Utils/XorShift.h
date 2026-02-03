#pragma once

#include <cstdint>

namespace mousefx {
namespace prng {

inline uint32_t Mix32(uint32_t x) {
    // Wang hash mix
    x = (x ^ 61u) ^ (x >> 16u);
    x = x + (x << 3u);
    x = x ^ (x >> 4u);
    x = x * 0x27d4eb2du;
    x = x ^ (x >> 15u);
    return x;
}

class XorShift32 final {
public:
    explicit XorShift32(uint32_t seed) : state_(seed ? seed : 0x12345678u) {}

    uint32_t NextU32() {
        uint32_t x = state_;
        x ^= x << 13u;
        x ^= x >> 17u;
        x ^= x << 5u;
        state_ = x;
        return x;
    }

    float Next01() {
        // 24-bit mantissa for stable float in [0,1)
        return (float)((NextU32() >> 8u) & 0x00FFFFFFu) * (1.0f / 16777216.0f);
    }

    float Range(float a, float b) {
        return a + (b - a) * Next01();
    }

private:
    uint32_t state_;
};

} // namespace prng
} // namespace mousefx


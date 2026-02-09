#pragma once

#include <ppl.h>

#include <utility>
#include <vector>

namespace mousefx::compute {

// Build a fixed-size result array from an index-based builder.
// The caller only provides the build function; threading policy is centralized here.
template <typename T, typename BuildFn>
std::vector<T> BuildArray(int count, int parallelThreshold, BuildFn&& buildFn) {
    if (count <= 0) return {};

    std::vector<T> out((size_t)count);
    if (count >= parallelThreshold) {
        Concurrency::parallel_for(0, count, [&](int i) {
            out[(size_t)i] = buildFn(i);
        });
    } else {
        for (int i = 0; i < count; ++i) {
            out[(size_t)i] = buildFn(i);
        }
    }
    return out;
}

} // namespace mousefx::compute

#pragma once

#include <ppl.h>

#include <thread>
#include <utility>
#include <vector>

namespace mousefx::compute {

enum class ParallelProfile {
    AggressiveSmallBatch, // Small collections that should still parallelize quickly.
    Balanced,             // Default for medium compute batches.
    Throughput,           // Prefer larger batches before parallel overhead.
};

inline int ResolveParallelThreshold(ParallelProfile profile) {
    switch (profile) {
        case ParallelProfile::AggressiveSmallBatch: return 2;
        case ParallelProfile::Balanced: return 4;
        case ParallelProfile::Throughput: return 8;
        default: return 4;
    }
}

inline bool ShouldRunParallel(int count, int parallelThreshold) {
    if (count <= 0 || count < parallelThreshold) return false;
    const unsigned int cores = std::thread::hardware_concurrency();
    return cores == 0 || cores > 1;
}

// Build a fixed-size result array from an index-based builder.
// The caller only provides the build function; threading policy is centralized here.
template <typename T, typename BuildFn>
std::vector<T> BuildArray(int count, int parallelThreshold, BuildFn&& buildFn) {
    if (count <= 0) return {};

    std::vector<T> out((size_t)count);
    if (ShouldRunParallel(count, parallelThreshold)) {
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

template <typename T, typename BuildFn>
std::vector<T> BuildArray(int count, ParallelProfile profile, BuildFn&& buildFn) {
    return BuildArray<T>(count, ResolveParallelThreshold(profile), std::forward<BuildFn>(buildFn));
}

} // namespace mousefx::compute

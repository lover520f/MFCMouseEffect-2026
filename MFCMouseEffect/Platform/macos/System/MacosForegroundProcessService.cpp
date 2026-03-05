#include "pch.h"

#include "Platform/macos/System/MacosForegroundProcessService.h"

#include "Platform/macos/System/MacosForegroundProcessSwiftBridge.h"

#include <array>
#include <chrono>
#include <string>

namespace mousefx {
namespace {

constexpr int32_t kProcessNameBufferCapacity = 1024;

uint64_t CurrentSteadyTickMs() {
    const auto now = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(now).count());
}

} // namespace

std::string MacosForegroundProcessService::CurrentProcessBaseName() {
    std::lock_guard<std::mutex> lock(mutex_);
    const uint64_t nowTickMs = CurrentSteadyTickMs();
    if ((nowTickMs - lastCheckTickMs_) < kCacheIntervalMs) {
        return lastProcessBaseName_;
    }

    lastCheckTickMs_ = nowTickMs;
    const std::string resolvedProcessName = QueryForegroundProcessBaseName();
    if (!resolvedProcessName.empty()) {
        lastProcessBaseName_ = resolvedProcessName;
    }
    return lastProcessBaseName_;
}

std::string MacosForegroundProcessService::QueryForegroundProcessBaseName() {
#if !defined(__APPLE__)
    return {};
#else
    std::array<char, kProcessNameBufferCapacity> nameBuffer{};
    const int32_t outcome = mfx_macos_resolve_foreground_process_name_v1(
        nameBuffer.data(),
        kProcessNameBufferCapacity);
    if (outcome <= 0) {
        return {};
    }
    return std::string(nameBuffer.data());
#endif
}

} // namespace mousefx

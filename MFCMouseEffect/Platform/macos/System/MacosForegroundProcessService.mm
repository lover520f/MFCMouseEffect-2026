#include "pch.h"

#include "Platform/macos/System/MacosForegroundProcessService.Internal.h"
#include "Platform/macos/System/MacosForegroundProcessService.h"

namespace mousefx {

std::string MacosForegroundProcessService::CurrentProcessBaseName() {
    std::lock_guard<std::mutex> lock(mutex_);
    const uint64_t nowTickMs = macos_foreground_process_detail::CurrentSteadyTickMs();
    if ((nowTickMs - lastCheckTickMs_) < kCacheIntervalMs) {
        return lastProcessBaseName_;
    }

    lastCheckTickMs_ = nowTickMs;
    lastProcessBaseName_ = QueryForegroundProcessBaseName();
    return lastProcessBaseName_;
}

std::string MacosForegroundProcessService::QueryForegroundProcessBaseName() {
    return macos_foreground_process_detail::ResolveForegroundProcessBaseName();
}

} // namespace mousefx

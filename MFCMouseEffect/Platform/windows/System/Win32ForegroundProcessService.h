#pragma once

#include "MouseFx/Core/System/IForegroundProcessService.h"

#include <cstdint>
#include <mutex>
#include <string>

namespace mousefx {

class Win32ForegroundProcessService final : public IForegroundProcessService {
public:
    std::string CurrentProcessBaseName() override;

private:
    static std::string QueryForegroundProcessBaseName();

    static constexpr uint64_t kCacheIntervalMs = 200;
    std::mutex mutex_{};
    uint64_t lastCheckTickMs_ = 0;
    std::string lastProcessBaseName_{};
};

} // namespace mousefx

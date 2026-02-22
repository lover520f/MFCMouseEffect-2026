#pragma once

#include "MouseFx/Core/System/IMonotonicClockService.h"

namespace mousefx {

class Win32MonotonicClockService final : public IMonotonicClockService {
public:
    uint64_t NowMs() const override;
};

} // namespace mousefx

#pragma once

#include "MouseFx/Core/System/IForegroundSuppressionService.h"

namespace mousefx {

class NullForegroundSuppressionService final : public IForegroundSuppressionService {
public:
    bool ShouldSuppress(uint64_t /*nowTickMs*/) override {
        return false;
    }
};

} // namespace mousefx

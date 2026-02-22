#pragma once

#include "MouseFx/Core/System/IForegroundProcessService.h"

namespace mousefx {

class NullForegroundProcessService final : public IForegroundProcessService {
public:
    std::string CurrentProcessBaseName() override {
        return {};
    }
};

} // namespace mousefx

#pragma once

#include <utility>

#include "MouseFx/Interfaces/ITrailEffectFallback.h"

namespace mousefx {

class NullTrailEffectFallback final : public ITrailEffectFallback {
public:
    bool Create() override { return false; }
    void Shutdown() override {}
    void Configure(bool, int, int, std::unique_ptr<ITrailRenderer>) override {}
    void AddPoint(const ScreenPoint&) override {}
};

} // namespace mousefx

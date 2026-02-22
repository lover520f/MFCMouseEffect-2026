#pragma once

#include "MouseFx/Interfaces/ITrailEffectFallback.h"
#include "MouseFx/Windows/TrailWindow.h"

namespace mousefx {

class Win32TrailEffectFallback final : public ITrailEffectFallback {
public:
    bool Create() override;
    void Shutdown() override;
    void Configure(bool isChromatic, int durationMs, int maxPoints, std::unique_ptr<ITrailRenderer> renderer) override;
    void AddPoint(const ScreenPoint& pt) override;

private:
    TrailWindow window_{};
    bool created_ = false;
};

} // namespace mousefx

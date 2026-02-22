#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Interfaces/ITrailEffectFallback.h"
#include <memory>

namespace mousefx {

class ITrailRenderer;
class TrailOverlayLayer;

class TrailEffect final : public IMouseEffect {
public:
    TrailEffect(const std::string& themeName, const std::string& type, int durationMs, int maxPoints, const TrailRendererParamsConfig& params);
    ~TrailEffect() override;

    EffectCategory Category() const override { return EffectCategory::Trail; }
    const char* TypeName() const override { return type_.c_str(); }

    bool Initialize() override;
    void Shutdown() override;
    void OnMouseMove(const ScreenPoint& pt) override;

private:
    std::unique_ptr<ITrailRenderer> CreateRenderer() const;

    std::unique_ptr<ITrailEffectFallback> fallback_;
    TrailOverlayLayer* hostLayer_ = nullptr;
    std::string type_;
    int durationMs_ = 350;
    int maxPoints_ = 40;
    TrailRendererParamsConfig params_{};
    bool isChromatic_ = false;
};

} // namespace mousefx

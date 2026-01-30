#pragma once

#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Windows/TrailWindow.h"
#include <memory>

namespace mousefx {

class TrailEffect final : public IMouseEffect {
public:
    TrailEffect(const std::string& themeName, const std::string& type = "line");
    ~TrailEffect() override;

    EffectCategory Category() const override { return EffectCategory::Trail; }
    const char* TypeName() const override { return type_.c_str(); }

    bool Initialize() override;
    void Shutdown() override;
    void OnMouseMove(const POINT& pt) override;

private:
    std::unique_ptr<TrailWindow> window_;
    std::string type_;
    bool isChromatic_ = false;
};

} // namespace mousefx

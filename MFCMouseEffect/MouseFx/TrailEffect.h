#pragma once

#include "IMouseEffect.h"
#include "TrailWindow.h"
#include <memory>

namespace mousefx {

class TrailEffect final : public IMouseEffect {
public:
    TrailEffect();
    ~TrailEffect() override;

    bool Initialize() override;
    void Shutdown() override;
    void OnClick(const ClickEvent& event) override;
    void OnMouseMove(const POINT& pt) override;

private:
    std::unique_ptr<TrailWindow> window_;
};

} // namespace mousefx

#pragma once

#include "MouseFx/Interfaces/IOverlayLayer.h"
#include "MouseFx/Interfaces/IRippleRenderer.h"
#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Styles/RippleStyle.h"

#include <memory>
#include <string>
#include <vector>

namespace mousefx {

class RippleOverlayLayer final : public IOverlayLayer {
public:
    RippleOverlayLayer() = default;
    ~RippleOverlayLayer() override = default;

    uint64_t ShowRipple(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params);
    uint64_t ShowContinuous(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params);
    void UpdatePosition(uint64_t id, const ScreenPoint& pt);
    void Stop(uint64_t id);
    bool IsActive(uint64_t id) const;
    void SendCommand(uint64_t id, const std::string& cmd, const std::string& args);
    void BroadcastCommand(const std::string& cmd, const std::string& args);

    void Update(uint64_t nowMs) override;
    void Render(Gdiplus::Graphics& graphics) override;
    bool IsAlive() const override { return true; }

private:
    struct RippleInstance {
        uint64_t id = 0;
        ClickEvent ev{};
        RippleStyle style{};
        RenderParams params{};
        std::unique_ptr<IRippleRenderer> renderer{};
        uint64_t startTick = 0;
        uint64_t rawElapsedMs = 0;
        uint64_t elapsedMs = 0;
        float t = 0.0f;
        bool renderReady = true;
        bool active = false;
        bool continuous = false;
    };

    ScreenPoint ResolveRenderCenter(const RippleInstance& instance) const;
    RippleInstance* FindById(uint64_t id);
    const RippleInstance* FindById(uint64_t id) const;

    std::vector<RippleInstance> instances_{};
    uint64_t nextId_ = 1;
};

} // namespace mousefx

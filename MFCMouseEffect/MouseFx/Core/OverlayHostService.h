#pragma once

#include <windows.h>
#include <memory>
#include <string>

#include "MouseFx/Core/GlobalMouseHook.h"
#include "MouseFx/Styles/RippleStyle.h"

namespace mousefx {

class ITrailRenderer;
class IRippleRenderer;
class IOverlayLayer;
class OverlayHostWindow;
class TrailOverlayLayer;
class ParticleTrailOverlayLayer;
class RippleOverlayLayer;
struct RenderParams;

class OverlayHostService final {
public:
    static OverlayHostService& Instance();

    bool Initialize();
    void Shutdown();

    IOverlayLayer* AttachLayer(std::unique_ptr<IOverlayLayer> layer);
    TrailOverlayLayer* AttachTrailLayer(std::unique_ptr<ITrailRenderer> renderer, int durationMs, int maxPoints, bool isChromatic);
    ParticleTrailOverlayLayer* AttachParticleTrailLayer(bool isChromatic);
    uint64_t ShowRipple(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params);
    uint64_t ShowContinuousRipple(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params);
    void UpdateRipplePosition(uint64_t id, const POINT& pt);
    void StopRipple(uint64_t id);
    bool IsRippleActive(uint64_t id) const;
    void SendRippleCommand(uint64_t id, const std::string& cmd, const std::string& args);
    void BroadcastRippleCommand(const std::string& cmd, const std::string& args);
    void DetachLayer(IOverlayLayer* layer);

private:
    OverlayHostService() = default;
    ~OverlayHostService() = default;

    OverlayHostService(const OverlayHostService&) = delete;
    OverlayHostService& operator=(const OverlayHostService&) = delete;

    std::unique_ptr<OverlayHostWindow> host_{};
    RippleOverlayLayer* rippleLayer_ = nullptr;

    RippleOverlayLayer* EnsureRippleLayer();
};

} // namespace mousefx

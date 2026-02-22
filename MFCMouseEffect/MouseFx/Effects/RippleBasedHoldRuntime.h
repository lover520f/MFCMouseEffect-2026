#pragma once

#include "MouseFx/Interfaces/IHoldRuntime.h"

#include <cstdint>
#include <string>

namespace mousefx {

/// Adapter that implements IHoldRuntime by delegating to
/// OverlayHostService's overlay-ripple system.
/// Covers both CPU-only and GPU V2 ripple paths.
class RippleBasedHoldRuntime : public IHoldRuntime {
public:
    /// @param rendererName  the renderer type to request from RendererRegistry
    ///                      (e.g. "charge", "hold_quantum_halo_gpu_v2")
    /// @param isGpuV2Route  true if this renderer uses GPU V2 hold_state commands
    /// @param isChromatic   true for chromatic (random color) mode
    explicit RippleBasedHoldRuntime(
        const std::string& rendererName,
        bool isGpuV2Route,
        bool isChromatic);

    ~RippleBasedHoldRuntime() override = default;

    bool Start(const RippleStyle& style, const ScreenPoint& pt) override;
    void Update(uint32_t holdMs, const ScreenPoint& pt) override;
    void Stop() override;
    bool IsRunning() const override;

private:
    void SendHoldStateCommand(uint32_t holdMs, const ScreenPoint& pt) const;
    void SendHoldEndCommand(const ScreenPoint& pt) const;

    std::string rendererName_;
    bool isGpuV2Route_ = false;
    bool isChromatic_ = false;
    uint64_t rippleId_ = 0;
    ScreenPoint lastSentPoint_{};
    bool hasLastSentPoint_ = false;
};

} // namespace mousefx

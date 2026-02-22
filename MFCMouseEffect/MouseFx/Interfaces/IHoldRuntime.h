#pragma once

#include "MouseFx/Core/Protocol/InputTypes.h"
#include "MouseFx/Styles/RippleStyle.h"

#include <cstdint>

namespace mousefx {

/// Abstract interface for hold-effect rendering strategies (Strategy Pattern).
/// Concrete implementations wrap either the overlay-ripple path or
/// direct GPU rendering backends.
class IHoldRuntime {
public:
    virtual ~IHoldRuntime() = default;

    /// Begin the hold effect with the given visual style at screen point.
    virtual bool Start(const RippleStyle& style, const ScreenPoint& pt) = 0;

    /// Called every frame while the button is held.
    /// @param holdMs   milliseconds since hold started.
    /// @param pt       current (possibly smoothed) cursor position.
    virtual void Update(uint32_t holdMs, const ScreenPoint& pt) = 0;

    /// End the hold effect (button released or shutdown).
    virtual void Stop() = 0;

    /// Whether the runtime is actively rendering.
    virtual bool IsRunning() const = 0;
};

} // namespace mousefx

#pragma once

#include <windows.h>
#include "GlobalMouseHook.h"

namespace mousefx {

// Interface for all mouse click effects.
class IMouseEffect {
public:
    virtual ~IMouseEffect() = default;

    // Called when the effect should initialize (e.g. create window pools).
    // Returns true on success.
    virtual bool Initialize() = 0;

    // Called when the effect should cleanup resources.
    virtual void Shutdown() = 0;

    // Called on mouse click events.
    virtual void OnClick(const ClickEvent& event) = 0;

    // Called on mouse move events (optional, default no-op).
    virtual void OnMouseMove(const POINT& pt) { (void)pt; }
};

} // namespace mousefx

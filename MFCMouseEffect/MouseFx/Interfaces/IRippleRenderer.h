#pragma once

#include <gdiplus.h>
#include <cstdint>
#include "MouseFx/Styles/RippleStyle.h"

namespace mousefx {

// Common render parameters
struct RenderParams {
    float directionRad = 0.0f;
    float intensity = 1.0f;
    bool loop = true;
};

class IRippleRenderer {
public:
    virtual ~IRippleRenderer() = default;

    // Called when the effect starts (or restarts for looping)
    virtual void Start(const RippleStyle& style) {}

    // Called every frame
    // t: normalized progress (0.0 to 1.0)
    // elapsedMs: time since start in milliseconds
    // sizePx: current window size in pixels
    // g: GDI+ graphics object to draw on
    virtual void Render(Gdiplus::Graphics& g, float t, uint64_t elapsedMs, int sizePx, const RippleStyle& style) = 0;

    // Handle external commands/interaction
    virtual void OnCommand(const std::string& cmd, const std::string& args) {}
};

} // namespace mousefx

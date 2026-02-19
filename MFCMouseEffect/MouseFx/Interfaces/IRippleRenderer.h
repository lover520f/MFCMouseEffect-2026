#pragma once

#include <gdiplus.h>
#include <cstdint>
#include <string>
#include "MouseFx/Styles/RippleStyle.h"

namespace mousefx {

// Common render parameters
struct RenderParams {
    float directionRad = 0.0f;
    float intensity = 1.0f;
    bool loop = true;

    // Optional motion model (screen-space, pixels / second^2).
    bool useKinematics = false;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    float accelerationX = 0.0f;
    float accelerationY = 0.0f;

    // Optional delay before an instance becomes visible.
    uint32_t startDelayMs = 0;
};

class IRippleRenderer {
public:
    virtual ~IRippleRenderer() = default;

    // Called when the effect starts (or restarts for looping)
    virtual void Start(const RippleStyle& style) {}

    // Optional: per-instance render params (direction/intensity/loop, etc.).
    // Called before Start() when available, and can be updated while running.
    virtual void SetParams(const RenderParams& params) {}

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

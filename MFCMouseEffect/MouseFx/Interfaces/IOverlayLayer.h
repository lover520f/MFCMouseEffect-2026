#pragma once

#include <cstdint>
#include <gdiplus.h>

#include "MouseFx/Gpu/OverlayGpuCommandStream.h"

namespace mousefx {

class IOverlayLayer {
public:
    virtual ~IOverlayLayer() = default;
    virtual void Update(uint64_t nowMs) = 0;
    virtual void Render(Gdiplus::Graphics& graphics) = 0;
    virtual bool IsAlive() const = 0;
    virtual bool IntersectsScreenRect(int left, int top, int right, int bottom) const {
        (void)left;
        (void)top;
        (void)right;
        (void)bottom;
        return true;
    }
    virtual void AppendGpuCommands(gpu::OverlayGpuCommandStream& stream, uint64_t nowMs) const {
        (void)stream;
        (void)nowMs;
    }
};

} // namespace mousefx

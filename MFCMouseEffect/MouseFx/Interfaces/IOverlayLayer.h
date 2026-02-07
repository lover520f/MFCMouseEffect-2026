#pragma once

#include <cstdint>
#include <gdiplus.h>

namespace mousefx {

class IOverlayLayer {
public:
    virtual ~IOverlayLayer() = default;
    virtual void Update(uint64_t nowMs) = 0;
    virtual void Render(Gdiplus::Graphics& graphics) = 0;
    virtual bool IsAlive() const = 0;
};

} // namespace mousefx

#pragma once

#include "MouseFx/Styles/RippleStyle.h"

#include <cstdint>
#include <string>

namespace mousefx {

struct QuantumHaloPresenterFrameArgs {
    int cursorScreenX = 0;
    int cursorScreenY = 0;
    int sizePx = 0;
    float t = 0.0f;
    uint64_t elapsedMs = 0;
    uint32_t holdMs = 0;
    const RippleStyle* style = nullptr;
};

class IQuantumHaloPresenterBackend {
public:
    virtual ~IQuantumHaloPresenterBackend() = default;

    virtual bool Start() = 0;
    virtual void Shutdown() = 0;
    virtual bool IsReady() const = 0;
    virtual const std::string& LastErrorReason() const = 0;
    virtual bool RenderFrame(const QuantumHaloPresenterFrameArgs& frame) = 0;
};

} // namespace mousefx

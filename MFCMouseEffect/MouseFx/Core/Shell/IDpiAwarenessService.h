#pragma once

namespace mousefx {

// Platform DPI-awareness bootstrap abstraction.
class IDpiAwarenessService {
public:
    virtual ~IDpiAwarenessService() = default;

    virtual void EnableForScreenCoords() = 0;
};

} // namespace mousefx

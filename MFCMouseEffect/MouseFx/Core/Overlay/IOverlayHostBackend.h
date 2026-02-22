#pragma once

#include <memory>

namespace mousefx {

class IOverlayLayer;

// Platform-specific overlay backend abstraction.
class IOverlayHostBackend {
public:
    virtual ~IOverlayHostBackend() = default;

    virtual bool Create() = 0;
    virtual void Shutdown() = 0;
    virtual IOverlayLayer* AddLayer(std::unique_ptr<IOverlayLayer> layer) = 0;
    virtual void RemoveLayer(IOverlayLayer* layer) = 0;
};

} // namespace mousefx

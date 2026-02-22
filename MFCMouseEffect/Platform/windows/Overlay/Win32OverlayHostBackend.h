#pragma once

#include <memory>

#include "MouseFx/Core/Overlay/IOverlayHostBackend.h"

namespace mousefx {

class Win32OverlayHostWindow;

class Win32OverlayHostBackend final : public IOverlayHostBackend {
public:
    Win32OverlayHostBackend();
    ~Win32OverlayHostBackend() override;

    Win32OverlayHostBackend(const Win32OverlayHostBackend&) = delete;
    Win32OverlayHostBackend& operator=(const Win32OverlayHostBackend&) = delete;

    bool Create() override;
    void Shutdown() override;
    IOverlayLayer* AddLayer(std::unique_ptr<IOverlayLayer> layer) override;
    void RemoveLayer(IOverlayLayer* layer) override;

private:
    std::unique_ptr<Win32OverlayHostWindow> hostWindow_{};
};

} // namespace mousefx

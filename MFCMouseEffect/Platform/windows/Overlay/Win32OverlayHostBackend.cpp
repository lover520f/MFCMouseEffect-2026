#include "pch.h"

#include "Platform/windows/Overlay/Win32OverlayHostBackend.h"
#include "Platform/windows/Overlay/Win32OverlayHostWindow.h"

namespace mousefx {

Win32OverlayHostBackend::Win32OverlayHostBackend()
    : hostWindow_(std::make_unique<Win32OverlayHostWindow>()) {}

Win32OverlayHostBackend::~Win32OverlayHostBackend() {
    Shutdown();
}

bool Win32OverlayHostBackend::Create() {
    return hostWindow_ && hostWindow_->Create();
}

void Win32OverlayHostBackend::Shutdown() {
    if (!hostWindow_) {
        return;
    }
    hostWindow_->Shutdown();
}

IOverlayLayer* Win32OverlayHostBackend::AddLayer(std::unique_ptr<IOverlayLayer> layer) {
    if (!hostWindow_) {
        return nullptr;
    }
    return hostWindow_->AddLayer(std::move(layer));
}

void Win32OverlayHostBackend::RemoveLayer(IOverlayLayer* layer) {
    if (!hostWindow_ || !layer) {
        return;
    }
    hostWindow_->RemoveLayer(layer);
}

} // namespace mousefx

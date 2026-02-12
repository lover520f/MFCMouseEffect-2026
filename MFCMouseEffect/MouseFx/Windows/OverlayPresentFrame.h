#pragma once

#include <windows.h>

#include <vector>

namespace mousefx::gpu {
class OverlayGpuCommandStream;
}

namespace mousefx {

class IOverlayLayer;

struct OverlayPresentFrame {
    HWND hwnd = nullptr;
    HDC memDc = nullptr;
    void* bits = nullptr;
    int width = 0;
    int height = 0;
    int surfaceX = 0;
    int surfaceY = 0;
    bool* hadVisibleContent = nullptr;
    const std::vector<IOverlayLayer*>* layers = nullptr;
    const gpu::OverlayGpuCommandStream* gpuCommandStream = nullptr;
};

} // namespace mousefx

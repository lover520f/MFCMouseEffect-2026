#include "pch.h"

#include "OverlayLayeredCpuPresenter.h"

#include <algorithm>

namespace mousefx {

bool OverlayLayeredCpuPresenter::Present(const OverlayPresentFrame& frame) {
    HWND hwnd = frame.hwnd;
    HDC memDc = frame.memDc;
    void* bits = frame.bits;
    const int width = frame.width;
    const int height = frame.height;
    const int surfaceX = frame.surfaceX;
    const int surfaceY = frame.surfaceY;
    bool* hadVisibleContent = frame.hadVisibleContent;
    const std::vector<IOverlayLayer*>* layers = frame.layers;
    if (!hwnd || !memDc || !bits || width <= 0 || height <= 0) return false;
    if (!hadVisibleContent || !layers) return false;

    const int surfaceLeft = surfaceX;
    const int surfaceTop = surfaceY;
    const int surfaceRight = surfaceX + width;
    const int surfaceBottom = surfaceY + height;

    std::vector<IOverlayLayer*> visibleLayers{};
    visibleLayers.reserve(layers->size());
    for (IOverlayLayer* layer : *layers) {
        if (layer && layer->IsAlive() &&
            layer->IntersectsScreenRect(surfaceLeft, surfaceTop, surfaceRight, surfaceBottom)) {
            visibleLayers.push_back(layer);
        }
    }
    const bool hasAnyLayerVisibleOnSurface = !visibleLayers.empty();
    if (!hasAnyLayerVisibleOnSurface && !(*hadVisibleContent)) {
        return true;
    }

    ZeroMemory(bits, (size_t)width * (size_t)height * 4);

    Gdiplus::Bitmap bmp(width, height, width * 4, PixelFormat32bppPARGB, static_cast<BYTE*>(bits));
    Gdiplus::Graphics graphics(&bmp);
    graphics.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    for (IOverlayLayer* layer : visibleLayers) {
        layer->Render(graphics);
    }

    POINT ptSrc{0, 0};
    SIZE sizeWnd{width, height};
    POINT ptDst{surfaceX, surfaceY};
    BLENDFUNCTION bf{AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};
    UpdateLayeredWindow(hwnd, nullptr, &ptDst, &sizeWnd, memDc, &ptSrc, 0, &bf, ULW_ALPHA);
    *hadVisibleContent = hasAnyLayerVisibleOnSurface;
    return true;
}

} // namespace mousefx

#pragma once

#include "MouseFx/Styles/RippleStyle.h"

#include <gdiplus.h>
#include <d2d1.h>
#include <dxgiformat.h>
#include <wrl/client.h>
#include <windows.h>

#include <atomic>
#include <cstdint>

namespace mousefx {

class FluxFieldHudGpuV2D2DBackend final {
public:
    void ResetSession();
    bool IsAvailable() const;

    bool Render(
        Gdiplus::Graphics& g,
        float t,
        uint64_t elapsedMs,
        int sizePx,
        const RippleStyle& style,
        uint32_t holdMs,
        bool hasCursorState,
        int cursorScreenX,
        int cursorScreenY);

private:
    bool EnsureResources();
    void MarkFailure();
    void MarkComExceptionFailure();

    Microsoft::WRL::ComPtr<ID2D1Factory> factory_{};
    Microsoft::WRL::ComPtr<ID2D1DCRenderTarget> target_{};
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> brush_{};
    int failureCount_ = 0;
    bool disabled_ = false;
    inline static std::atomic<bool> globalBlocked_{false};
};

} // namespace mousefx

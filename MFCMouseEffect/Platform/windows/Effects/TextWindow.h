#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <cstdint>
#include <wrl/client.h>
#include <d2d1.h>
#include <dwrite.h>

#include "MouseFx/Core/Effects/TextEffectCompute.h"
#include "MouseFx/Core/Protocol/InputTypes.h"

namespace mousefx {

class TextWindow final {
public:
    TextWindow() = default;
    ~TextWindow();

    TextWindow(const TextWindow&) = delete;
    TextWindow& operator=(const TextWindow&) = delete;

    bool Create();
    bool IsActive() const { return active_; }
    uint64_t StartTick() const { return startTick_; }

    void StartAtComputed(const ScreenPoint& anchorPoint, const TextEffectRenderCommand& command);

private:
    static constexpr UINT_PTR kTimerId = 4;
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    void OnTick();
    void RenderFrame(const TextEffectRenderFrame& frame);
    bool RenderEmojiBaseFrame();
    void PresentEmojiCachedFrame(const TextEffectRenderFrame& frame);
    void PresentBackbuffer(int left, int top, BYTE alpha);
    void EnsureSurface(int w, int h);
    void DestroySurface();
    bool EnsureD2DResources();
    bool EnsureTextLayout(float dpi, float widthDip, float heightDip);
    void DestroyD2DResources();
    void UpdateFrameTimerForPoint(const ScreenPoint& anchorPoint, bool force);

    static const wchar_t* ClassName();
    static bool EnsureClassRegistered();

    HWND hwnd_ = nullptr;
    bool active_ = false;

    TextEffectRenderCommand command_{};
    uint64_t startTick_ = 0;
    bool emojiColorMode_ = false;
    bool emojiFrameReady_ = false;
    int baseLeft_ = 0;
    int baseTop_ = 0;
    UINT frameTimerIntervalMs_ = 16;
    bool frameTimerArmed_ = false;

    // Layered window backbuffer.
    HDC memDc_ = nullptr;
    HBITMAP dib_ = nullptr;
    void* bits_ = nullptr;
    int width_ = 0;
    int height_ = 0;

    Microsoft::WRL::ComPtr<ID2D1DCRenderTarget> d2dTarget_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> d2dBrush_;
    Microsoft::WRL::ComPtr<IDWriteTextFormat> textFormat_;
    Microsoft::WRL::ComPtr<IDWriteTextLayout> textLayout_;
    float layoutDpi_ = 0.0f;
    float layoutWidthDip_ = 0.0f;
    float layoutHeightDip_ = 0.0f;
};

} // namespace mousefx

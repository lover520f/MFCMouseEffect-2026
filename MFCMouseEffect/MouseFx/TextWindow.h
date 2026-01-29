#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <cstdint>
#include <wrl/client.h>
#include <d2d1.h>
#include <dwrite.h>

#include "EffectConfig.h"

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

    void StartAt(const POINT& pt, const std::wstring& text, Argb color, const TextConfig& config);

private:
    static constexpr UINT_PTR kTimerId = 4;
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    void OnTick();
    void RenderFrame(float t);
    void EnsureSurface(int w, int h);
    void DestroySurface();
    bool EnsureD2DResources();
    void DestroyD2DResources();

    static const wchar_t* ClassName();
    static bool EnsureClassRegistered();

    HWND hwnd_ = nullptr;
    bool active_ = false;

    TextConfig config_{};
    std::wstring text_;
    Argb color_{};
    POINT startPt_{};
    uint64_t startTick_ = 0;
    float driftX_ = 0.0f;
    float swayFreq_ = 1.0f;
    float swayAmp_ = 0.0f;

    // Layered window backbuffer.
    HDC memDc_ = nullptr;
    HBITMAP dib_ = nullptr;
    void* bits_ = nullptr;
    int width_ = 0;
    int height_ = 0;

    Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory_;
    Microsoft::WRL::ComPtr<IDWriteFactory> dwriteFactory_;
    Microsoft::WRL::ComPtr<ID2D1DCRenderTarget> d2dTarget_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> d2dBrush_;
};

} // namespace mousefx

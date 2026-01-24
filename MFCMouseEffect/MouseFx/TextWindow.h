#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <cstdint>

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

    static const wchar_t* ClassName();
    static bool EnsureClassRegistered();

    HWND hwnd_ = nullptr;
    bool active_ = false;

    TextConfig config_{};
    std::wstring text_;
    Argb color_{};
    POINT startPt_{};
    uint64_t startTick_ = 0;

    // Layered window backbuffer.
    HDC memDc_ = nullptr;
    HBITMAP dib_ = nullptr;
    void* bits_ = nullptr;
    int width_ = 0;
    int height_ = 0;
};

} // namespace mousefx

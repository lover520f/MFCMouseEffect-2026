#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <cstdint>

#include "GlobalMouseHook.h"
#include "RippleStyle.h"

namespace mousefx {

class RippleWindow final {
public:
    RippleWindow() = default;
    ~RippleWindow();

    RippleWindow(const RippleWindow&) = delete;
    RippleWindow& operator=(const RippleWindow&) = delete;

    bool Create();
    bool IsActive() const { return active_; }
    uint64_t StartTick() const { return startTick_; }

    void StartAt(const ClickEvent& ev);
    void StartContinuous(const ClickEvent& ev);
    void UpdatePosition(const POINT& pt);
    void Stop();

    enum class DrawMode {
        Ripple,
        IconStar
    };
    void SetDrawMode(DrawMode mode) { drawMode_ = mode; }

private:
    static constexpr UINT_PTR kTimerId = 1;
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    void OnTick();
    void RenderFrame(float t);
    void EnsureSurface(int sizePx);
    void DestroySurface();

    static const wchar_t* ClassName();
    static bool EnsureClassRegistered();

    HWND hwnd_ = nullptr;
    bool active_ = false;
    bool continuous_ = false;

    RippleStyle style_{};
    ClickEvent current_{};
    uint64_t startTick_ = 0;

    // Layered window backbuffer.
    HDC memDc_ = nullptr;
    HBITMAP dib_ = nullptr;
    void* bits_ = nullptr;
    int sizePx_ = 0;

    DrawMode drawMode_ = DrawMode::Ripple;
};

} // namespace mousefx


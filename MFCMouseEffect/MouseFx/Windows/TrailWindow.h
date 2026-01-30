#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <deque>
#include <vector>

#include "MouseFx/Interfaces/ITrailRenderer.h"
#include <memory> 

namespace mousefx {

class TrailWindow final {
public:
    TrailWindow();
    ~TrailWindow();

    bool Create();
    void Shutdown();
    
    // Add a new point to the trail. 
    void AddPoint(const POINT& pt);
    
    // Clear trail immediately.
    void Clear();
    void SetChromatic(bool b) { isChromatic_ = b; }
    void SetColor(const Gdiplus::Color& c) { color_ = c; }

    void SetRenderer(std::unique_ptr<ITrailRenderer> renderer) {
        renderer_ = std::move(renderer);
    }

private:
    static constexpr UINT_PTR kTimerId = 2;
    bool isChromatic_ = false;
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    void OnTick();
    void Render();
    void UpdateLayered();
    void EnsureSurface(int w, int h);
    void DestroySurface();

    static const wchar_t* ClassName();
    static bool EnsureClassRegistered();

    HWND hwnd_ = nullptr;
    
    // Uses mousefx::TrailPoint from ITrailRenderer.h (namespace scope)
    std::deque<TrailPoint> points_;
    
    // Config
    int maxPoints_ = 40; // Increased for smoother trails
    int durationMs_ = 350; // trail lifetime
    Gdiplus::Color color_{ 220, 100, 255, 218 }; // Light Cyan/Greenish

    // Surface
    HDC memDc_ = nullptr;
    HBITMAP dib_ = nullptr;
    void* bits_ = nullptr;
    int width_ = 0;
    int height_ = 0;

    std::unique_ptr<ITrailRenderer> renderer_;
};

} // namespace mousefx

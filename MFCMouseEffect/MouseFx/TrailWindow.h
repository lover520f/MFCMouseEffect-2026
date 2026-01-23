#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <deque>
#include <vector>

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

private:
    static constexpr UINT_PTR kTimerId = 2;
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
    
    struct TrailPoint {
        POINT pt;
        uint64_t addedTime;
    };
    std::deque<TrailPoint> points_;
    
    // Config
    int maxPoints_ = 20;
    int durationMs_ = 350; // trail lifetime
    Gdiplus::Color color_{ 220, 100, 255, 218 }; // Light Cyan/Greenish

    // Surface
    HDC memDc_ = nullptr;
    HBITMAP dib_ = nullptr;
    void* bits_ = nullptr;
    int width_ = 0;
    int height_ = 0;
};

} // namespace mousefx

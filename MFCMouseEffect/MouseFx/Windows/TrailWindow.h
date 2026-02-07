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
    void SetMaxPoints(int n) {
        if (n < 2) n = 2;
        if (n > 240) n = 240;
        maxPoints_ = n;
    }
    void SetDurationMs(int ms) {
        if (ms < 80) ms = 80;
        if (ms > 2000) ms = 2000;
        durationMs_ = ms;
    }

    void SetRenderer(std::unique_ptr<ITrailRenderer> renderer) {
        renderer_ = std::move(renderer);
    }

private:
    static constexpr UINT_PTR kTimerId = 2;
    static constexpr UINT kMsgEnsureTopmost = WM_APP + 0x31;
    bool isChromatic_ = false;
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static void CALLBACK ForegroundEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD eventTime);

    LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    void OnTick();
    void Render();
    void UpdateLayered();
    void SampleCursorPoint(uint64_t nowMs);
    void EnsureSurface(int w, int h);
    void DestroySurface();
    void EnsureTopmostZOrder(bool force = false);
    void RegisterForegroundHook();
    void UnregisterForegroundHook();

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
    uint64_t lastTopmostEnsureMs_ = 0;
    HWINEVENTHOOK foregroundHook_ = nullptr;
    POINT latestCursorPt_{};
    bool hasLatestCursorPt_ = false;
    POINT lastSamplePt_{};
    bool hasLastSamplePt_ = false;

    std::unique_ptr<ITrailRenderer> renderer_;
};

} // namespace mousefx

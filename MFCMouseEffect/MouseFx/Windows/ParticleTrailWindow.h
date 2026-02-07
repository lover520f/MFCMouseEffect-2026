#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <memory>

namespace mousefx {

class ParticleTrailWindow final {
public:
    ParticleTrailWindow();
    ~ParticleTrailWindow();

    bool Create();
    void Shutdown();
    
    void Emit(const POINT& pt, int count = 5);
    void UpdateCursor(const POINT& pt);
    void Clear();
    void SetChromatic(bool b) { isChromatic_ = b; }

private:
    static constexpr UINT_PTR kTimerId = 3;
    static constexpr UINT kMsgEnsureTopmost = WM_APP + 0x32;
    bool isChromatic_ = false;
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static void CALLBACK ForegroundEventProc(HWINEVENTHOOK hook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD idEventThread, DWORD eventTime);

    LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    void OnTick();
    void Render();
    void UpdateLayered();
    void EnsureSurface(int w, int h);
    void DestroySurface();
    void EnsureTopmostZOrder(bool force = false);
    void RegisterForegroundHook();
    void UnregisterForegroundHook();

    struct Particle {
        float x, y;
        float vx, vy;
        float life;      // 1.0 (new) to 0.0 (dead)
        float hue;       // 0 to 360
        float size;
    };

    static const wchar_t* ClassName();
    static bool EnsureClassRegistered();

    HWND hwnd_ = nullptr;
    std::vector<Particle> particles_;
    uint64_t lastTick_ = 0;

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
    POINT lastEmitCursorPt_{};
    bool hasLastEmitCursorPt_ = false;
};

} // namespace mousefx

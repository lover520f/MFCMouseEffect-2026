#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <cstdint>
#include <vector>
#include <memory>

#include "MouseFx/Core/Effects/TrailEffectCompute.h"

namespace mousefx {

class ParticleTrailWindow final {
public:
    ParticleTrailWindow();
    ~ParticleTrailWindow();

    bool Create();
    void Shutdown();

    void AddCommand(const TrailEffectRenderCommand& command);
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
    void UpdateFrameTimerForPoint(const ScreenPoint* hintPoint, bool force);

    struct Particle {
        float x, y;
        float vx, vy;
        float life;      // 1.0 (new) to 0.0 (dead)
        float hue;       // 0 to 360
        float size;
        float renderRadiusPx;
        float renderOpacity;
        float decayScale;
        uint32_t baseArgb;
        bool useHue;
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
    UINT frameTimerIntervalMs_ = 16;
    bool frameTimerArmed_ = false;
    float globalHue_ = 0.0f;
    uint32_t rngState_ = 0x7F4A7C15u;

    void Emit(const TrailEffectRenderCommand& command, int count);
};

} // namespace mousefx

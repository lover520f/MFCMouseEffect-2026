#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <cstdint>
#include <vector>

#include "GlobalMouseHook.h"
#include "RippleStyle.h"

namespace mousefx {

class RippleWindow final {
public:
    RippleWindow() = default;
    ~RippleWindow();

    RippleWindow(const RippleWindow&) = delete;
    RippleWindow& operator=(const RippleWindow&) = delete;

    enum class DrawMode {
        Ripple,
        IconStar,
        ScrollChevron,
        ChargeRing,
        HoverCrosshair,
        LightningSingularity,
        HexForceField
    };

    struct RenderParams {
        float directionRad = 0.0f;
        float intensity = 1.0f;
        bool loop = true;
    };

    bool Create();
    bool IsActive() const { return active_; }
    uint64_t StartTick() const { return startTick_; }

    void StartAt(const ClickEvent& ev);
    void StartContinuous(const ClickEvent& ev);
    void StartAt(const ClickEvent& ev, const RippleStyle& style, DrawMode mode, const RenderParams& params);
    void StartContinuous(const ClickEvent& ev, const RippleStyle& style, DrawMode mode, const RenderParams& params);
    void UpdatePosition(const POINT& pt);
    void Stop();

    void SetDrawMode(DrawMode mode) { drawMode_ = mode; }
    void UpdateRenderParams(const RenderParams& params) { render_ = params; }

private:
    static constexpr UINT_PTR kTimerId = 1;
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    void OnTick();
    void RenderFrame(float t, uint64_t elapsedMs);
    void EnsureSurface(int sizePx);
    void DestroySurface();
    
    // For LightningSingularity
    struct LightningParticle {
        float angle;      // Approach angle
        float speed;      // Approach speed
        float phase;      // For flickering
        float distOffset; // Random start distance offset
        bool  active;
    };
    std::vector<LightningParticle> lightningParticles_;
    void InitLightningParticles();

    static const wchar_t* ClassName();
    static bool EnsureClassRegistered();

    HWND hwnd_ = nullptr;
    bool active_ = false;
    bool continuous_ = false;
    bool loop_ = true;

    RippleStyle style_{};
    ClickEvent current_{};
    RenderParams render_{};
    uint64_t startTick_ = 0;

    // Layered window backbuffer.
    HDC memDc_ = nullptr;
    HBITMAP dib_ = nullptr;
    void* bits_ = nullptr;
    int sizePx_ = 0;

    DrawMode drawMode_ = DrawMode::Ripple;
};

} // namespace mousefx

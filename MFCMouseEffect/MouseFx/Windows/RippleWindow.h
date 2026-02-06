#pragma once

#include <windows.h>
#include <gdiplus.h>
#include <cstdint>
#include <vector>
#include <memory>
#include <string>

#include "MouseFx/Core/GlobalMouseHook.h"
#include "MouseFx/Styles/RippleStyle.h"
#include "MouseFx/Interfaces/IRippleRenderer.h"

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

    void StartAt(const ClickEvent& ev, std::unique_ptr<IRippleRenderer> renderer);
    void StartContinuous(const ClickEvent& ev, std::unique_ptr<IRippleRenderer> renderer);
    
    // Note: RenderParams is now defined in IRippleRenderer.h
    void StartAt(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params);
    void StartContinuous(const ClickEvent& ev, const RippleStyle& style, std::unique_ptr<IRippleRenderer> renderer, const RenderParams& params);
    
    void UpdatePosition(const POINT& pt);
    void Stop();
    
    void SendCommand(const std::string& cmd, const std::string& args) {
        if (renderer_) renderer_->OnCommand(cmd, args);
    }

    void UpdateRenderParams(const RenderParams& params) {
        render_ = params;
        if (renderer_) renderer_->SetParams(render_);
        loop_ = render_.loop;
    }

private:
    static constexpr UINT_PTR kTimerId = 1;
    static const wchar_t* ClassName();
    static bool EnsureClassRegistered();
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    LRESULT OnMessage(UINT msg, WPARAM wParam, LPARAM lParam);
    void OnTick();
    void RenderFrame(float t, uint64_t elapsedMs);
    void EnsureSurface(int sizePx);
    void DestroySurface();

    HWND hwnd_ = nullptr;
    bool active_ = false;
    uint64_t startTick_ = 0;
    
    bool continuous_ = false;
    bool loop_ = false;
    
    RippleStyle style_{};
    RenderParams render_{}; // Stored params, sometimes used by window logic (like loop)
    ClickEvent current_{}; 
    
    std::unique_ptr<IRippleRenderer> renderer_;

    // Double buffering
    HDC memDc_ = nullptr;
    HBITMAP dib_ = nullptr;
    void* bits_ = nullptr;
    int sizePx_ = 0;
};

} // namespace mousefx

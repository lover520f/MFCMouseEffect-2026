#pragma once

#include <windows.h>
#include <memory>
#include <cstdint>
#include <vector>
#include <string>

#include "GdiPlusSession.h"
#include "GlobalMouseHook.h"
#include "IMouseEffect.h"
#include "EffectConfig.h"

namespace mousefx {

// Owns the subsystem lifecycle: message-only dispatcher, GDI+ init, hook, and current effect.
class AppController final {
public:
    AppController();
    ~AppController();

    AppController(const AppController&) = delete;
    AppController& operator=(const AppController&) = delete;

    enum class StartStage : uint8_t {
        None = 0,
        GdiPlusStartup,
        DispatchWindow,
        EffectInit,
        GlobalHook,
    };

    struct StartDiagnostics {
        StartStage stage{StartStage::None};
        DWORD error{ERROR_SUCCESS};
    };

    bool Start();
    void Stop();
    
    // Switch the active effect.
    // "ripple" -> RippleEffect
    // "trail" -> TrailEffect
    // "icon_star" -> IconEffect
    // "none" -> nullptr
    void SetEffect(const std::string& type);

    // Handle JSON command string.
    void HandleCommand(const std::string& jsonCmd);

    StartDiagnostics Diagnostics() const { return diag_; }
    
    // Get current config (for effects to read)
    const EffectConfig& Config() const { return config_; }

private:
    static LRESULT CALLBACK DispatchWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT OnDispatchMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    bool CreateDispatchWindow();
    void DestroyDispatchWindow();
    void ParseAndExecute(const std::string& line);

    HWND dispatchHwnd_ = nullptr;

    GdiPlusSession gdiplus_{};
    GlobalMouseHook hook_{};
    
    std::unique_ptr<IMouseEffect> currentEffect_;
    EffectConfig config_{};
    StartDiagnostics diag_{};

#ifdef _DEBUG
    uint32_t debugClickCount_ = 0;
#endif
};

} // namespace mousefx

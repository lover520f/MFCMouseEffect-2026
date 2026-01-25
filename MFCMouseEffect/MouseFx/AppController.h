#pragma once

#include <windows.h>
#include <memory>
#include <cstdint>
#include <array>
#include <string>

#include "GdiPlusSession.h"
#include "GlobalMouseHook.h"
#include "IMouseEffect.h"
#include "EffectConfig.h"

namespace mousefx {

// Owns the subsystem lifecycle: message-only dispatcher, GDI+ init, hook, and effects.
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
    
    // Set effect for a specific category.
    // type = "ripple", "star", "line", etc. or "none" to disable.
    void SetEffect(EffectCategory category, const std::string& type);
    
    // Clear (disable) effect for a category.
    void ClearEffect(EffectCategory category);

    // Set visual theme (affects themed effects).
    void SetTheme(const std::string& theme);

    // Set settings window UI language (persisted).
    void SetUiLanguage(const std::string& lang);
    
    // Set custom text content for Text Effect
    void SetTextEffectContent(const std::vector<std::wstring>& texts);

    // Get the current effect for a category (may be null).
    IMouseEffect* GetEffect(EffectCategory category) const;

    // Handle JSON command string.
    void HandleCommand(const std::string& jsonCmd);

    StartDiagnostics Diagnostics() const { return diag_; }
    
    // Get current config (for effects to read)
    // Get current config (for effects to read)
    const EffectConfig& Config() const { return config_; }

    // Reset settings to defaults
    void ResetConfig();

private:
    static LRESULT CALLBACK DispatchWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT OnDispatchMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    bool CreateDispatchWindow();
    void DestroyDispatchWindow();
    
    // Factory method to create effect by category and type name.
    std::unique_ptr<IMouseEffect> CreateEffect(EffectCategory category, const std::string& type);

    HWND dispatchHwnd_ = nullptr;

    GdiPlusSession gdiplus_{};
    GlobalMouseHook hook_{};
    
    // One effect slot per category.
    static constexpr size_t kCategoryCount = static_cast<size_t>(EffectCategory::Count);
    std::array<std::unique_ptr<IMouseEffect>, kCategoryCount> effects_{};
    
    EffectConfig config_{};
    std::wstring configDir_{};
    StartDiagnostics diag_{};

    uint64_t lastInputTime_ = 0;
    bool hovering_ = false;
    static constexpr UINT_PTR kHoverTimerId = 2;
    static constexpr DWORD kHoverThresholdMs = 2000;

    // Hold delay logic
    static constexpr UINT_PTR kHoldTimerId = 5;
    static constexpr DWORD kHoldDelayMs = 350; // Increased to 350ms to distinguish from click
    struct PendingHold {
        POINT pt;
        int button;
        bool active = false;
    } pendingHold_{};
    bool ignoreNextClick_ = false; // If hold triggered, ignore the subsequent click

#ifdef _DEBUG
    uint32_t debugClickCount_ = 0;
#endif
};

} // namespace mousefx

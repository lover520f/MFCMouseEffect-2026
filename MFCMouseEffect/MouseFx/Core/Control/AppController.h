#pragma once

#include <windows.h>
#include <memory>
#include <cstdint>
#include <array>
#include <string>
#include <vector>

#include "MouseFx/Core/System/GdiPlusSession.h"
#include "MouseFx/Core/System/IGlobalMouseHook.h"
#include "MouseFx/Core/Overlay/IInputIndicatorOverlay.h"
#include "MouseFx/Core/Automation/InputAutomationEngine.h"
#include "MouseFx/Core/Automation/ShortcutCaptureSession.h"
#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Core/Config/EffectConfig.h"
#include "MouseFx/Core/System/VmForegroundDetector.h"

namespace mousefx {

class CommandHandler;
class DispatchRouter;
namespace wasm {
class WasmEffectHost;
}

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
    // Set text click font size in point units.
    void SetTextEffectFontSize(float sizePt);
    void SetInputIndicatorConfig(const InputIndicatorConfig& cfg);
    void SetInputAutomationConfig(const InputAutomationConfig& cfg);
    // Set hold follow mode (precise|smooth|efficient).
    void SetHoldFollowMode(const std::string& mode);
    // Set hold presenter backend preference (auto or backend id).
    void SetHoldPresenterBackend(const std::string& backend);

    // Advanced tuning: trail history + renderer params (persisted).
    void SetTrailTuning(const std::string& style, const TrailProfilesConfig& profiles, const TrailRendererParamsConfig& params);

    // Get the current effect for a category (may be null).
    IMouseEffect* GetEffect(EffectCategory category) const;

    // Handle JSON command string.
    void HandleCommand(const std::string& jsonCmd);

    StartDiagnostics Diagnostics() const { return diag_; }
    
    // Get current config (for effects to read)
    const EffectConfig& Config() const { return config_; }
    EffectConfig GetConfigSnapshot() const;

    // Reset settings to defaults
    void ResetConfig();

    // --- Methods exposed for CommandHandler delegation ---
    void PersistConfig();
    void SetActiveEffectType(EffectCategory category, const std::string& type);
    void ReloadConfigFromDisk();
    std::string ResolveRuntimeEffectType(EffectCategory category, const std::string& requestedType, std::string* outReason) const;
    void SetWasmEnabled(bool enabled);
    void SetWasmFallbackToBuiltinClick(bool enabled);
    void SetWasmManifestPath(const std::string& manifestPath);
    void SetWasmCatalogRootPath(const std::string& catalogRootPath);
    void SetWasmExecutionBudget(uint32_t outputBufferBytes, uint32_t maxCommands, double maxExecutionMs);
    bool LoadWasmPluginFromManifestPath(const std::string& manifestPath);
    bool ShouldFallbackToBuiltinClickWhenWasmActive() const;
    
    // --- Methods exposed for DispatchRouter delegation ---
    void OnDispatchActivity(UINT msg, WPARAM wParam);
    bool IsVmEffectsSuppressed() const { return vmEffectsSuppressed_; }
    bool ConsumeIgnoreNextClick();
    void OnGlobalKey(const KeyEvent& ev);
    IInputIndicatorOverlay& IndicatorOverlay() { return *inputIndicatorOverlay_; }
    InputAutomationEngine& InputAutomation() { return inputAutomationEngine_; }
    bool ConsumeLatestMove(POINT* outPt);
    DWORD CurrentHoldDurationMs() const;
    void BeginHoldTracking(const POINT& pt, int button);
    void EndHoldTracking();
    void ClearPendingHold();
    void CancelPendingHold(HWND hwnd);
    bool ConsumePendingHold(POINT* outPt, int* outButton);
    void MarkIgnoreNextClick();
    bool TryEnterHover(POINT* outPt);
    HWND DispatchWindowHandle() const { return dispatchHwnd_; }
    std::string StartShortcutCaptureSession(uint64_t timeoutMs);
    void StopShortcutCaptureSession(const std::string& sessionId);
    ShortcutCaptureSession::PollResult PollShortcutCaptureSession(const std::string& sessionId);
    wasm::WasmEffectHost* WasmHost() const { return wasmEffectHost_.get(); }
    static constexpr UINT_PTR HoverTimerId() { return kHoverTimerId; }
    static constexpr UINT_PTR HoldTimerId() { return kHoldTimerId; }
    static constexpr DWORD HoldDelayMs() { return kHoldDelayMs; }
#ifdef _DEBUG
    void LogDebugClick(const ClickEvent& ev);
#else
    void LogDebugClick(const ClickEvent&) {}
#endif

private:
    static LRESULT CALLBACK DispatchWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    bool CreateDispatchWindow();
    void DestroyDispatchWindow();
    
    // Factory method to create effect by category and type name.
    std::unique_ptr<IMouseEffect> CreateEffect(EffectCategory category, const std::string& type);
    const std::string* ActiveTypeForCategory(EffectCategory category) const;
    std::string* MutableActiveTypeForCategory(EffectCategory category);
    bool IsActiveEffectEnabled(EffectCategory category) const;
    void ReapplyActiveEffect(EffectCategory category);
    std::string ResolveConfiguredClickType() const;
    void ApplyConfiguredEffects();
    bool NormalizeActiveEffectTypes();
    void InitializeWasmHost();
    void ShutdownWasmHost();
    void ApplyWasmConfigToHost(bool tryLoadManifest);


    void NotifyGpuFallbackIfNeeded(const std::string& reason);
    void WriteGpuRouteStatusSnapshot(EffectCategory category, const std::string& requestedType, const std::string& effectiveType, const std::string& reason) const;
    void UpdateVmSuppressionState();
    void ApplyVmSuppression(bool suppressed);
    void SuspendEffectsForVm();
    void ResumeEffectsAfterVm();

    HWND dispatchHwnd_ = nullptr;

    GdiPlusSession gdiplus_{};
    std::unique_ptr<IGlobalMouseHook> hook_{};
    
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

    bool holdButtonDown_ = false;
    uint64_t holdDownTick_ = 0;
    bool gpuFallbackNotifiedThisSession_ = false;
    std::unique_ptr<CommandHandler> commandHandler_;
    std::unique_ptr<DispatchRouter> dispatchRouter_;
    std::unique_ptr<IInputIndicatorOverlay> inputIndicatorOverlay_{};
    InputAutomationEngine inputAutomationEngine_{};
    mutable ShortcutCaptureSession shortcutCaptureSession_{};
    std::unique_ptr<wasm::WasmEffectHost> wasmEffectHost_{};
    VmForegroundDetector vmForegroundDetector_{};
    bool vmEffectsSuppressed_ = false;

#ifdef _DEBUG
    uint32_t debugClickCount_ = 0;
#endif
};

} // namespace mousefx

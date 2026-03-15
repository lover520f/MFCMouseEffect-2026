#pragma once

#include <array>
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "MouseFx/Core/Control/DispatchMessage.h"
#include "MouseFx/Core/Control/IDispatchMessageHandler.h"
#include "MouseFx/Core/Control/IDispatchMessageHost.h"
#include "MouseFx/Core/Control/IDispatchMessageCodec.h"
#include "MouseFx/Core/System/ICursorPositionService.h"
#include "MouseFx/Core/System/IMonotonicClockService.h"
#include "MouseFx/Core/System/IForegroundProcessService.h"
#include "MouseFx/Core/System/IForegroundSuppressionService.h"
#include "MouseFx/Core/System/IGlobalMouseHook.h"
#include "MouseFx/Core/System/IKeyboardInjector.h"
#include "MouseFx/Core/Overlay/IInputIndicatorOverlay.h"
#include "MouseFx/Core/Control/InputIndicatorWasmDispatchFeature.h"
#include "MouseFx/Core/Automation/InputAutomationEngine.h"
#include "MouseFx/Core/Automation/ShortcutCaptureSession.h"
#include "MouseFx/Interfaces/IMouseEffect.h"
#include "MouseFx/Core/Config/EffectConfig.h"

namespace mousefx {

class CommandHandler;
class DispatchRouter;
class GdiPlusSession;
namespace wasm {
class WasmEffectHost;
}

// Owns the subsystem lifecycle: message-only dispatcher, GDI+ init, hook, and effects.
class AppController final : public IDispatchMessageHandler {
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
        uint32_t error{0};
    };

    enum class InputCaptureFailureReason : uint8_t {
        None = 0,
        PermissionDenied,
        Unsupported,
        StartFailed,
    };

    struct InputCaptureRuntimeStatus {
        bool active{false};
        uint32_t error{0};
        InputCaptureFailureReason reason{InputCaptureFailureReason::None};
    };
    
    struct InputIndicatorWasmRouteStatus {
        std::string eventKind;
        std::string renderMode;
        std::string reason;
        uint64_t eventTickMs{0};
        bool routeAttempted{false};
        bool anchorsResolved{false};
        bool hostPresent{false};
        bool hostEnabled{false};
        bool pluginLoaded{false};
        bool eventSupported{false};
        bool invokeAttempted{false};
        bool renderedByWasm{false};
        bool wasmFallbackEnabled{false};
        bool nativeFallbackApplied{false};
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
    void SetRuntimeDiagnosticsEnabled(bool enabled);
    bool RuntimeDiagnosticsEnabled() const;
    // Set hold follow mode (precise|smooth|efficient).
    void SetHoldFollowMode(const std::string& mode);
    // Set hold presenter backend preference (auto or backend id).
    void SetHoldPresenterBackend(const std::string& backend);
    // Set overlay target FPS (0=auto max refresh, positive value=cap).
    void SetOverlayTargetFps(int targetFps);

    // Advanced tuning: trail history + renderer params (persisted).
    void SetTrailTuning(const std::string& style, const TrailProfilesConfig& profiles, const TrailRendererParamsConfig& params);
    // Set trail line width (persisted).
    void SetTrailLineWidth(float lineWidth);
    void SetEffectSizeScales(const EffectSizeScaleConfig& scales);
    void SetEffectConflictPolicy(const EffectConflictPolicyConfig& policy);
    void SetEffectsBlacklistApps(const std::vector<std::string>& apps);

    // Get the current effect for a category (may be null).
    IMouseEffect* GetEffect(EffectCategory category) const;

    // Handle JSON command string.
    void HandleCommand(const std::string& jsonCmd);

    StartDiagnostics Diagnostics() const { return diag_; }
    InputCaptureRuntimeStatus InputCaptureStatus() const;
    bool EffectsSuspendedByInputCapture() const;
    void SetInputCaptureStatusCallback(std::function<void(const InputCaptureRuntimeStatus&)> callback);
    
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
    void SetWasmManifestPathForChannel(const std::string& channel, const std::string& manifestPath);
    std::string ResolveWasmManifestPathForChannel(const std::string& channel) const;
    void SetWasmCatalogRootPath(const std::string& catalogRootPath);
    void SetThemeCatalogRootPath(const std::string& rootPath);
    void SetWasmExecutionBudget(uint32_t outputBufferBytes, uint32_t maxCommands, double maxExecutionMs);
    bool LoadWasmPluginFromManifestPath(
        const std::string& manifestPath,
        const std::string& surface = {},
        const std::string& effectChannel = {});
    bool ShouldFallbackToBuiltinClickWhenWasmActive() const;
    
    // --- Methods exposed for DispatchRouter delegation ---
    void OnDispatchActivity(DispatchMessageKind kind, uint32_t timerId);
    bool IsVmEffectsSuppressed() const { return vmEffectsSuppressed_; }
    uint64_t VmForegroundSuppressionCheckIntervalMs() const;
    bool ConsumeIgnoreNextClick();
    void OnGlobalKey(const KeyEvent& ev);
    void DispatchInputIndicatorClick(const ClickEvent& ev);
    void DispatchInputIndicatorScroll(const ScrollEvent& ev);
    void DispatchInputIndicatorKey(const KeyEvent& ev);
    InputIndicatorWasmRouteStatus ReadInputIndicatorWasmRouteStatus() const;
    IInputIndicatorOverlay& IndicatorOverlay() { return *inputIndicatorOverlay_; }
    InputAutomationEngine& InputAutomation() { return inputAutomationEngine_; }
    const InputAutomationEngine& InputAutomation() const { return inputAutomationEngine_; }
    bool ConsumeLatestMove(ScreenPoint* outPt);
    uint64_t CurrentTickMs() const;
    uint32_t CurrentHoldDurationMs() const;
    void BeginHoldTracking(const ScreenPoint& pt, int button);
    void EndHoldTracking();
    void ArmHoldTimer();
    void DisarmHoldTimer();
    void ArmHoldUpdateTimer();
    void DisarmHoldUpdateTimer();
    void ArmWasmFrameTimer();
    void DisarmWasmFrameTimer();
    void ClearPendingHold();
    void CancelPendingHold();
    bool ConsumePendingHold(ScreenPoint* outPt, int* outButton);
    void MarkIgnoreNextClick();
    bool IsHoldButtonDown() const { return holdButtonDown_; }
    int HoldTrackingButton() const { return holdTrackingButton_; }
    bool IsHovering() const { return hovering_; }
    bool TryEnterHover(ScreenPoint* outPt);
    bool QueryCursorScreenPoint(ScreenPoint* outPt) const;
    void RememberLastPointerPoint(const ScreenPoint& pt);
    bool TryGetLastPointerPoint(ScreenPoint* outPt) const;
    void KillDispatchTimer(uintptr_t timerId);
    std::string CurrentForegroundProcessBaseName();
    bool IsEffectsBlockedByAppBlacklist();
    bool InjectShortcutForTest(const std::string& chordText);
    std::string StartShortcutCaptureSession(uint64_t timeoutMs);
    void StopShortcutCaptureSession(const std::string& sessionId);
    ShortcutCaptureSession::PollResult PollShortcutCaptureSession(const std::string& sessionId);
    wasm::WasmEffectHost* WasmHost() const;
    wasm::WasmEffectHost* WasmIndicatorHost() const { return wasmIndicatorHost_.get(); }
    wasm::WasmEffectHost* WasmEffectsHostForChannel(const std::string& channel) const;
    wasm::WasmEffectHost* WasmHostForSurface(const std::string& surface) const;
    static constexpr uintptr_t HoverTimerId() { return kHoverTimerId; }
    static constexpr uintptr_t HoldTimerId() { return kHoldTimerId; }
    static constexpr uintptr_t HoldUpdateTimerId() { return kHoldUpdateTimerId; }
    static constexpr uintptr_t WasmFrameTimerId() { return kWasmFrameTimerId; }
    static constexpr uint32_t HoldDelayMs() { return kHoldDelayMs; }
#ifdef _DEBUG
    void LogDebugClick(const ClickEvent& ev);
#else
    void LogDebugClick(const ClickEvent&) {}
#endif

private:
    intptr_t OnDispatchMessage(
        uintptr_t sourceHandle,
        uint32_t msg,
        uintptr_t wParam,
        intptr_t lParam) override;
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
    void ApplyOverlayTargetFpsToPlatform();
    uint32_t ResolveWasmFrameTimerIntervalMs() const;
    bool NormalizeConfiguredThemeName();
    bool NormalizeActiveEffectTypes();
    void InitializeWasmHost();
    void ShutdownWasmHost();
    void ApplyWasmConfigToHost(bool tryLoadManifest);
    bool EnsureInputIndicatorWasmBudgetFloor();
    void SyncInputIndicatorWasmHostToConfig();


    void NotifyGpuFallbackIfNeeded(const std::string& reason);
    void WriteGpuRouteStatusSnapshot(EffectCategory category, const std::string& requestedType, const std::string& effectiveType, const std::string& reason) const;
    void NotifyInputCaptureStatusChanged();
    void RefreshInputCaptureRuntimeState();
    void EnterInputCaptureDegradedMode(uint32_t error);
    void UpdateVmSuppressionState();
    void ApplyVmSuppression(bool suppressed);
    void SuspendEffectsForVm();
    void ResumeEffectsAfterVm();
    static InputCaptureFailureReason ClassifyInputCaptureFailure(bool active, uint32_t error);
    void RecordInputIndicatorWasmRouteStatus(
        const char* eventKind,
        const InputIndicatorWasmRouteTrace& trace,
        bool renderedByWasm,
        bool wasmFallbackEnabled,
        bool nativeFallbackApplied);

    std::unique_ptr<GdiPlusSession> gdiplus_{};
    std::unique_ptr<IDispatchMessageHost> dispatchMessageHost_{};
    std::unique_ptr<IDispatchMessageCodec> dispatchMessageCodec_{};
    std::unique_ptr<ICursorPositionService> cursorPositionService_{};
    std::unique_ptr<IMonotonicClockService> monotonicClockService_{};
    std::unique_ptr<IForegroundProcessService> foregroundProcessService_{};
    std::unique_ptr<IForegroundSuppressionService> foregroundSuppressionService_{};
    std::unique_ptr<IGlobalMouseHook> hook_{};
    std::unique_ptr<IKeyboardInjector> keyboardInjector_{};
    
    // One effect slot per category.
    static constexpr size_t kCategoryCount = static_cast<size_t>(EffectCategory::Count);
    std::array<std::unique_ptr<IMouseEffect>, kCategoryCount> effects_{};
    
    EffectConfig config_{};
    std::wstring configDir_{};
    StartDiagnostics diag_{};
    std::atomic<bool> inputCaptureActive_{false};
    std::atomic<uint32_t> inputCaptureError_{0};
    std::atomic<bool> effectsSuspendedByInputCapture_{false};
    std::function<void(const InputCaptureRuntimeStatus&)> inputCaptureStatusCallback_{};
    mutable std::mutex inputCaptureStatusCallbackMutex_{};

    uint64_t lastInputTime_ = 0;
    ScreenPoint lastPointerPoint_{};
    bool hasLastPointerPoint_ = false;
    bool hovering_ = false;
    static constexpr uintptr_t kHoverTimerId = 2;
    static constexpr uint32_t kHoverThresholdMs = 2000;
    static constexpr uintptr_t kInputCaptureHealthTimerId = 6;
    static constexpr uintptr_t kWasmFrameTimerId = 10;

    // Hold delay logic
    static constexpr uintptr_t kHoldTimerId = 5;
    static constexpr uint32_t kHoldDelayMs = 350; // Increased to 350ms to distinguish from click
    static constexpr uintptr_t kHoldUpdateTimerId = 9;
    static constexpr uint32_t kHoldUpdateIntervalMs = 16; // ~60fps periodic hold overlay update
    struct PendingHold {
        ScreenPoint pt{};
        int button;
        bool active = false;
    } pendingHold_{};
    bool ignoreNextClick_ = false; // If hold triggered, ignore the subsequent click

    bool holdButtonDown_ = false;
    int holdTrackingButton_ = 0;
    uint64_t holdDownTick_ = 0;
    uint64_t lastInputCaptureRestartAttemptTickMs_ = 0;
    static constexpr uint32_t kInputCaptureRestartRetryMs = 1000;
    uint32_t inputCaptureTransientErrorCode_ = 0;
    uint64_t inputCaptureTransientErrorSinceTickMs_ = 0;
    static constexpr uint32_t kInputCaptureTransientErrorGraceMs = 1200;
    bool gpuFallbackNotifiedThisSession_ = false;
    std::unique_ptr<CommandHandler> commandHandler_;
    std::unique_ptr<DispatchRouter> dispatchRouter_;
    std::unique_ptr<IInputIndicatorOverlay> inputIndicatorOverlay_{};
    InputIndicatorWasmDispatchFeature inputIndicatorWasmDispatch_{};
    mutable std::mutex inputIndicatorWasmRouteStatusMutex_{};
    InputIndicatorWasmRouteStatus inputIndicatorWasmRouteStatus_{};
    InputAutomationEngine inputAutomationEngine_{};
    bool runtimeDiagnosticsEnabled_ = false;
    mutable ShortcutCaptureSession shortcutCaptureSession_{};
    static constexpr size_t kWasmEffectsHostCount = 5;
    std::array<std::unique_ptr<wasm::WasmEffectHost>, kWasmEffectsHostCount> wasmEffectHosts_{};
    std::unique_ptr<wasm::WasmEffectHost> wasmIndicatorHost_{};
    bool vmEffectsSuppressed_ = false;

#ifdef _DEBUG
    uint32_t debugClickCount_ = 0;
#endif
};

} // namespace mousefx

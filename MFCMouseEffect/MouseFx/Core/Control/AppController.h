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
#include "MouseFx/Core/Control/IPetVisualHost.h"
#include "MouseFx/Core/Control/MouseCompanionPluginHostPhase0.h"
#include "MouseFx/Core/Control/MouseCompanionPluginHostV1.h"
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

    struct MouseCompanionRuntimeStatus {
        struct ActionCoverageActionStatus {
            std::string actionName;
            bool clipPresent{false};
            int trackCount{0};
            int mappedTrackCount{0};
            float coverageRatio{0.0f};
            std::vector<std::string> missingBoneTracks;
        };

        bool configEnabled{false};
        bool runtimePresent{false};
        bool pluginHostReady{false};
        std::string pluginHostPhase{"phase0"};
        std::string activePluginId;
        std::string activePluginVersion;
        std::string engineApiVersion;
        std::string compatibilityStatus;
        std::string fallbackReason;
        std::string lastPluginEvent;
        uint64_t lastPluginEventTickMs{0};
        uint64_t pluginEventCount{0};
        bool visualHostActive{false};
        bool visualModelLoaded{false};
        bool modelLoaded{false};
        bool actionLibraryLoaded{false};
        bool effectProfileLoaded{false};
        bool appearanceProfileLoaded{false};
        bool poseFrameAvailable{false};
        bool poseBindingConfigured{false};
        int skeletonBoneCount{0};
        std::string preferredRendererBackendSource;
        std::string preferredRendererBackend;
        std::string selectedRendererBackend;
        std::string rendererBackendSelectionReason;
        std::string rendererBackendFailureReason;
        std::vector<std::string> availableRendererBackends;
        std::vector<std::string> unavailableRendererBackends;
        std::vector<PetVisualHostRendererBackendCatalogEntry> rendererBackendCatalog;
        std::vector<std::string> realRendererUnmetRequirements;
        std::string rendererRuntimeBackend;
        bool rendererRuntimeReady{false};
        bool rendererRuntimeFrameRendered{false};
        uint64_t rendererRuntimeFrameCount{0};
        uint64_t rendererRuntimeLastRenderTickMs{0};
        std::string rendererRuntimeActionName{"idle"};
        std::string rendererRuntimeReactiveActionName{"idle"};
        float rendererRuntimeActionIntensity{0.0f};
        float rendererRuntimeReactiveActionIntensity{0.0f};
        bool rendererRuntimeModelReady{false};
        bool rendererRuntimeActionLibraryReady{false};
        bool rendererRuntimeAppearanceProfileReady{false};
        bool rendererRuntimePoseFrameAvailable{false};
        bool rendererRuntimePoseBindingConfigured{false};
        std::string rendererRuntimeSceneRuntimeAdapterMode{"runtime_only"};
        uint32_t rendererRuntimeSceneRuntimePoseSampleCount{0};
        uint32_t rendererRuntimeSceneRuntimeBoundPoseSampleCount{0};
        std::string rendererRuntimeSceneRuntimeModelSceneAdapterState{"preview_only"};
        float rendererRuntimeSceneRuntimeModelSceneSeamReadiness{0.0f};
        std::string rendererRuntimeSceneRuntimeModelSceneAdapterBrief{
            "preview_only/unknown/runtime_only"};
        float rendererRuntimeSceneRuntimeModelNodeAdapterInfluence{0.0f};
        std::string rendererRuntimeSceneRuntimeModelNodeAdapterBrief{
            "preview_only/0.00"};
        std::string rendererRuntimeSceneRuntimeModelNodeChannelBrief{
            "body:0.00|face:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
        std::string rendererRuntimeSceneRuntimeModelNodeGraphState{"preview_only"};
        uint32_t rendererRuntimeSceneRuntimeModelNodeGraphNodeCount{0};
        uint32_t rendererRuntimeSceneRuntimeModelNodeGraphBoundNodeCount{0};
        std::string rendererRuntimeSceneRuntimeModelNodeGraphBrief{"preview_only/0/0"};
        std::string rendererRuntimeSceneRuntimeModelNodeBindingState{"preview_only"};
        uint32_t rendererRuntimeSceneRuntimeModelNodeBindingEntryCount{0};
        uint32_t rendererRuntimeSceneRuntimeModelNodeBindingBoundEntryCount{0};
        std::string rendererRuntimeSceneRuntimeModelNodeBindingBrief{"preview_only/0/0"};
        std::string rendererRuntimeSceneRuntimeModelNodeBindingWeightBrief{
            "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
        std::string rendererRuntimeSceneRuntimeModelNodeSlotState{"preview_only"};
        uint32_t rendererRuntimeSceneRuntimeModelNodeSlotCount{0};
        uint32_t rendererRuntimeSceneRuntimeModelNodeReadySlotCount{0};
        std::string rendererRuntimeSceneRuntimeModelNodeSlotBrief{"preview_only/0/0"};
        std::string rendererRuntimeSceneRuntimeModelNodeSlotNameBrief{
            "body:body_root|head:head_anchor|appendage:appendage_anchor|overlay:overlay_anchor|grounding:grounding_anchor"};
        std::string rendererRuntimeSceneRuntimeModelNodeRegistryState{"preview_only"};
        uint32_t rendererRuntimeSceneRuntimeModelNodeRegistryEntryCount{0};
        uint32_t rendererRuntimeSceneRuntimeModelNodeRegistryResolvedEntryCount{0};
        std::string rendererRuntimeSceneRuntimeModelNodeRegistryBrief{"preview_only/0/0"};
        std::string rendererRuntimeSceneRuntimeModelNodeRegistryAssetNodeBrief{
            "body:asset.body.root|head:asset.head.anchor|appendage:asset.appendage.anchor|overlay:asset.overlay.anchor|grounding:asset.grounding.anchor"};
        std::string rendererRuntimeSceneRuntimeModelNodeRegistryWeightBrief{
            "body:0.00|head:0.00|appendage:0.00|overlay:0.00|grounding:0.00"};
        float rendererRuntimeSceneRuntimePoseAdapterInfluence{0.0f};
        float rendererRuntimeSceneRuntimePoseReadabilityBias{0.0f};
        std::string rendererRuntimeSceneRuntimePoseAdapterBrief{"runtime_only/0.00/0.00"};
        int rendererRuntimeFacingDirection{1};
        int rendererRuntimeSurfaceWidth{0};
        int rendererRuntimeSurfaceHeight{0};
        std::string rendererRuntimeModelSourceFormat{"unknown"};
        std::string rendererRuntimeAppearanceSkinVariantId{"default"};
        std::vector<std::string> rendererRuntimeAppearanceAccessoryIds;
        std::string rendererRuntimeAppearanceAccessoryFamily{"none"};
        std::string rendererRuntimeAppearanceComboPreset{"none"};
        std::string rendererRuntimeAppearanceRequestedPresetId;
        std::string rendererRuntimeAppearanceResolvedPresetId;
        std::string rendererRuntimeAppearancePluginId;
        std::string rendererRuntimeAppearancePluginKind;
        std::string rendererRuntimeAppearancePluginSource;
        std::string rendererRuntimeAppearancePluginSelectionReason;
        std::string rendererRuntimeAppearancePluginFailureReason;
        std::string rendererRuntimeAppearancePluginManifestPath;
        std::string rendererRuntimeAppearancePluginRuntimeBackend;
        std::string rendererRuntimeAppearancePluginMetadataPath;
        uint32_t rendererRuntimeAppearancePluginMetadataSchemaVersion{0};
        std::string rendererRuntimeAppearancePluginAppearanceSemanticsMode{
            "legacy_manifest_compat"};
        std::string rendererRuntimeAppearancePluginSampleTier;
        std::string rendererRuntimeAppearancePluginContractBrief{
            "legacy_manifest_compat/-/-"};
        std::string rendererRuntimeDefaultLaneCandidate{"builtin"};
        std::string rendererRuntimeDefaultLaneSource{"runtime_builtin_default"};
        std::string rendererRuntimeDefaultLaneRolloutStatus{"stay_on_builtin"};
        std::string rendererRuntimeDefaultLaneStyleIntent{"style_candidate:none"};
        std::string rendererRuntimeDefaultLaneCandidateTier{"builtin_shipped_default"};
        std::string configuredModelPath;
        std::string configuredActionLibraryPath;
        std::string configuredEffectProfilePath;
        std::string configuredAppearanceProfilePath;
        std::string configuredRendererBackendPreferenceSource;
        std::string configuredRendererBackendPreferenceName;
        std::string visualModelPath;
        std::string loadedModelPath;
        std::string loadedModelSourceFormat{"unknown"};
        std::string loadedActionLibraryPath;
        std::string loadedEffectProfilePath;
        std::string loadedAppearanceProfilePath;
        bool modelConvertedToCanonical{false};
        std::vector<std::string> modelImportDiagnostics;
        std::string visualModelLoadError;
        std::string modelLoadError;
        std::string actionLibraryLoadError;
        std::string effectProfileLoadError;
        std::string appearanceProfileLoadError;
        int lastActionCode{0};
        float lastActionIntensity{0.0f};
        uint64_t lastActionTickMs{0};
        std::string lastActionName{"idle"};
        int clickStreak{0};
        float clickStreakTintAmount{0.0f};
        int clickStreakBreakMs{650};
        float clickStreakDecayPerSecond{0.36f};
        bool actionCoverageReady{false};
        int actionCoverageExpectedActionCount{0};
        int actionCoverageCoveredActionCount{0};
        int actionCoverageMissingActionCount{0};
        int actionCoverageSkeletonBoneCount{0};
        int actionCoverageTotalTrackCount{0};
        int actionCoverageMappedTrackCount{0};
        float actionCoverageOverallRatio{0.0f};
        std::string actionCoverageError;
        std::vector<std::string> actionCoverageMissingActions;
        std::vector<std::string> actionCoverageMissingBoneNames;
        std::vector<ActionCoverageActionStatus> actionCoverageActions;
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
    // Set launch-at-startup preference (persisted).
    void SetLaunchAtStartup(bool enabled);
    
    // Set custom text content for Text Effect
    void SetTextEffectContent(const std::vector<std::wstring>& texts);
    // Set text click font size in point units.
    void SetTextEffectFontSize(float sizePt);
    void SetMouseCompanionConfig(const MouseCompanionConfig& cfg);
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
    MouseCompanionRuntimeStatus ReadMouseCompanionRuntimeStatus() const;
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
    void DispatchPetMove(const ScreenPoint& pt);
    void DispatchPetScroll(const ScreenPoint& pt, int delta);
    void DispatchPetClick(const ClickEvent& ev);
    void DispatchPetButtonDown(const ScreenPoint& pt, int button);
    void DispatchPetButtonUp(const ScreenPoint& pt, int button);
    void DispatchPetHoverStart(const ScreenPoint& pt);
    void DispatchPetHoverEnd(const ScreenPoint& pt);
    void DispatchPetHoldStart(const ScreenPoint& pt, int button, uint32_t holdMs);
    void DispatchPetHoldUpdate(const ScreenPoint& pt, uint32_t holdMs);
    void DispatchPetHoldEnd(const ScreenPoint& pt);
    void TickPetVisualFrame();
    void KillDispatchTimer(uintptr_t timerId);
    std::string CurrentForegroundProcessBaseName() const;
    std::string ProcessBaseNameForScreenPoint(const ScreenPoint& pt) const;
    bool IsEffectsBlockedByAppBlacklist();
    bool IsEffectsBlockedByAppBlacklistAtPoint(const ScreenPoint& pt) const;
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
    uint32_t ActiveHoverThresholdMs() const;
    uint32_t ActiveHoldDelayMs() const;
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
    void SyncLaunchAtStartupRegistration();
    void SyncLaunchAtStartupManifest();


    void NotifyGpuFallbackIfNeeded(const std::string& reason);
    void WriteGpuRouteStatusSnapshot(EffectCategory category, const std::string& requestedType, const std::string& effectiveType, const std::string& reason) const;
    bool IsProcessBlockedByEffectsBlacklist(const std::string& processBaseName) const;
    void NotifyInputCaptureStatusChanged();
    void RefreshInputCaptureRuntimeState();
    void TryLoadDefaultPetModel();
    void TryLoadDefaultPetActionLibrary();
    void TryLoadDefaultPetEffectProfile();
    void TryLoadDefaultPetAppearanceProfile();
    void RecomputePetActionCoverageStatus();
    void EnsurePetVisualHost();
    void ShutdownPetVisualHost();
    bool TryLoadPetModelIntoVisualHost(const std::string& modelPath);
    bool TryLoadPetActionLibraryIntoVisualHost(const std::string& actionLibraryPath);
    bool TryLoadPetAppearanceProfileIntoVisualHost(const std::string& appearanceProfilePath);
    void TryApplyPetModelToVisualHost();
    void TryApplyPetActionLibraryToVisualHost();
    void ApplyPetVisualFollowProfile();
    void TryApplyPetAppearanceToVisualHost();
    bool EnsurePetVisualPoseBinding();
    void SyncPetVisualHostDiagnostics(const PetVisualHostDiagnostics& diagnostics);
    void ClearPetVisualHostDiagnostics();
    void UpdatePetVisualState(const ScreenPoint& pt, int actionCode, float actionIntensity, float headTintAmount);
    ScreenPoint ResolvePetRuntimeCursorPoint(const ScreenPoint& rawPt, double dtSeconds, int smoothingPercent);
    void ResetPetDispatchRuntimeState();
    void UpdatePetPrimaryPressTravel(const ScreenPoint& pt);
    bool EvaluatePetPrimaryClickEligibility(uint64_t nowTickMs) const;
    void SyncPetClickStreakRuntimeStatus(const MouseCompanionConfig& activeConfig);
    void UpdatePetClickStreakDecay(uint64_t nowTickMs, const MouseCompanionConfig& activeConfig);
    void RegisterPetClickStreakClick(uint64_t nowTickMs, const MouseCompanionConfig& activeConfig);
    uint32_t ResolvePetVisualHoldEnterMs(const MouseCompanionConfig& activeConfig) const;
    double ResolvePetVisualHoldStableSpeedThresholdPxPerSec(const MouseCompanionConfig& activeConfig) const;
    bool IsPetVisualHoldSuppressedByScroll(uint64_t nowTickMs, const MouseCompanionConfig& activeConfig) const;
    bool ResolvePetVisualHoldState(
        const MouseCompanionConfig& activeConfig,
        uint64_t nowTickMs,
        float* outIntensity) const;
    void UpdatePetPointerMotion(const ScreenPoint& pt, uint64_t nowTickMs);
    void DecayPetPointerMotion(uint64_t nowTickMs, const MouseCompanionConfig& activeConfig);
    void ResolvePetContinuousAction(const MouseCompanionConfig& activeConfig, int* outActionCode, float* outActionIntensity) const;
    MouseCompanionPetRuntimeConfig BuildMouseCompanionPluginRuntimeConfig(const MouseCompanionConfig& cfg) const;
    void SyncMouseCompanionPluginPhase0Status();
    void ResetMouseCompanionPluginHosts(const MouseCompanionConfig& cfg, uint64_t nowTickMs);
    void OnMouseCompanionPluginConfigChanged(const MouseCompanionConfig& cfg, uint64_t nowTickMs);
    void RecordMouseCompanionPluginPhase0Input(const char* eventName);
    void RecordMouseCompanionPluginInput(const char* eventName, const MouseCompanionPetInputEvent& event);
    void CommitMouseCompanionPluginResolvedFrame(const MouseCompanionPetPoseFrame& frame);
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
    static constexpr uint32_t kHoverThresholdMs = 1500;
    static constexpr uint32_t kHoverThresholdTestMs = 320;
    static constexpr uintptr_t kInputCaptureHealthTimerId = 6;
    static constexpr uintptr_t kWasmFrameTimerId = 10;

    // Hold delay logic
    static constexpr uintptr_t kHoldTimerId = 5;
    static constexpr uint32_t kHoldDelayMs = 260;
    static constexpr uint32_t kHoldDelayTestMs = 120;
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
    mutable std::mutex mouseCompanionRuntimeStatusMutex_{};
    MouseCompanionRuntimeStatus mouseCompanionRuntimeStatus_{};
    InputAutomationEngine inputAutomationEngine_{};
    std::unique_ptr<IPetVisualHost> petVisualHost_{};
    std::string loadedPetModelPath_{};
    std::string loadedPetActionLibraryPath_{};
    std::string loadedPetEffectProfilePath_{};
    std::string loadedPetAppearanceProfilePath_{};
    bool petVisualPoseBindingConfigured_{false};
    bool petVisualPoseBindingAttempted_{false};
    bool petDragging_ = false;
    uint64_t petLastTickMs_ = 0;
    static constexpr uint32_t kPetClickMaxPressMs = 220;
    static constexpr double kPetClickMaxTravelPx = 10.0;
    static constexpr double kPetDragStartTravelPx = 1.0;
    static constexpr uint32_t kPetClickSuppressAfterScrollMs = 140;
    static constexpr uint32_t kPetScrollImpulseDurationMs = 720;
    static constexpr uint32_t kPetScrollImpulseDurationTestMs = 560;
    static constexpr uint32_t kPetVisualHoldEnterMs = 130;
    static constexpr uint32_t kPetVisualHoldEnterTestMs = 90;
    static constexpr double kPetVisualHoldStableSpeedThresholdPxPerSec = 24.0;
    static constexpr double kPetVisualHoldStableSpeedThresholdTestPxPerSec = 30.0;
    static constexpr uint32_t kPetVisualHoldSuppressAfterScrollMs = 720;
    bool petHasSmoothedCursor_ = false;
    double petSmoothedCursorX_ = 0.0;
    double petSmoothedCursorY_ = 0.0;
    ScreenPoint petLastDispatchPoint_{};
    bool petHasLastDispatchPoint_ = false;
    uint64_t petReleaseHoldUntilTickMs_ = 0;
    struct PetPrimaryPressState {
        bool active = false;
        ScreenPoint downPoint{};
        uint64_t downTickMs = 0;
        double maxTravelPx = 0.0;
        bool holdTriggered = false;
        bool releaseReady = false;
        uint64_t releaseTickMs = 0;
        uint32_t releasePressMs = 0;
        double releaseMaxTravelPx = 0.0;
    } petPrimaryPress_{};
    uint64_t petLastScrollTickMs_ = 0;
    struct PetClickStreakState {
        int streak = 0;
        uint64_t lastClickTickMs = 0;
        uint64_t lastUpdateTickMs = 0;
        float tintAmount = 0.0f;
    } petClickStreak_{};
    struct PetPointerMotionState {
        bool hasSample = false;
        ScreenPoint lastSamplePoint{};
        uint64_t lastSampleTickMs = 0;
        uint64_t lastEvalTickMs = 0;
        double moveSpeedPxPerSec = 0.0;
    } petPointerMotion_{};
    struct PetVisualPoseRuntimeState {
        float holdPulse = 0.0f;
        float followWalkPhase = 0.0f;
        float scrollFlapProgress = 1.0f;
        float scrollFlapDurationSec = 0.18f;
        float scrollFlapAmplitude = 0.35f;
        float queuedScrollFlapDurationSec = 0.18f;
        float queuedScrollFlapAmplitude = 0.35f;
        uint32_t pendingScrollFlapCount = 0;
        uint64_t lastScrollEventTickMs = 0;
        uint64_t lastTickMs = 0;
    } petVisualPoseRuntime_{};
    bool runtimeDiagnosticsEnabled_ = false;
    mutable ShortcutCaptureSession shortcutCaptureSession_{};
    MouseCompanionPluginHostPhase0 petPluginHostPhase0_{};
    MouseCompanionPluginHostV1 petPluginHostV1_{};
    static constexpr size_t kWasmEffectsHostCount = 5;
    std::array<std::unique_ptr<wasm::WasmEffectHost>, kWasmEffectsHostCount> wasmEffectHosts_{};
    std::unique_ptr<wasm::WasmEffectHost> wasmIndicatorHost_{};
    bool vmEffectsSuppressed_ = false;

#ifdef _DEBUG
    uint32_t debugClickCount_ = 0;
#endif
};

} // namespace mousefx

#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "WasmBinaryCodec.h"
#include "WasmCommandBufferParser.h"
#include "WasmExecutionBudgetGuard.h"
#include "WasmPluginManifest.h"
#include "WasmPluginPaths.h"
#include "WasmPluginAbi.h"
#include "WasmRuntime.h"
#include "WasmRuntimeFactory.h"

namespace mousefx::wasm {

enum class EventKind : uint8_t {
    Click = 1,
    Move = 2,
    Scroll = 3,
    HoldStart = 4,
    HoldUpdate = 5,
    HoldEnd = 6,
    HoverStart = 7,
    HoverEnd = 8,
    IndicatorClick = 9,
    IndicatorScroll = 10,
    IndicatorKey = 11,
};

struct EventInvokeInput final {
    EventKind kind = EventKind::Click;
    int32_t x = 0;
    int32_t y = 0;
    int32_t delta = 0;
    uint32_t holdMs = 0;
    uint8_t button = 0;
    uint8_t flags = 0;
    uint64_t eventTickMs = 0;
    IndicatorEventTailV1 indicatorMetrics{};
    bool hasIndicatorMetrics = false;
    IndicatorEventContextTailV2 indicatorContext{};
    bool hasIndicatorContext = false;
    std::string dynamicTextLabelUtf8{};
};

struct FrameInvokeInput final {
    int32_t cursorX = 0;
    int32_t cursorY = 0;
    uint32_t frameDeltaMs = 0;
    bool pointerValid = false;
    bool holdActive = false;
    uint64_t frameTickMs = 0;
};

struct ExecutionBudget final {
    uint32_t outputBufferBytes = 16u * 1024u;
    uint32_t maxCommands = 256;
    double maxEventExecutionMs = 1.0;
};

struct HostDiagnostics final {
    bool enabled = false;
    std::string runtimeBackend{"null"};
    std::string runtimeFallbackReason{};
    bool pluginLoaded = false;
    uint32_t pluginApiVersion = 0;
    std::string activePluginId{};
    std::string activePluginName{};
    std::wstring activeManifestPath{};
    std::wstring activeWasmPath{};
    uint64_t lastCallDurationMicros = 0;
    uint32_t lastOutputBytes = 0;
    uint32_t lastCommandCount = 0;
    bool lastCallExceededBudget = false;
    bool lastCallRejectedByBudget = false;
    bool lastOutputTruncatedByBudget = false;
    bool lastCommandTruncatedByBudget = false;
    std::string lastBudgetReason{};
    CommandParseError lastParseError = CommandParseError::None;
    bool lastRenderedByWasm = false;
    uint32_t lastExecutedTextCommands = 0;
    uint32_t lastExecutedImageCommands = 0;
    uint32_t lastExecutedPulseCommands = 0;
    uint32_t lastExecutedPolylineCommands = 0;
    uint32_t lastExecutedPathStrokeCommands = 0;
    uint32_t lastExecutedPathFillCommands = 0;
    uint32_t lastExecutedGlowBatchCommands = 0;
    uint32_t lastExecutedSpriteBatchCommands = 0;
    uint32_t lastExecutedGlowEmitterCommands = 0;
    uint32_t lastExecutedGlowEmitterRemoveCommands = 0;
    uint32_t lastExecutedSpriteEmitterCommands = 0;
    uint32_t lastExecutedSpriteEmitterRemoveCommands = 0;
    uint32_t lastExecutedParticleEmitterCommands = 0;
    uint32_t lastExecutedParticleEmitterRemoveCommands = 0;
    uint32_t lastExecutedRibbonTrailCommands = 0;
    uint32_t lastExecutedRibbonTrailRemoveCommands = 0;
    uint32_t lastExecutedQuadFieldCommands = 0;
    uint32_t lastExecutedQuadFieldRemoveCommands = 0;
    uint32_t lastExecutedGroupRemoveCommands = 0;
    uint32_t lastExecutedGroupPresentationCommands = 0;
    uint32_t lastExecutedGroupClipRectCommands = 0;
    uint32_t lastExecutedGroupLayerCommands = 0;
    uint32_t lastExecutedGroupTransformCommands = 0;
    uint32_t lastExecutedGroupLocalOriginCommands = 0;
    uint32_t lastExecutedGroupMaterialCommands = 0;
    uint32_t lastExecutedGroupPassCommands = 0;
    uint32_t lastThrottledRenderCommands = 0;
    uint32_t lastThrottledByCapacityRenderCommands = 0;
    uint32_t lastThrottledByIntervalRenderCommands = 0;
    uint32_t lastDroppedRenderCommands = 0;
    uint64_t lifetimeInvokeCalls = 0;
    uint64_t lifetimeInvokeSuccessCalls = 0;
    uint64_t lifetimeInvokeFailedCalls = 0;
    uint64_t lifetimeInvokeDurationMicros = 0;
    uint64_t lifetimeInvokeExceededBudgetCalls = 0;
    uint64_t lifetimeInvokeRejectedByBudgetCalls = 0;
    uint64_t lifetimeRenderDispatches = 0;
    uint64_t lifetimeRenderedByWasmDispatches = 0;
    uint64_t lifetimeExecutedTextCommands = 0;
    uint64_t lifetimeExecutedImageCommands = 0;
    uint64_t lifetimeExecutedPulseCommands = 0;
    uint64_t lifetimeExecutedPolylineCommands = 0;
    uint64_t lifetimeExecutedPathStrokeCommands = 0;
    uint64_t lifetimeExecutedPathFillCommands = 0;
    uint64_t lifetimeExecutedGlowBatchCommands = 0;
    uint64_t lifetimeExecutedSpriteBatchCommands = 0;
    uint64_t lifetimeExecutedGlowEmitterCommands = 0;
    uint64_t lifetimeExecutedGlowEmitterRemoveCommands = 0;
    uint64_t lifetimeExecutedSpriteEmitterCommands = 0;
    uint64_t lifetimeExecutedSpriteEmitterRemoveCommands = 0;
    uint64_t lifetimeExecutedParticleEmitterCommands = 0;
    uint64_t lifetimeExecutedParticleEmitterRemoveCommands = 0;
    uint64_t lifetimeExecutedRibbonTrailCommands = 0;
    uint64_t lifetimeExecutedRibbonTrailRemoveCommands = 0;
    uint64_t lifetimeExecutedQuadFieldCommands = 0;
    uint64_t lifetimeExecutedQuadFieldRemoveCommands = 0;
    uint64_t lifetimeExecutedGroupRemoveCommands = 0;
    uint64_t lifetimeExecutedGroupPresentationCommands = 0;
    uint64_t lifetimeExecutedGroupClipRectCommands = 0;
    uint64_t lifetimeExecutedGroupLayerCommands = 0;
    uint64_t lifetimeExecutedGroupTransformCommands = 0;
    uint64_t lifetimeExecutedGroupLocalOriginCommands = 0;
    uint64_t lifetimeExecutedGroupMaterialCommands = 0;
    uint64_t lifetimeExecutedGroupPassCommands = 0;
    uint64_t lifetimeThrottledRenderCommands = 0;
    uint64_t lifetimeThrottledByCapacityRenderCommands = 0;
    uint64_t lifetimeThrottledByIntervalRenderCommands = 0;
    uint64_t lifetimeDroppedRenderCommands = 0;
    std::string lastRenderError{};
    std::string lastLoadFailureStage{};
    std::string lastLoadFailureCode{};
    std::string lastError{};
};

class WasmEffectHost final {
public:
    explicit WasmEffectHost(std::unique_ptr<IWasmRuntime> runtime = nullptr);

    bool LoadPlugin(const std::wstring& modulePath);
    bool LoadPluginFromManifest(const std::wstring& manifestPath);
    bool ReloadPlugin();
    void UnloadPlugin();
    bool IsPluginLoaded() const;

    void SetEnabled(bool enabled);
    bool Enabled() const;

    void SetExecutionBudget(const ExecutionBudget& budget);
    const ExecutionBudget& GetExecutionBudget() const;
    bool SupportsInputEvent(EventKind kind) const;
    bool SupportsFrameTick() const;
    const HostDiagnostics& Diagnostics() const;
    void RecordRenderExecution(
        bool renderedByWasm,
        uint32_t executedTextCommands,
        uint32_t executedImageCommands,
        uint32_t executedPulseCommands,
        uint32_t executedPolylineCommands,
        uint32_t executedPathStrokeCommands,
        uint32_t executedPathFillCommands,
        uint32_t executedGlowBatchCommands,
        uint32_t executedSpriteBatchCommands,
        uint32_t executedGlowEmitterCommands,
        uint32_t executedGlowEmitterRemoveCommands,
        uint32_t executedSpriteEmitterCommands,
        uint32_t executedSpriteEmitterRemoveCommands,
        uint32_t executedParticleEmitterCommands,
        uint32_t executedParticleEmitterRemoveCommands,
        uint32_t executedRibbonTrailCommands,
        uint32_t executedRibbonTrailRemoveCommands,
        uint32_t executedQuadFieldCommands,
        uint32_t executedQuadFieldRemoveCommands,
        uint32_t executedGroupRemoveCommands,
        uint32_t executedGroupPresentationCommands,
        uint32_t executedGroupClipRectCommands,
        uint32_t executedGroupLayerCommands,
        uint32_t executedGroupTransformCommands,
        uint32_t executedGroupLocalOriginCommands,
        uint32_t executedGroupMaterialCommands,
        uint32_t executedGroupPassCommands,
        uint32_t throttledRenderCommands,
        uint32_t throttledByCapacityRenderCommands,
        uint32_t throttledByIntervalRenderCommands,
        uint32_t droppedRenderCommands,
        const std::string& renderError);

    void ResetPluginState();
    bool InvokeInput(const EventInvokeInput& input, std::vector<uint8_t>* outCommandBuffer);
    bool InvokeFrame(const FrameInvokeInput& input, std::vector<uint8_t>* outCommandBuffer);

private:
    bool InvokePayload(
        const uint8_t* payloadPtr,
        uint32_t payloadBytes,
        bool isFrameInvoke,
        std::vector<uint8_t>* outCommandBuffer);
    EventInputV2 BuildEventInputV2(const EventInvokeInput& input) const;
    FrameInputV2 BuildFrameInputV2(const FrameInvokeInput& input) const;
    void SetLoadFailure(const std::string& stage, const std::string& code, const std::string& message);
    void ClearLoadFailure();
    void SetError(const std::string& error);
    void ClearError();
    void ClearActivePluginMetadata();

    std::unique_ptr<IWasmRuntime> runtime_{};
    bool enabled_ = false;
    ExecutionBudget budget_{};
    HostDiagnostics diagnostics_{};
    PluginManifest activeManifest_{};
};

} // namespace mousefx::wasm

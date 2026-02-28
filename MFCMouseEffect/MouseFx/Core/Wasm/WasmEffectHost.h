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
    const HostDiagnostics& Diagnostics() const;
    void RecordRenderExecution(
        bool renderedByWasm,
        uint32_t executedTextCommands,
        uint32_t executedImageCommands,
        uint32_t throttledRenderCommands,
        uint32_t throttledByCapacityRenderCommands,
        uint32_t throttledByIntervalRenderCommands,
        uint32_t droppedRenderCommands,
        const std::string& renderError);

    void ResetPluginState();
    bool InvokeEvent(const EventInvokeInput& input, std::vector<uint8_t>* outCommandBuffer);

private:
    EventInputV1 BuildEventInputV1(const EventInvokeInput& input) const;
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

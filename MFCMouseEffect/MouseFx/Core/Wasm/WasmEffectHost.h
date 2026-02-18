#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "WasmBinaryCodec.h"
#include "WasmCommandBufferParser.h"
#include "WasmPluginAbi.h"
#include "WasmRuntime.h"
#include "WasmRuntimeFactory.h"

namespace mousefx::wasm {

struct ClickInvokeInput final {
    int32_t x = 0;
    int32_t y = 0;
    uint8_t button = 0;
    uint64_t eventTickMs = 0;
};

struct ExecutionBudget final {
    uint32_t outputBufferBytes = 16u * 1024u;
    uint32_t maxCommands = 256;
    double maxEventExecutionMs = 1.0;
};

struct HostDiagnostics final {
    bool enabled = false;
    bool pluginLoaded = false;
    uint32_t pluginApiVersion = 0;
    uint64_t lastCallDurationMicros = 0;
    uint32_t lastOutputBytes = 0;
    uint32_t lastCommandCount = 0;
    bool lastCallExceededBudget = false;
    CommandParseError lastParseError = CommandParseError::None;
    std::string lastError{};
};

class WasmEffectHost final {
public:
    explicit WasmEffectHost(std::unique_ptr<IWasmRuntime> runtime = CreateRuntime(RuntimeBackend::Null));

    bool LoadPlugin(const std::wstring& modulePath);
    void UnloadPlugin();
    bool IsPluginLoaded() const;

    void SetEnabled(bool enabled);
    bool Enabled() const;

    void SetExecutionBudget(const ExecutionBudget& budget);
    const ExecutionBudget& GetExecutionBudget() const;
    const HostDiagnostics& Diagnostics() const;

    void ResetPluginState();
    bool InvokeClick(const ClickInvokeInput& input, std::vector<uint8_t>* outCommandBuffer);

private:
    ClickInputV1 BuildClickInputV1(const ClickInvokeInput& input) const;
    void SetError(const std::string& error);
    void ClearError();

    std::unique_ptr<IWasmRuntime> runtime_{};
    bool enabled_ = false;
    ExecutionBudget budget_{};
    HostDiagnostics diagnostics_{};
};

} // namespace mousefx::wasm

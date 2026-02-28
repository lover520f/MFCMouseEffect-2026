#include "pch.h"

#include "WasmEffectHost.h"

#include <algorithm>
#include <chrono>

namespace mousefx::wasm {
namespace {

void ResetInvokeDiagnostics(HostDiagnostics* diagnostics) {
    if (!diagnostics) {
        return;
    }
    diagnostics->lastCallDurationMicros = 0;
    diagnostics->lastOutputBytes = 0;
    diagnostics->lastCommandCount = 0;
    diagnostics->lastCallExceededBudget = false;
    diagnostics->lastCallRejectedByBudget = false;
    diagnostics->lastOutputTruncatedByBudget = false;
    diagnostics->lastCommandTruncatedByBudget = false;
    diagnostics->lastBudgetReason.clear();
    diagnostics->lastParseError = CommandParseError::None;
    diagnostics->lastRenderedByWasm = false;
    diagnostics->lastExecutedTextCommands = 0;
    diagnostics->lastExecutedImageCommands = 0;
    diagnostics->lastThrottledRenderCommands = 0;
    diagnostics->lastThrottledByCapacityRenderCommands = 0;
    diagnostics->lastThrottledByIntervalRenderCommands = 0;
    diagnostics->lastDroppedRenderCommands = 0;
    diagnostics->lastRenderError.clear();
}

} // namespace

bool WasmEffectHost::InvokeEvent(const EventInvokeInput& input, std::vector<uint8_t>* outCommandBuffer) {
    diagnostics_.lifetimeInvokeCalls += 1;
    auto markInvokeFailure = [this]() {
        diagnostics_.lifetimeInvokeFailedCalls += 1;
    };
    if (!outCommandBuffer) {
        SetError("Output command buffer pointer is null.");
        markInvokeFailure();
        return false;
    }
    outCommandBuffer->clear();
    ResetInvokeDiagnostics(&diagnostics_);

    if (!enabled_) {
        markInvokeFailure();
        return false;
    }
    if (!diagnostics_.pluginLoaded || !runtime_) {
        SetError("WASM plugin is not loaded.");
        markInvokeFailure();
        return false;
    }
    if (budget_.outputBufferBytes == 0) {
        SetError("WASM output budget is zero.");
        markInvokeFailure();
        return false;
    }

    std::vector<uint8_t> output(budget_.outputBufferBytes, 0);
    uint32_t writtenBytes = 0;
    std::string error;
    bool ok = false;

    const auto start = std::chrono::steady_clock::now();
    const EventInputV1 eventInput = BuildEventInputV1(input);
    const auto payload = SerializeEventInputV1(eventInput);
    ok = runtime_->CallOnEvent(
        payload.data(),
        static_cast<uint32_t>(payload.size()),
        output.data(),
        static_cast<uint32_t>(output.size()),
        &writtenBytes,
        &error);
    const auto end = std::chrono::steady_clock::now();
    diagnostics_.lastCallDurationMicros = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());
    diagnostics_.lifetimeInvokeDurationMicros += diagnostics_.lastCallDurationMicros;

    const double elapsedMs = static_cast<double>(diagnostics_.lastCallDurationMicros) / 1000.0;
    diagnostics_.lastCallExceededBudget = elapsedMs > budget_.maxEventExecutionMs;
    if (diagnostics_.lastCallExceededBudget) {
        diagnostics_.lifetimeInvokeExceededBudgetCalls += 1;
    }

    if (!ok) {
        SetError(error.empty() ? "WASM plugin call failed." : error);
        markInvokeFailure();
        return false;
    }

    const uint32_t cappedBytes = std::min<uint32_t>(writtenBytes, static_cast<uint32_t>(output.size()));
    output.resize(cappedBytes);
    diagnostics_.lastOutputBytes = cappedBytes;
    const CommandParseResult parseResult =
        WasmCommandBufferParser::Parse(output.data(), output.size(), budget_.maxCommands);
    diagnostics_.lastCommandCount = static_cast<uint32_t>(parseResult.commands.size());
    diagnostics_.lastParseError = parseResult.error;
    if (parseResult.error == CommandParseError::CommandLimitExceeded) {
        output.resize(parseResult.consumedBytes);
        diagnostics_.lastOutputBytes = parseResult.consumedBytes;
    } else if (parseResult.error != CommandParseError::None) {
        output.clear();
        diagnostics_.lastOutputBytes = 0;
        SetError(std::string("WASM command buffer parse failed: ") + CommandParseErrorToString(parseResult.error));
        markInvokeFailure();
        return false;
    }

    const BudgetCheckInput budgetInput{
        budget_.outputBufferBytes,
        budget_.maxCommands,
        budget_.maxEventExecutionMs,
        writtenBytes,
        diagnostics_.lastCommandCount,
        elapsedMs,
        parseResult.error == CommandParseError::CommandLimitExceeded,
    };
    const BudgetCheckResult budgetResult = WasmExecutionBudgetGuard::Evaluate(budgetInput);
    diagnostics_.lastCallRejectedByBudget = !budgetResult.accepted;
    diagnostics_.lastOutputTruncatedByBudget = budgetResult.outputTruncated;
    diagnostics_.lastCommandTruncatedByBudget = budgetResult.commandTruncated;
    diagnostics_.lastBudgetReason = budgetResult.reason;
    if (!budgetResult.accepted) {
        diagnostics_.lifetimeInvokeRejectedByBudgetCalls += 1;
        output.clear();
        diagnostics_.lastOutputBytes = 0;
        SetError(std::string("WASM budget rejected event: ") + budgetResult.reason);
        markInvokeFailure();
        return false;
    }

    outCommandBuffer->swap(output);
    diagnostics_.lifetimeInvokeSuccessCalls += 1;
    ClearError();
    return true;
}

EventInputV1 WasmEffectHost::BuildEventInputV1(const EventInvokeInput& input) const {
    EventInputV1 payload{};
    payload.x = input.x;
    payload.y = input.y;
    payload.delta = input.delta;
    payload.holdMs = input.holdMs;
    payload.kind = static_cast<uint8_t>(input.kind);
    payload.button = input.button;
    payload.flags = input.flags;
    payload.eventTickMs = input.eventTickMs;
    return payload;
}

} // namespace mousefx::wasm

#include "pch.h"

#include "WasmEffectHost.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <utility>

namespace mousefx::wasm {

WasmEffectHost::WasmEffectHost(std::unique_ptr<IWasmRuntime> runtime)
    : runtime_(runtime ? std::move(runtime) : CreateRuntime(RuntimeBackend::Null)) {
    diagnostics_.enabled = enabled_;
}

bool WasmEffectHost::LoadPlugin(const std::wstring& modulePath) {
    if (!runtime_) {
        SetError("WASM host runtime is null.");
        diagnostics_.pluginLoaded = false;
        diagnostics_.pluginApiVersion = 0;
        return false;
    }

    std::string error;
    if (!runtime_->LoadModuleFromFile(modulePath, &error)) {
        SetError(error.empty() ? "Failed to load WASM module." : error);
        diagnostics_.pluginLoaded = false;
        diagnostics_.pluginApiVersion = 0;
        return false;
    }

    uint32_t apiVersion = 0;
    if (!runtime_->CallGetApiVersion(&apiVersion, &error)) {
        runtime_->UnloadModule();
        SetError(error.empty() ? "Failed to call mfx_plugin_get_api_version." : error);
        diagnostics_.pluginLoaded = false;
        diagnostics_.pluginApiVersion = 0;
        return false;
    }
    if (apiVersion != kPluginApiVersionV1) {
        runtime_->UnloadModule();
        SetError("Unsupported plugin api_version.");
        diagnostics_.pluginLoaded = false;
        diagnostics_.pluginApiVersion = 0;
        return false;
    }

    diagnostics_.pluginLoaded = true;
    diagnostics_.pluginApiVersion = apiVersion;
    ClearError();
    return true;
}

void WasmEffectHost::UnloadPlugin() {
    if (runtime_) {
        runtime_->UnloadModule();
    }
    diagnostics_.pluginLoaded = false;
    diagnostics_.pluginApiVersion = 0;
    diagnostics_.lastOutputBytes = 0;
    diagnostics_.lastCallDurationMicros = 0;
    diagnostics_.lastCallExceededBudget = false;
}

bool WasmEffectHost::IsPluginLoaded() const {
    return diagnostics_.pluginLoaded;
}

void WasmEffectHost::SetEnabled(bool enabled) {
    enabled_ = enabled;
    diagnostics_.enabled = enabled;
}

bool WasmEffectHost::Enabled() const {
    return enabled_;
}

void WasmEffectHost::SetExecutionBudget(const ExecutionBudget& budget) {
    budget_ = budget;
}

const ExecutionBudget& WasmEffectHost::GetExecutionBudget() const {
    return budget_;
}

const HostDiagnostics& WasmEffectHost::Diagnostics() const {
    return diagnostics_;
}

void WasmEffectHost::ResetPluginState() {
    if (runtime_ && diagnostics_.pluginLoaded) {
        runtime_->ResetPluginState();
    }
}

bool WasmEffectHost::InvokeClick(const ClickInvokeInput& input, std::vector<uint8_t>* outCommandBuffer) {
    if (!outCommandBuffer) {
        SetError("Output command buffer pointer is null.");
        return false;
    }
    outCommandBuffer->clear();

    diagnostics_.lastCallDurationMicros = 0;
    diagnostics_.lastOutputBytes = 0;
    diagnostics_.lastCallExceededBudget = false;

    if (!enabled_) {
        return false;
    }
    if (!diagnostics_.pluginLoaded || !runtime_) {
        SetError("WASM plugin is not loaded.");
        return false;
    }
    if (budget_.outputBufferBytes == 0) {
        SetError("WASM output budget is zero.");
        return false;
    }

    const auto payload = BuildClickInputPayload(input);
    std::vector<uint8_t> output(budget_.outputBufferBytes, 0);

    uint32_t writtenBytes = 0;
    std::string error;
    const auto start = std::chrono::steady_clock::now();
    const bool ok = runtime_->CallOnClick(
        payload.data(),
        static_cast<uint32_t>(payload.size()),
        output.data(),
        static_cast<uint32_t>(output.size()),
        &writtenBytes,
        &error);
    const auto end = std::chrono::steady_clock::now();
    diagnostics_.lastCallDurationMicros = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::microseconds>(end - start).count());

    const double elapsedMs = static_cast<double>(diagnostics_.lastCallDurationMicros) / 1000.0;
    diagnostics_.lastCallExceededBudget = elapsedMs > budget_.maxEventExecutionMs;

    if (!ok) {
        SetError(error.empty() ? "WASM plugin call failed." : error);
        return false;
    }

    const uint32_t cappedBytes = std::min<uint32_t>(writtenBytes, static_cast<uint32_t>(output.size()));
    output.resize(cappedBytes);
    diagnostics_.lastOutputBytes = cappedBytes;
    outCommandBuffer->swap(output);
    ClearError();
    return true;
}

std::array<uint8_t, sizeof(ClickInputV1)> WasmEffectHost::BuildClickInputPayload(const ClickInvokeInput& input) const {
    ClickInputV1 payload{};
    payload.x = input.x;
    payload.y = input.y;
    payload.button = input.button;
    payload.eventTickMs = input.eventTickMs;

    std::array<uint8_t, sizeof(ClickInputV1)> bytes{};
    std::memcpy(bytes.data(), &payload, sizeof(payload));
    return bytes;
}

void WasmEffectHost::SetError(const std::string& error) {
    diagnostics_.lastError = error;
}

void WasmEffectHost::ClearError() {
    diagnostics_.lastError.clear();
}

} // namespace mousefx::wasm


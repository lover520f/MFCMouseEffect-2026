#include "pch.h"

#include "AppController.h"

#include "MouseFx/Core/Config/EffectConfigInternal.h"
#include "MouseFx/Core/Wasm/WasmEffectHost.h"
#include "MouseFx/Utils/StringUtils.h"

namespace mousefx {
namespace {

constexpr uint32_t kInputIndicatorWasmMinOutputBufferBytes = 4096u;
constexpr uint32_t kInputIndicatorWasmMinCommands = 32u;

enum class EffectsWasmLane : size_t {
    Click = 0,
    Trail = 1,
    Scroll = 2,
    Hold = 3,
    Hover = 4,
};

wasm::ExecutionBudget BuildExecutionBudget(const WasmConfig& cfg) {
    wasm::ExecutionBudget budget{};
    budget.outputBufferBytes = cfg.outputBufferBytes;
    budget.maxCommands = cfg.maxCommands;
    budget.maxEventExecutionMs = cfg.maxEventExecutionMs;
    return budget;
}

EffectsWasmLane LaneFromIndex(size_t index) {
    switch (index) {
    case 0:
        return EffectsWasmLane::Click;
    case 1:
        return EffectsWasmLane::Trail;
    case 2:
        return EffectsWasmLane::Scroll;
    case 3:
        return EffectsWasmLane::Hold;
    case 4:
        return EffectsWasmLane::Hover;
    default:
        return EffectsWasmLane::Click;
    }
}

size_t LaneIndex(EffectsWasmLane lane) {
    return static_cast<size_t>(lane);
}

bool TryResolveEffectsLane(const std::string& channelRaw, EffectsWasmLane* outLane) {
    if (!outLane) {
        return false;
    }
    const std::string channel = ToLowerAscii(TrimAscii(channelRaw));
    if (channel.empty() || channel == "click") {
        *outLane = EffectsWasmLane::Click;
        return true;
    }
    if (channel == "trail" || channel == "move") {
        *outLane = EffectsWasmLane::Trail;
        return true;
    }
    if (channel == "scroll") {
        *outLane = EffectsWasmLane::Scroll;
        return true;
    }
    if (channel == "hold") {
        *outLane = EffectsWasmLane::Hold;
        return true;
    }
    if (channel == "hover") {
        *outLane = EffectsWasmLane::Hover;
        return true;
    }
    return false;
}

std::string* MutableEffectsLaneManifestPath(WasmConfig* cfg, EffectsWasmLane lane) {
    if (!cfg) {
        return nullptr;
    }
    switch (lane) {
    case EffectsWasmLane::Click:
        return &cfg->manifestPathClick;
    case EffectsWasmLane::Trail:
        return &cfg->manifestPathTrail;
    case EffectsWasmLane::Scroll:
        return &cfg->manifestPathScroll;
    case EffectsWasmLane::Hold:
        return &cfg->manifestPathHold;
    case EffectsWasmLane::Hover:
        return &cfg->manifestPathHover;
    default:
        return nullptr;
    }
}

const std::string* EffectsLaneManifestPath(const WasmConfig& cfg, EffectsWasmLane lane) {
    switch (lane) {
    case EffectsWasmLane::Click:
        return &cfg.manifestPathClick;
    case EffectsWasmLane::Trail:
        return &cfg.manifestPathTrail;
    case EffectsWasmLane::Scroll:
        return &cfg.manifestPathScroll;
    case EffectsWasmLane::Hold:
        return &cfg.manifestPathHold;
    case EffectsWasmLane::Hover:
        return &cfg.manifestPathHover;
    default:
        return nullptr;
    }
}

std::string ResolveEffectsConfiguredManifestPath(const WasmConfig& cfg, EffectsWasmLane lane) {
    const std::string* lanePath = EffectsLaneManifestPath(cfg, lane);
    if (lanePath) {
        const std::string specific = TrimAscii(*lanePath);
        if (!specific.empty()) {
            return specific;
        }
    }
    return TrimAscii(cfg.manifestPath);
}

void ClearEffectsLaneManifestPaths(WasmConfig* cfg) {
    if (!cfg) {
        return;
    }
    cfg->manifestPathClick.clear();
    cfg->manifestPathTrail.clear();
    cfg->manifestPathScroll.clear();
    cfg->manifestPathHold.clear();
    cfg->manifestPathHover.clear();
}

std::string ResolveEffectsStartupManifestPath(const EffectConfig& cfg, EffectsWasmLane lane) {
    return ResolveEffectsConfiguredManifestPath(cfg.wasm, lane);
}

std::string ResolveIndicatorStartupManifestPath(const EffectConfig& cfg) {
    if (cfg.inputIndicator.renderMode != "wasm") {
        return {};
    }
    return TrimAscii(cfg.inputIndicator.wasmManifestPath);
}

bool IsIndicatorSurface(const std::string& surface) {
    return ToLowerAscii(TrimAscii(surface)) == "indicator";
}

} // namespace

wasm::WasmEffectHost* AppController::WasmHost() const {
    return WasmEffectsHostForChannel("click");
}

wasm::WasmEffectHost* AppController::WasmEffectsHostForChannel(const std::string& channel) const {
    EffectsWasmLane lane = EffectsWasmLane::Click;
    if (!TryResolveEffectsLane(channel, &lane)) {
        return nullptr;
    }
    const size_t laneIndex = LaneIndex(lane);
    if (laneIndex >= wasmEffectHosts_.size()) {
        return nullptr;
    }
    return wasmEffectHosts_[laneIndex].get();
}

wasm::WasmEffectHost* AppController::WasmHostForSurface(const std::string& surface) const {
    if (IsIndicatorSurface(surface)) {
        return wasmIndicatorHost_.get();
    }
    return WasmHost();
}

void AppController::InitializeWasmHost() {
    for (auto& host : wasmEffectHosts_) {
        if (!host) {
            host = std::make_unique<wasm::WasmEffectHost>();
        }
    }
    if (!wasmIndicatorHost_) {
        wasmIndicatorHost_ = std::make_unique<wasm::WasmEffectHost>();
    }
    ApplyWasmConfigToHost(true);
}

void AppController::ApplyWasmConfigToHost(bool tryLoadManifest) {
    bool hasEffectsHost = false;
    for (const auto& host : wasmEffectHosts_) {
        if (host) {
            hasEffectsHost = true;
            break;
        }
    }
    if (!hasEffectsHost && !wasmIndicatorHost_) {
        return;
    }
    config_.wasm = config_internal::SanitizeWasmConfig(config_.wasm);
    if (config_.inputIndicator.renderMode == "wasm") {
        if (config_.wasm.outputBufferBytes < kInputIndicatorWasmMinOutputBufferBytes) {
            config_.wasm.outputBufferBytes = kInputIndicatorWasmMinOutputBufferBytes;
        }
        if (config_.wasm.maxCommands < kInputIndicatorWasmMinCommands) {
            config_.wasm.maxCommands = kInputIndicatorWasmMinCommands;
        }
        config_.wasm = config_internal::SanitizeWasmConfig(config_.wasm);
    }

    const wasm::ExecutionBudget budget = BuildExecutionBudget(config_.wasm);
    for (size_t i = 0; i < wasmEffectHosts_.size(); ++i) {
        wasm::WasmEffectHost* host = wasmEffectHosts_[i].get();
        if (!host) {
            continue;
        }
        host->SetExecutionBudget(budget);
        host->SetEnabled(false);
        const std::string effectsManifestPath = ResolveEffectsStartupManifestPath(config_, LaneFromIndex(i));
        if (tryLoadManifest && !effectsManifestPath.empty()) {
            host->LoadPluginFromManifest(Utf8ToWString(effectsManifestPath));
        }
    }
    if (wasmIndicatorHost_) {
        wasmIndicatorHost_->SetExecutionBudget(budget);
        wasmIndicatorHost_->SetEnabled(false);
        const std::string indicatorManifestPath = ResolveIndicatorStartupManifestPath(config_);
        if (tryLoadManifest && !indicatorManifestPath.empty()) {
            wasmIndicatorHost_->LoadPluginFromManifest(Utf8ToWString(indicatorManifestPath));
        }
    }
    for (auto& host : wasmEffectHosts_) {
        if (!host) {
            continue;
        }
        host->SetEnabled(config_.wasm.enabled);
    }
    if (wasmIndicatorHost_) {
        wasmIndicatorHost_->SetEnabled(config_.wasm.enabled);
    }
}

bool AppController::EnsureInputIndicatorWasmBudgetFloor() {
    WasmConfig next = config_internal::SanitizeWasmConfig(config_.wasm);
    bool adjusted = false;
    if (next.outputBufferBytes < kInputIndicatorWasmMinOutputBufferBytes) {
        next.outputBufferBytes = kInputIndicatorWasmMinOutputBufferBytes;
        adjusted = true;
    }
    if (next.maxCommands < kInputIndicatorWasmMinCommands) {
        next.maxCommands = kInputIndicatorWasmMinCommands;
        adjusted = true;
    }
    if (!adjusted) {
        return false;
    }

    config_.wasm = config_internal::SanitizeWasmConfig(next);
    for (auto& host : wasmEffectHosts_) {
        if (!host) {
            continue;
        }
        host->SetExecutionBudget(BuildExecutionBudget(config_.wasm));
    }
    if (wasmIndicatorHost_) {
        wasmIndicatorHost_->SetExecutionBudget(BuildExecutionBudget(config_.wasm));
    }
    return true;
}

void AppController::ShutdownWasmHost() {
    for (auto& host : wasmEffectHosts_) {
        if (!host) {
            continue;
        }
        host->SetEnabled(false);
        host->UnloadPlugin();
    }
    if (wasmIndicatorHost_) {
        wasmIndicatorHost_->SetEnabled(false);
        wasmIndicatorHost_->UnloadPlugin();
    }
}

void AppController::SetWasmEnabled(bool enabled) {
    WasmConfig next = config_.wasm;
    next.enabled = enabled;
    config_.wasm = config_internal::SanitizeWasmConfig(next);
    for (auto& host : wasmEffectHosts_) {
        if (!host) {
            continue;
        }
        host->SetEnabled(config_.wasm.enabled);
    }
    if (wasmIndicatorHost_) {
        wasmIndicatorHost_->SetEnabled(config_.wasm.enabled);
    }
    PersistConfig();
}

void AppController::SetWasmFallbackToBuiltinClick(bool enabled) {
    WasmConfig next = config_.wasm;
    next.fallbackToBuiltinClick = enabled;
    config_.wasm = config_internal::SanitizeWasmConfig(next);
    PersistConfig();
}

void AppController::SetWasmManifestPath(const std::string& manifestPath) {
    WasmConfig next = config_.wasm;
    next.manifestPath = manifestPath;
    config_.wasm = config_internal::SanitizeWasmConfig(next);
    PersistConfig();
}

void AppController::SetWasmManifestPathForChannel(const std::string& channel, const std::string& manifestPath) {
    if (TrimAscii(channel).empty()) {
        SetWasmManifestPath(manifestPath);
        return;
    }
    EffectsWasmLane lane = EffectsWasmLane::Click;
    if (!TryResolveEffectsLane(channel, &lane)) {
        return;
    }

    WasmConfig next = config_.wasm;
    std::string* lanePath = MutableEffectsLaneManifestPath(&next, lane);
    if (!lanePath) {
        return;
    }
    *lanePath = manifestPath;
    config_.wasm = config_internal::SanitizeWasmConfig(next);
    PersistConfig();
}

std::string AppController::ResolveWasmManifestPathForChannel(const std::string& channel) const {
    EffectsWasmLane lane = EffectsWasmLane::Click;
    if (!TryResolveEffectsLane(channel, &lane)) {
        return TrimAscii(config_.wasm.manifestPath);
    }
    return ResolveEffectsConfiguredManifestPath(config_.wasm, lane);
}

void AppController::SetWasmCatalogRootPath(const std::string& catalogRootPath) {
    WasmConfig next = config_.wasm;
    next.catalogRootPath = catalogRootPath;
    config_.wasm = config_internal::SanitizeWasmConfig(next);
    PersistConfig();
}

void AppController::SetWasmExecutionBudget(
    uint32_t outputBufferBytes,
    uint32_t maxCommands,
    double maxExecutionMs) {
    WasmConfig next = config_.wasm;
    next.outputBufferBytes = outputBufferBytes;
    next.maxCommands = maxCommands;
    next.maxEventExecutionMs = maxExecutionMs;
    config_.wasm = config_internal::SanitizeWasmConfig(next);
    for (auto& host : wasmEffectHosts_) {
        if (!host) {
            continue;
        }
        host->SetExecutionBudget(BuildExecutionBudget(config_.wasm));
    }
    if (wasmIndicatorHost_) {
        wasmIndicatorHost_->SetExecutionBudget(BuildExecutionBudget(config_.wasm));
    }
    PersistConfig();
}

bool AppController::LoadWasmPluginFromManifestPath(
    const std::string& manifestPath,
    const std::string& surface,
    const std::string& effectChannel) {
    if (IsIndicatorSurface(surface)) {
        if (!wasmIndicatorHost_) {
            return false;
        }
        InputIndicatorConfig nextIndicator = config_.inputIndicator;
        nextIndicator.wasmManifestPath = manifestPath;
        nextIndicator = config_internal::SanitizeInputIndicatorConfig(nextIndicator);
        if (TrimAscii(nextIndicator.wasmManifestPath).empty()) {
            return false;
        }
        const bool ok =
            wasmIndicatorHost_->LoadPluginFromManifest(Utf8ToWString(nextIndicator.wasmManifestPath));
        if (ok) {
            config_.inputIndicator = std::move(nextIndicator);
            PersistConfig();
        }
        return ok;
    }

    bool hasEffectsHost = false;
    for (const auto& host : wasmEffectHosts_) {
        if (host) {
            hasEffectsHost = true;
            break;
        }
    }
    if (!hasEffectsHost) {
        return false;
    }

    const std::string normalizedChannel = TrimAscii(effectChannel);
    if (!normalizedChannel.empty()) {
        EffectsWasmLane lane = EffectsWasmLane::Click;
        if (!TryResolveEffectsLane(normalizedChannel, &lane)) {
            return false;
        }
        wasm::WasmEffectHost* host = wasmEffectHosts_[LaneIndex(lane)].get();
        if (!host) {
            return false;
        }

        WasmConfig next = config_.wasm;
        std::string* lanePath = MutableEffectsLaneManifestPath(&next, lane);
        if (!lanePath) {
            return false;
        }
        *lanePath = manifestPath;
        next = config_internal::SanitizeWasmConfig(next);
        const std::string resolvedManifestPath = ResolveEffectsConfiguredManifestPath(next, lane);
        if (resolvedManifestPath.empty()) {
            return false;
        }
        const bool ok = host->LoadPluginFromManifest(Utf8ToWString(resolvedManifestPath));
        if (ok) {
            config_.wasm = std::move(next);
            PersistConfig();
        }
        return ok;
    }

    WasmConfig next = config_.wasm;
    next.manifestPath = manifestPath;
    ClearEffectsLaneManifestPaths(&next);
    next = config_internal::SanitizeWasmConfig(next);
    if (next.manifestPath.empty()) {
        return false;
    }

    bool ok = true;
    for (auto& host : wasmEffectHosts_) {
        if (!host) {
            continue;
        }
        ok = host->LoadPluginFromManifest(Utf8ToWString(next.manifestPath)) && ok;
    }
    if (ok) {
        config_.wasm = std::move(next);
        PersistConfig();
    }
    return ok;
}

bool AppController::ShouldFallbackToBuiltinClickWhenWasmActive() const {
    return config_.wasm.fallbackToBuiltinClick;
}

} // namespace mousefx

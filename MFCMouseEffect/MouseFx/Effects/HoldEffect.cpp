#include "pch.h"
#include "HoldEffect.h"
#include "MouseFx/Effects/HoldEffectCommandAdapter.h"

#include "MouseFx/Core/Config/ConfigPathResolver.h"
#include "MouseFx/Core/Effects/HoldEffectCompute.h"
#include "MouseFx/Effects/RippleBasedHoldRuntime.h"
#include "MouseFx/Effects/HoldRouteCatalog.h"
#include "MouseFx/Renderers/HoldRuntimeRegistry.h"
#include "MouseFx/Renderers/Hold/Presentation/QuantumHaloPresenterSelection.h"
#include "MouseFx/Styles/ThemeStyle.h"
#include "MouseFx/Utils/TimeUtils.h"
#include "Platform/PlatformHoldRuntimeFactory.h"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace mousefx {

// ---------------------------------------------------------------------------
// Diagnostics helper (observability hook)
// ---------------------------------------------------------------------------

static void WriteHoldRuntimeSnapshot(
    const char* eventName,
    const std::string& type,
    bool running,
    const ScreenPoint& pt,
    uint32_t holdMs) {
    const std::wstring diagDir = ResolveLocalDiagDirectory();
    if (diagDir.empty()) return;
    std::error_code ec;
    std::filesystem::create_directories(diagDir, ec);
    if (ec) return;
    const std::filesystem::path outFile = std::filesystem::path(diagDir) / L"hold_runtime_auto.json";
    std::ofstream out(outFile, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return;
    std::ostringstream ss;
    ss << "{"
       << "\"event\":\"" << (eventName ? eventName : "") << "\","
       << "\"type\":\"" << type << "\","
       << "\"running\":" << (running ? "true" : "false") << ","
       << "\"x\":" << pt.x << ","
       << "\"y\":" << pt.y << ","
       << "\"hold_ms\":" << holdMs
       << "}";
    out << ss.str();
}

// ---------------------------------------------------------------------------
// Runtime creation helper
// ---------------------------------------------------------------------------

static std::unique_ptr<IHoldRuntime> CreateRuntime(
    const std::string& type,
    const std::string& themeName) {
    // 1. Try HoldRuntimeRegistry (for future extensibility)
    auto runtime = HoldRuntimeRegistry::Instance().Create(type);
    if (runtime) return runtime;

    // 2. Direct GPU path
    if (hold_route::IsQuantumHaloGpuV2DirectType(type)) {
        if (auto runtime = platform::CreatePlatformHoldRuntime(type)) {
            return runtime;
        }
    }

    // 3. Ripple-based path (default)
    const bool gpuV2 = hold_route::IsGpuV2RouteType(type);
    const bool chromatic = (ToLowerAscii(themeName) == "chromatic");
    return std::make_unique<RippleBasedHoldRuntime>(type, gpuV2, chromatic);
}

// ---------------------------------------------------------------------------
// HoldEffect
// ---------------------------------------------------------------------------

HoldEffect::HoldEffect(
    const std::string& themeName,
    const std::string& type,
    const std::string& followMode,
    const std::string& presenterBackend)
    : type_(type),
      followMode_(ParseHoldEffectFollowMode(followMode)) {
    style_ = GetThemePalette(themeName).hold;
    computeProfile_ = hold_effect_adapter::BuildHoldProfileFromStyle(style_);
    QuantumHaloPresenterSelection::SetConfiguredBackendPreference(
        QuantumHaloPresenterSelection::NormalizeBackendPreference(presenterBackend));
    runtime_ = CreateRuntime(type_, themeName);
}

HoldEffect::~HoldEffect() {
    Shutdown();
}

bool HoldEffect::Initialize() {
    return true;
}

void HoldEffect::Shutdown() {
    OnHoldEnd();
}

void HoldEffect::OnHoldStart(const ScreenPoint& pt, int button) {
    if (holdButton_ != 0) return; // Already holding

    holdPoint_ = pt;
    holdButton_ = button;
    followState_ = HoldEffectFollowState{};

    MouseButton holdMouseButton = MouseButton::Left;
    if (button == static_cast<int>(MouseButton::Right)) {
        holdMouseButton = MouseButton::Right;
    } else if (button == static_cast<int>(MouseButton::Middle)) {
        holdMouseButton = MouseButton::Middle;
    }
    const HoldEffectStartCommand command = ComputeHoldEffectStartCommand(
        pt,
        holdMouseButton,
        type_,
        computeProfile_);
    const RippleStyle runtimeStyle = hold_effect_adapter::BuildRuntimeStyleFromStartCommand(style_, command);

    if (runtime_) {
        runtime_->Start(runtimeStyle, command.overlayPoint);
        runtime_->Update(0u, command.overlayPoint);
    }

    WriteHoldRuntimeSnapshot(
        "hold_start", type_,
        runtime_ ? runtime_->IsRunning() : false,
        command.overlayPoint, 0);
}

void HoldEffect::OnHoldUpdate(const ScreenPoint& pt, uint32_t durationMs) {
    holdPoint_ = pt;

    const HoldEffectUpdateCommand command = ComputeHoldEffectUpdateCommand(
        pt,
        durationMs,
        NowMs(),
        followMode_,
        &followState_);
    if (command.emit && runtime_) {
        runtime_->Update(command.holdMs, command.overlayPoint);
    }
}

void HoldEffect::OnHoldEnd() {
    if (runtime_ && runtime_->IsRunning()) {
        runtime_->Stop();
    }

    WriteHoldRuntimeSnapshot(
        "hold_end", type_,
        false,
        holdPoint_, 0);

    holdButton_ = 0;
    followState_ = HoldEffectFollowState{};
}

void HoldEffect::OnCommand(const std::string& cmd, const std::string& args) {
    // Commands are handled internally by the runtime.
    (void)cmd;
    (void)args;
}

} // namespace mousefx

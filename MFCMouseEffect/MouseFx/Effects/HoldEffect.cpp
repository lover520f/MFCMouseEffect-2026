#include "pch.h"
#include "HoldEffect.h"

#include "MouseFx/Core/Config/ConfigPathResolver.h"
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
      followMode_(ParseFollowMode(followMode)) {
    style_ = GetThemePalette(themeName).hold;
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
    hasSmoothedPoint_ = false;
    lastEfficientPosMs_ = 0;

    if (runtime_) {
        runtime_->Start(style_, pt);
        runtime_->Update(0u, pt);
    }

    WriteHoldRuntimeSnapshot(
        "hold_start", type_,
        runtime_ ? runtime_->IsRunning() : false,
        pt, 0);
}

void HoldEffect::OnHoldUpdate(const ScreenPoint& pt, uint32_t durationMs) {
    holdPoint_ = pt;

    const uint64_t nowMs = NowMs();
    ScreenPoint outPt = pt;
    bool shouldUpdate = false;

    switch (followMode_) {
        case FollowMode::Precise:
            shouldUpdate = true;
            break;
        case FollowMode::Smooth: {
            const float alpha = 0.35f;
            if (!hasSmoothedPoint_) {
                smoothedX_ = (float)pt.x;
                smoothedY_ = (float)pt.y;
                hasSmoothedPoint_ = true;
            } else {
                smoothedX_ += ((float)pt.x - smoothedX_) * alpha;
                smoothedY_ += ((float)pt.y - smoothedY_) * alpha;
            }
            outPt.x = static_cast<int32_t>(std::lround(smoothedX_));
            outPt.y = static_cast<int32_t>(std::lround(smoothedY_));
            shouldUpdate = true;
            break;
        }
        case FollowMode::Efficient:
            if (nowMs - lastEfficientPosMs_ >= 20) {
                lastEfficientPosMs_ = nowMs;
                shouldUpdate = true;
            }
            break;
    }

    if (shouldUpdate && runtime_) {
        runtime_->Update(static_cast<uint32_t>(durationMs), outPt);
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
    hasSmoothedPoint_ = false;
    lastEfficientPosMs_ = 0;
}

void HoldEffect::OnCommand(const std::string& cmd, const std::string& args) {
    // Commands are handled internally by the runtime.
    (void)cmd;
    (void)args;
}

HoldEffect::FollowMode HoldEffect::ParseFollowMode(const std::string& mode) {
    std::string value = ToLowerAscii(mode);
    if (value == "precise") return FollowMode::Precise;
    if (value == "efficient") return FollowMode::Efficient;
    return FollowMode::Smooth;
}

bool HoldEffect::IsSamePoint(const ScreenPoint& a, const ScreenPoint& b) {
    return a.x == b.x && a.y == b.y;
}

} // namespace mousefx

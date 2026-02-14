// AppController.cpp

#include "pch.h"

#include "AppController.h"
#include "CommandHandler.h"
#include "DispatchRouter.h"
#include "MouseFxMessages.h"
#include "ConfigPathResolver.h"
#include "EffectFactory.h"
#include "OverlayHostService.h"
#include "JsonLite.h"
#include "MouseFx/ThirdParty/json.hpp"
#include "MouseFx/Utils/MathUtils.h"
#include "MouseFx/Utils/StringUtils.h"
#include "GpuProbeHelper.h"
#include "VmForegroundDetector.h"

#include <new>
#include <windowsx.h>  // For GET_X_LPARAM, GET_Y_LPARAM
#include <cctype>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <sstream>

namespace mousefx {

using json = nlohmann::json;

static const wchar_t* kDispatchClassName = L"MouseFxDispatchWindow";
static constexpr UINT_PTR kSelfTestTimerId = 0x4D46;

static std::string NormalizeHoldFollowMode(std::string mode) {
    mode = ToLowerAscii(mode);
    if (mode == "precise") return "precise";
    if (mode == "efficient") return "efficient";
    return "smooth";
}

static std::string NormalizeHoldEffectTypeAlias(const std::string& type) {
    // Backward compatibility for renamed GPU hold effect id.
    if (type == "hold_neon3d_gpu_v2") {
        return "hold_quantum_halo_gpu_v2";
    }
    return type;
}


AppController::AppController()
    : commandHandler_(std::make_unique<CommandHandler>(this))
    , dispatchRouter_(std::make_unique<DispatchRouter>(this)) {}

AppController::~AppController() {
    Stop();
}

EffectConfig AppController::GetConfigSnapshot() const {
    if (!dispatchHwnd_ || !IsWindow(dispatchHwnd_)) {
        return config_;
    }
    if (GetWindowThreadProcessId(dispatchHwnd_, nullptr) == GetCurrentThreadId()) {
        return config_;
    }

    EffectConfig snapshot{};
    SendMessageW(dispatchHwnd_, WM_MFX_GET_CONFIG, 0, reinterpret_cast<LPARAM>(&snapshot));
    return snapshot;
}

void AppController::PersistConfig() {
    if (!configDir_.empty()) {
        EffectConfig::Save(configDir_, config_);
    }
}

std::string AppController::ResolveRuntimeEffectType(
    EffectCategory category,
    const std::string& requestedType,
    std::string* outReason) const {
    if (outReason) outReason->clear();
    if (category != EffectCategory::Hold) {
        return requestedType;
    }
    const std::string normalizedType = NormalizeHoldEffectTypeAlias(requestedType);
    const bool isHoldQuantumHaloGpuV2 = (normalizedType == "hold_quantum_halo_gpu_v2");
    const bool isHoldFluxGpuV2 = (normalizedType == "hold_fluxfield_gpu_v2");
    if (!isHoldQuantumHaloGpuV2 && !isHoldFluxGpuV2) {
        return normalizedType;
    }

    if (isHoldFluxGpuV2) {
        if (outReason) *outReason = "flux_gpu_v2_d3d11_compute_route";
        return normalizedType;
    }
    if (isHoldQuantumHaloGpuV2) {
        if (outReason) *outReason = "quantum_halo_gpu_v2_d3d11_dcomp_direct_runtime_route";
        return normalizedType;
    }

    const DawnRuntimeProbeResult probe = ProbeDawnRuntimeOnce();
    if (!probe.available) {
        if (outReason) *outReason = probe.reason;
        return normalizedType;
    }

    // Runtime binary is loadable; keep gpu-v2 route selected.
    // Current renderer is placeholder and will be replaced by Dawn backend in later stages.
    if (outReason) *outReason = "dawn_runtime_ready_placeholder_renderer";
    return normalizedType;
}

void AppController::NotifyGpuFallbackIfNeeded(const std::string& reason) {
    if (gpuFallbackNotifiedThisSession_) return;
    gpuFallbackNotifiedThisSession_ = true;
    // UX decision: do not block runtime with modal dialogs.
    // Fallback status is exposed through Web settings state and local diagnostics.
#ifdef _DEBUG
    std::wstring dbg = L"MouseFx: GPU route fallback detected. reason=";
    dbg += Utf8ToWString(reason);
    dbg += L"\n";
    OutputDebugStringW(dbg.c_str());
#else
    (void)reason;
#endif
}

void AppController::WriteGpuRouteStatusSnapshot(
    EffectCategory category,
    const std::string& requestedType,
    const std::string& effectiveType,
    const std::string& reason) const {
    if (category != EffectCategory::Hold) {
        return;
    }
    const std::wstring diagDir = ResolveLocalDiagDirectory();
    if (diagDir.empty()) return;
    std::error_code ec;
    std::filesystem::create_directories(diagDir, ec);
    if (ec) return;
    const std::filesystem::path file = std::filesystem::path(diagDir) / L"gpu_route_status_auto.json";
    std::ofstream out(file, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) return;
    const std::string requestedNormalized = NormalizeHoldEffectTypeAlias(requestedType);
    std::ostringstream ss;
    ss << "{"
       << "\"category\":\"hold\","
       << "\"requested\":\"" << requestedType << "\","
       << "\"requested_normalized\":\"" << requestedNormalized << "\","
       << "\"effective\":\"" << effectiveType << "\","
       << "\"fallback_applied\":" << (requestedNormalized == effectiveType ? "false" : "true") << ","
       << "\"reason\":\"" << reason << "\""
       << "}";
    out << ss.str();
}

void AppController::SetActiveEffectType(EffectCategory category, const std::string& type) {
    switch (category) {
        case EffectCategory::Click: config_.active.click = type; break;
        case EffectCategory::Trail: config_.active.trail = type; break;
        case EffectCategory::Scroll: config_.active.scroll = type; break;
        case EffectCategory::Hover: config_.active.hover = type; break;
        case EffectCategory::Hold: config_.active.hold = type; break;
        default: break;
    }
}

bool AppController::Start() {
    if (dispatchHwnd_) return true;
    diag_ = {};

    // Load config from the best available directory (AppData preferred)
    configDir_ = ResolveConfigDirectory();
    config_ = EffectConfig::Load(configDir_);
    inputIndicatorOverlay_.Initialize();
    inputIndicatorOverlay_.UpdateConfig(config_.inputIndicator);

    diag_.stage = StartStage::GdiPlusStartup;
    if (!gdiplus_.Startup()) {
#ifdef _DEBUG
        OutputDebugStringW(L"MouseFx: GDI+ startup failed.\n");
#endif
        return false;
    }

    diag_.stage = StartStage::DispatchWindow;
    if (!CreateDispatchWindow()) {
#ifdef _DEBUG
        OutputDebugStringW(L"MouseFx: dispatch window creation failed.\n");
#endif
        Stop();
        return false;
    }

    // Initialize effects with defaults
    diag_.stage = StartStage::EffectInit;
    const std::string clickType = config_.active.click.empty()
        ? (config_.defaultEffect.empty() ? "ripple" : config_.defaultEffect)
        : config_.active.click;
    SetEffect(EffectCategory::Click, clickType);
    SetEffect(EffectCategory::Trail, config_.active.trail);
    SetEffect(EffectCategory::Scroll, config_.active.scroll);
    SetEffect(EffectCategory::Hold, config_.active.hold);
    SetEffect(EffectCategory::Hover, config_.active.hover);
    inputIndicatorOverlay_.UpdateConfig(config_.inputIndicator);

    bool normalizedChanged = false;
    auto normalizeActive = [&](EffectCategory category, std::string* slot) {
        if (!slot) return;
        std::string reason;
        const std::string effective = ResolveRuntimeEffectType(category, *slot, &reason);
        if (*slot != effective) {
            *slot = effective;
            normalizedChanged = true;
        }
    };
    normalizeActive(EffectCategory::Click, &config_.active.click);
    normalizeActive(EffectCategory::Trail, &config_.active.trail);
    normalizeActive(EffectCategory::Scroll, &config_.active.scroll);
    normalizeActive(EffectCategory::Hold, &config_.active.hold);
    normalizeActive(EffectCategory::Hover, &config_.active.hover);
    if (normalizedChanged) {
        PersistConfig();
    }

    lastInputTime_ = GetTickCount64();
    SetTimer(dispatchHwnd_, kHoverTimerId, 100, nullptr);

    diag_.stage = StartStage::GlobalHook;
    if (!hook_.Start(dispatchHwnd_)) {
#ifdef _DEBUG
        wchar_t buf[256]{};
        wsprintfW(buf, L"MouseFx: global hook start failed. GetLastError=%lu\n", hook_.LastError());
        OutputDebugStringW(buf);
#endif
        diag_.error = hook_.LastError();
        Stop();
        return false;
    }

//#ifdef _DEBUG
//    SetTimer(dispatchHwnd_, kSelfTestTimerId, 250, nullptr);
//#endif
    return true;
}

void AppController::Stop() {
    hook_.Stop();
    inputIndicatorOverlay_.Shutdown();
    for (auto& effect : effects_) {
        if (effect) {
            effect->Shutdown();
            effect.reset();
        }
    }
    OverlayHostService::Instance().Shutdown();
    DestroyDispatchWindow();
    gdiplus_.Shutdown();
}

// (Moved to top)
// Hold renderers are included in HoldEffect.cpp, but safe to include here too if needed, 
// though generally we rely on the creation site.
// Actually, AppController creates the Effects, but Effects create the Renderers (mostly).
// Except simpler effects might fallback?

std::unique_ptr<IMouseEffect> AppController::CreateEffect(EffectCategory category, const std::string& type) {
    return EffectFactory::Create(category, type, config_);
}

void AppController::SetEffect(EffectCategory category, const std::string& type) {
    size_t idx = static_cast<size_t>(category);
    if (idx >= kCategoryCount) return;

    std::string fallbackReason;
    const std::string requestedNormalized =
        (category == EffectCategory::Hold) ? NormalizeHoldEffectTypeAlias(type) : type;
    const std::string effectiveType = ResolveRuntimeEffectType(category, type, &fallbackReason);
    if (!fallbackReason.empty() && effectiveType != requestedNormalized) {
        NotifyGpuFallbackIfNeeded(fallbackReason);
    }
    WriteGpuRouteStatusSnapshot(category, type, effectiveType, fallbackReason);

    // Shutdown existing effect for this category
    if (effects_[idx]) {
        effects_[idx]->Shutdown();
        effects_[idx].reset();
    }

    // Create and initialize new effect
    effects_[idx] = CreateEffect(category, effectiveType);
    if (effects_[idx] && !vmEffectsSuppressed_) {
        effects_[idx]->Initialize();
    }

#ifdef _DEBUG
    wchar_t buf[256]{};
    wsprintfW(buf, L"MouseFx: SetEffect category=%hs type=%hs\n", 
              CategoryToString(category), effectiveType.c_str());
    OutputDebugStringW(buf);
#endif
}

void AppController::ClearEffect(EffectCategory category) {
    SetEffect(category, "none");
}

void AppController::SetTheme(const std::string& theme) {
    if (theme.empty()) return;
    config_.theme = theme;
    // Re-create themed effects to pick up new palette.
    SetEffect(EffectCategory::Scroll, config_.active.scroll);
    SetEffect(EffectCategory::Hold, config_.active.hold);
    SetEffect(EffectCategory::Hover, config_.active.hover);
    PersistConfig();
}

void AppController::SetUiLanguage(const std::string& lang) {
    if (lang.empty()) return;
    config_.uiLanguage = lang;
    PersistConfig();
}

void AppController::SetTextEffectContent(const std::vector<std::wstring>& texts) {
    config_.textClick.texts = texts;
    PersistConfig();
    // Note: TextEffect pulls from config each click, so no need to "reload" the effect object
    // unless we want to refresh its internal pool immediately.
    // TextEffect::Initialize() builds the pool.
    // We should probably re-initialize the text effect if it's active.
    if (auto* effect = GetEffect(EffectCategory::Click)) {
        // Simple way: re-set it to trigger re-init
        if (config_.active.click == "text") {
            SetEffect(EffectCategory::Click, "text");
        }
    }
}

void AppController::SetTextEffectFontSize(float sizePt) {
    const float clamped = ClampFloat(sizePt, 6.0f, 96.0f);
    if (std::fabs(config_.textClick.fontSize - clamped) < 0.01f) return;
    config_.textClick.fontSize = clamped;
    PersistConfig();
    if (auto* effect = GetEffect(EffectCategory::Click)) {
        if (config_.active.click == "text") {
            SetEffect(EffectCategory::Click, "text");
        }
    }
}

void AppController::SetInputIndicatorConfig(const InputIndicatorConfig& cfg) {
    config_.inputIndicator = cfg;
    inputIndicatorOverlay_.UpdateConfig(config_.inputIndicator);
    PersistConfig();
}

void AppController::SetHoldFollowMode(const std::string& mode) {
    const std::string normalized = NormalizeHoldFollowMode(mode);
    if (config_.holdFollowMode == normalized) return;
    config_.holdFollowMode = normalized;
    PersistConfig();
    if (!config_.active.hold.empty() && config_.active.hold != "none") {
        SetEffect(EffectCategory::Hold, config_.active.hold);
    }
}

void AppController::SetTrailTuning(const std::string& style, const TrailProfilesConfig& profiles, const TrailRendererParamsConfig& params) {
    config_.trailStyle = style.empty() ? "custom" : style;
    config_.trailProfiles = profiles;
    config_.trailParams = params;
    PersistConfig();

    // Recreate current trail effect to apply immediately (if any).
    if (!config_.active.trail.empty() && config_.active.trail != "none") {
        SetEffect(EffectCategory::Trail, config_.active.trail);
    }
}

void AppController::ResetConfig() {
    // 1. Get default config
    config_ = EffectConfig::GetDefault();
    
    // 2. Save it to disk
    PersistConfig();

    // 3. Re-apply everything
    SetEffect(EffectCategory::Click, config_.active.click);
    SetEffect(EffectCategory::Trail, config_.active.trail);
    SetEffect(EffectCategory::Scroll, config_.active.scroll);
    SetEffect(EffectCategory::Hold, config_.active.hold);
    SetEffect(EffectCategory::Hover, config_.active.hover);
    inputIndicatorOverlay_.UpdateConfig(config_.inputIndicator);
    
    // Theme/Language rely on being pulled by UI or re-applied if needed?
    // SettingsWnd calls sync, so it will pull new values.
    // But existing effects might need theme re-apply.
    SetTheme(config_.theme);
}

void AppController::ReloadConfigFromDisk() {
    if (configDir_.empty()) return;

    EffectConfig loaded = EffectConfig::Load(configDir_);
    config_ = loaded;

    const std::string clickType = config_.active.click.empty()
        ? (config_.defaultEffect.empty() ? "ripple" : config_.defaultEffect)
        : config_.active.click;

    SetEffect(EffectCategory::Click, clickType);
    SetEffect(EffectCategory::Trail, config_.active.trail);
    SetEffect(EffectCategory::Scroll, config_.active.scroll);
    SetEffect(EffectCategory::Hold, config_.active.hold);
    SetEffect(EffectCategory::Hover, config_.active.hover);

    bool normalizedChanged = false;
    auto normalizeActive = [&](EffectCategory category, std::string* slot) {
        if (!slot) return;
        std::string reason;
        const std::string effective = ResolveRuntimeEffectType(category, *slot, &reason);
        if (*slot != effective) {
            *slot = effective;
            normalizedChanged = true;
        }
    };
    normalizeActive(EffectCategory::Click, &config_.active.click);
    normalizeActive(EffectCategory::Trail, &config_.active.trail);
    normalizeActive(EffectCategory::Scroll, &config_.active.scroll);
    normalizeActive(EffectCategory::Hold, &config_.active.hold);
    normalizeActive(EffectCategory::Hover, &config_.active.hover);
    if (normalizedChanged) {
        PersistConfig();
    }

#ifdef _DEBUG
    OutputDebugStringW(L"MouseFx: reload_config applied.\n");
#endif
}

IMouseEffect* AppController::GetEffect(EffectCategory category) const {
    size_t idx = static_cast<size_t>(category);
    if (idx >= kCategoryCount) return nullptr;
    return effects_[idx].get();
}

void AppController::HandleCommand(const std::string& jsonCmd) {
    if (!dispatchHwnd_) return;

    // Thread Safety: Marshal to UI thread if we are on a background thread.
    if (GetWindowThreadProcessId(dispatchHwnd_, nullptr) != GetCurrentThreadId()) {
        SendMessageW(dispatchHwnd_, WM_MFX_EXEC_CMD, 0, reinterpret_cast<LPARAM>(&jsonCmd));
        return;
    }

    commandHandler_->Handle(jsonCmd);
}

bool AppController::CreateDispatchWindow() {
    if (dispatchHwnd_) return true;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &AppController::DispatchWndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.lpszClassName = kDispatchClassName;
    if (RegisterClassExW(&wc) == 0 && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
        diag_.error = GetLastError();
        return false;
    }

    dispatchHwnd_ = CreateWindowExW(
        0, kDispatchClassName, L"", 0,
        0, 0, 0, 0,
        HWND_MESSAGE, nullptr,
        GetModuleHandleW(nullptr), this
    );
    if (!dispatchHwnd_) {
        diag_.error = GetLastError();
    }
    return dispatchHwnd_ != nullptr;
}

void AppController::DestroyDispatchWindow() {
    if (dispatchHwnd_) {
        DestroyWindow(dispatchHwnd_);
        dispatchHwnd_ = nullptr;
    }
}

void AppController::UpdateVmSuppressionState() {
    const uint64_t now = GetTickCount64();
    const bool suppress = vmForegroundDetector_.ShouldSuppress(now);
    if (suppress == vmEffectsSuppressed_) return;
    ApplyVmSuppression(suppress);
}

void AppController::ApplyVmSuppression(bool suppressed) {
    if (suppressed) {
        SuspendEffectsForVm();
    } else {
        ResumeEffectsAfterVm();
    }
    vmEffectsSuppressed_ = suppressed;
}

void AppController::SuspendEffectsForVm() {
    if (dispatchHwnd_) {
        KillTimer(dispatchHwnd_, kHoldTimerId);
    }
    pendingHold_.active = false;
    ignoreNextClick_ = false;
    holdButtonDown_ = false;
    holdDownTick_ = 0;
    hovering_ = false;
    inputIndicatorOverlay_.Hide();

    for (auto& effect : effects_) {
        if (effect) effect->Shutdown();
    }
}

void AppController::ResumeEffectsAfterVm() {
    for (auto& effect : effects_) {
        if (effect) effect->Initialize();
    }
}

LRESULT CALLBACK AppController::DispatchWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    AppController* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<AppController*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->dispatchHwnd_ = hwnd;
    } else {
        self = reinterpret_cast<AppController*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->dispatchRouter_->Route(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}


} // namespace mousefx

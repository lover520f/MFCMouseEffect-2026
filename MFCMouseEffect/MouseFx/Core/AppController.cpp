// AppController.cpp

#include "pch.h"

#include "AppController.h"
#include "MouseFxMessages.h"
#include "ConfigPathResolver.h"
#include "EffectFactory.h"
#include "OverlayHostService.h"
#include "JsonLite.h"
#include "MouseFx/ThirdParty/json.hpp"

#include <new>
#include <windowsx.h>  // For GET_X_LPARAM, GET_Y_LPARAM
#include <cctype>

namespace mousefx {

using json = nlohmann::json;

static const wchar_t* kDispatchClassName = L"MouseFxDispatchWindow";
static constexpr UINT_PTR kSelfTestTimerId = 0x4D46;

static std::string TrimAscii(std::string s) {
    auto is_space = [](unsigned char ch) {
        return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
    };
    size_t b = 0;
    while (b < s.size() && is_space((unsigned char)s[b])) b++;
    size_t e = s.size();
    while (e > b && is_space((unsigned char)s[e - 1])) e--;
    if (b == 0 && e == s.size()) return s;
    return s.substr(b, e - b);
}

static std::string ToLowerAscii(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return s;
}

static std::string NormalizeHoldFollowMode(std::string mode) {
    mode = ToLowerAscii(mode);
    if (mode == "precise") return "precise";
    if (mode == "efficient") return "efficient";
    return "smooth";
}

static std::string NormalizeRenderBackend(std::string backend) {
    backend = ToLowerAscii(backend);
    if (backend == "dawn") return "dawn";
    if (backend == "cpu") return "cpu";
    return "auto";
}

static std::string NormalizeGpuBridgeMode(std::string mode) {
    mode = ToLowerAscii(mode);
    if (mode == "compositor") return "compositor";
    return "host_compat";
}

static std::wstring Utf8ToWString(const std::string& s) {
    if (s.empty()) return {};
    int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
    if (len <= 0) return {};
    std::wstring out((size_t)len, L'\0');
    int written = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, out.empty() ? nullptr : &out[0], len);
    if (written <= 0) return {};
    if (!out.empty() && out.back() == L'\0') out.pop_back();
    return out;
}

static int ClampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static float ClampFloat(float x, float lo, float hi) {
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

AppController::AppController() = default;

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
    deferredBackendApplyPending_ = false;
    deferredDawnUpgradePending_ = false;
    deferredDawnUpgradeRetryCount_ = 0;

    // Load config from the best available directory (AppData preferred)
    configDir_ = ResolveConfigDirectory();
    config_ = EffectConfig::Load(configDir_);
    config_.renderBackend = NormalizeRenderBackend(config_.renderBackend);
    OverlayHostService::Instance().SetGpuBridgeModeRequest(config_.gpuBridgeModeRequest);
    // Startup tightening:
    // - If configured backend is dawn/auto and queue is already ready, start directly on target backend.
    // - Otherwise keep target preference and let async probe promote to Dawn.
    if (config_.renderBackend == "cpu") {
        (void)OverlayHostService::Instance().SetRenderBackendPreference("cpu");
        deferredBackendApplyPending_ = false;
    } else {
        (void)OverlayHostService::Instance().SetRenderBackendPreference(config_.renderBackend);
        deferredBackendApplyPending_ =
            (OverlayHostService::Instance().GetActiveRenderBackend() != "dawn");
    }

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

    if (deferredBackendApplyPending_) {
        SetTimer(dispatchHwnd_, kDeferredBackendTimerId, 1, nullptr);
    }

//#ifdef _DEBUG
//    SetTimer(dispatchHwnd_, kSelfTestTimerId, 250, nullptr);
//#endif
    return true;
}

void AppController::Stop() {
    hook_.Stop();
    deferredBackendApplyPending_ = false;
    deferredDawnUpgradePending_ = false;
    deferredDawnUpgradeRetryCount_ = 0;
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

    // Shutdown existing effect for this category
    if (effects_[idx]) {
        effects_[idx]->Shutdown();
        effects_[idx].reset();
    }

    // Create and initialize new effect
    effects_[idx] = CreateEffect(category, type);
    if (effects_[idx]) {
        effects_[idx]->Initialize();
    }

#ifdef _DEBUG
    wchar_t buf[256]{};
    wsprintfW(buf, L"MouseFx: SetEffect category=%hs type=%hs\n", 
              CategoryToString(category), type.c_str());
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

void AppController::SetRenderBackend(const std::string& backend) {
    const std::string normalized = NormalizeRenderBackend(backend);
    if (config_.renderBackend == normalized) return;
    config_.renderBackend = normalized;
    PersistConfig();

    if (normalized == "cpu") {
        deferredDawnUpgradePending_ = false;
        deferredDawnUpgradeRetryCount_ = 0;
        if (dispatchHwnd_) {
            KillTimer(dispatchHwnd_, kDeferredBackendTimerId);
        }
    }

    const bool shouldRecreateNow = OverlayHostService::Instance().SetRenderBackendPreference(normalized);
    if (shouldRecreateNow) {
        if (normalized != "cpu" && !IsBackendSwitchIdleWindow()) {
            deferredDawnUpgradePending_ = true;
            deferredDawnUpgradeRetryCount_ = 0;
            if (dispatchHwnd_) {
                SetTimer(dispatchHwnd_, kDeferredBackendTimerId, 80, nullptr);
            }
        } else {
            RecreateActiveEffects();
        }
    }

    if ((normalized == "dawn" || normalized == "auto") &&
        !OverlayHostService::Instance().IsDawnQueueReady()) {
        deferredDawnUpgradePending_ = true;
        deferredDawnUpgradeRetryCount_ = 0;
        if (dispatchHwnd_) {
            SetTimer(dispatchHwnd_, kDeferredBackendTimerId, 200, nullptr);
        }
    } else {
        deferredDawnUpgradePending_ = false;
        deferredDawnUpgradeRetryCount_ = 0;
    }
}

void AppController::SetGpuBridgeModeRequest(const std::string& mode) {
    const std::string normalized = NormalizeGpuBridgeMode(mode);
    const bool changed = (config_.gpuBridgeModeRequest != normalized);
    if (changed) {
        config_.gpuBridgeModeRequest = normalized;
        PersistConfig();
    }
    OverlayHostService::Instance().SetGpuBridgeModeRequest(normalized);
    if (config_.renderBackend == "cpu") {
        return;
    }
    if (OverlayHostService::Instance().GetActiveRenderBackend() == "dawn" ||
        OverlayHostService::Instance().IsDawnQueueReady()) {
        RecreateActiveEffects();
        return;
    }
    deferredDawnUpgradePending_ = true;
    deferredDawnUpgradeRetryCount_ = 0;
    if (dispatchHwnd_) {
        SetTimer(dispatchHwnd_, kDeferredBackendTimerId, 200, nullptr);
    }
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
    OverlayHostService::Instance().SetGpuBridgeModeRequest(config_.gpuBridgeModeRequest);
    const bool shouldRecreateNow = OverlayHostService::Instance().SetRenderBackendPreference(config_.renderBackend);
    if (shouldRecreateNow) {
        RecreateActiveEffects();
    } else if (config_.renderBackend != "cpu") {
        deferredDawnUpgradePending_ = true;
        deferredDawnUpgradeRetryCount_ = 0;
        if (dispatchHwnd_) {
            SetTimer(dispatchHwnd_, kDeferredBackendTimerId, 200, nullptr);
        }
    }
    
    // Theme/Language rely on being pulled by UI or re-applied if needed?
    // SettingsWnd calls sync, so it will pull new values.
    // But existing effects might need theme re-apply.
    SetTheme(config_.theme);
}

void AppController::ReloadConfigFromDisk() {
    if (configDir_.empty()) return;

    EffectConfig loaded = EffectConfig::Load(configDir_);
    config_ = loaded;
    OverlayHostService::Instance().SetGpuBridgeModeRequest(config_.gpuBridgeModeRequest);
    const bool shouldRecreateNow = OverlayHostService::Instance().SetRenderBackendPreference(config_.renderBackend);
    if (shouldRecreateNow) {
        RecreateActiveEffects();
    } else if (config_.renderBackend != "cpu") {
        deferredDawnUpgradePending_ = true;
        deferredDawnUpgradeRetryCount_ = 0;
        if (dispatchHwnd_) {
            SetTimer(dispatchHwnd_, kDeferredBackendTimerId, 200, nullptr);
        }
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

bool AppController::IsBackendSwitchIdleWindow() const {
    const uint64_t now = GetTickCount64();
    const uint64_t elapsed = (now >= lastInputTime_) ? (now - lastInputTime_) : 0;
    return elapsed >= kDeferredBackendIdleThresholdMs;
}

bool AppController::IsLikelySystemWindowDrag(HWND dispatchHwnd) const {
    // During window move/resize, system usually captures mouse to the dragged window.
    // We treat "captured by another hwnd while button is down" as drag-like workload.
    const HWND capture = GetCapture();
    return (capture != nullptr && capture != dispatchHwnd);
}

bool AppController::ShouldDispatchDragMove(uint64_t nowTick) {
    const uint64_t elapsed = (nowTick >= lastDragMoveDispatchTick_)
        ? (nowTick - lastDragMoveDispatchTick_)
        : 0;
    if (elapsed < kWindowDragDispatchIntervalMs) {
        return false;
    }
    lastDragMoveDispatchTick_ = nowTick;
    return true;
}

bool AppController::IsHoldNeon3DTypeName(const std::string& type) {
    const std::string t = ToLowerAscii(type);
    return t == "neon3d" || t == "hold_neon3d";
}

void AppController::SetTrailLatencyPriorityMode(bool enabled) {
    if (trailLatencyPriorityActive_ == enabled) return;
    trailLatencyPriorityActive_ = enabled;
    if (auto* effect = GetEffect(EffectCategory::Trail)) {
        effect->OnCommand("latency_priority", enabled ? "on" : "off");
    }
    OverlayHostService::Instance().RequestImmediateFrame();
}

bool AppController::ShouldPrioritizeHoldLatency() const {
    if (!holdEffectRunning_) return false;
    if (!IsHoldNeon3DTypeName(config_.active.hold)) return false;
    if (OverlayHostService::Instance().GetActiveRenderBackend() != "dawn") return false;
    // When final present is still layered CPU fallback, prioritize hold follow latency.
    return !OverlayHostService::Instance().IsGpuPresentActive();
}

void AppController::RecreateActiveEffects() {
    const std::string clickType = config_.active.click.empty()
        ? (config_.defaultEffect.empty() ? "ripple" : config_.defaultEffect)
        : config_.active.click;
    SetEffect(EffectCategory::Click, clickType);
    SetEffect(EffectCategory::Trail, config_.active.trail);
    SetEffect(EffectCategory::Scroll, config_.active.scroll);
    SetEffect(EffectCategory::Hold, config_.active.hold);
    SetEffect(EffectCategory::Hover, config_.active.hover);
}

void AppController::HandleCommand(const std::string& jsonCmd) {
    if (!dispatchHwnd_) return;

    // Thread Safety: Marshal to UI thread if we are on a background thread.
    // SendMessage is synchronous, so the string on the caller's stack is safe to pass by pointer.
    if (GetWindowThreadProcessId(dispatchHwnd_, nullptr) != GetCurrentThreadId()) {
        SendMessageW(dispatchHwnd_, WM_MFX_EXEC_CMD, 0, reinterpret_cast<LPARAM>(&jsonCmd));
        return;
    }

    std::string cmd = ExtractJsonStringValue(jsonCmd, "cmd");
    
    if (cmd == "set_effect") {
        std::string category = ExtractJsonStringValue(jsonCmd, "category");
        std::string type = ExtractJsonStringValue(jsonCmd, "type");
        
        if (category.empty()) {
            // Legacy format: {"cmd": "set_effect", "type": "ripple"}
            // Assume click category for backward compatibility
            SetEffect(EffectCategory::Click, type);
            SetActiveEffectType(EffectCategory::Click, type);
        } else {
            const auto cat = CategoryFromString(category);
            SetEffect(cat, type);
            SetActiveEffectType(cat, type);
        }
        PersistConfig();
    } else if (cmd == "clear_effect") {
        std::string category = ExtractJsonStringValue(jsonCmd, "category");
        const auto cat = CategoryFromString(category);
        ClearEffect(cat);
        SetActiveEffectType(cat, "none");
        PersistConfig();
    } else if (cmd == "set_theme") {
        std::string theme = ExtractJsonStringValue(jsonCmd, "theme");
        SetTheme(theme);
    } else if (cmd == "set_render_backend") {
        std::string backend = ExtractJsonStringValue(jsonCmd, "backend");
        SetRenderBackend(backend);
    } else if (cmd == "set_gpu_bridge_mode") {
        std::string mode = ExtractJsonStringValue(jsonCmd, "mode");
        SetGpuBridgeModeRequest(mode);
    } else if (cmd == "set_ui_language") {
        std::string lang = ExtractJsonStringValue(jsonCmd, "lang");
        SetUiLanguage(lang);
    } else if (cmd == "effect_cmd") {
        // Generic command pass-through: { "cmd": "effect_cmd", "category": "hold", "command": "speed", "args": "2.0" }
        std::string category = ExtractJsonStringValue(jsonCmd, "category");
        std::string command = ExtractJsonStringValue(jsonCmd, "command");
        std::string args = ExtractJsonStringValue(jsonCmd, "args");
        
        if (!category.empty()) {
            const auto cat = CategoryFromString(category);
            if (auto* effect = GetEffect(cat)) {
                effect->OnCommand(command, args);
            }
        }
    } else if (cmd == "reload_config") {
        ReloadConfigFromDisk();
    } else if (cmd == "reset_config") {
        ResetConfig();
    } else if (cmd == "apply_settings") {
        json root;
        try {
            root = json::parse(jsonCmd);
        } catch (...) {
            return;
        }
        if (!root.contains("payload") || !root["payload"].is_object()) return;
        const json& p = root["payload"];

        // Active effects first (theme application recreates themed effects).
        bool activeChanged = false;
        if (p.contains("active") && p["active"].is_object()) {
            auto applyActive = [&](EffectCategory category, const char* key) {
                const json& a = p["active"];
                if (!a.contains(key) || !a[key].is_string()) return;
                std::string type = a[key].get<std::string>();
                if (type.empty()) return;

                if (type == "none") {
                    ClearEffect(category);
                } else {
                    SetEffect(category, type);
                }
                SetActiveEffectType(category, type);
                activeChanged = true;
            };

            applyActive(EffectCategory::Click, "click");
            applyActive(EffectCategory::Trail, "trail");
            applyActive(EffectCategory::Scroll, "scroll");
            applyActive(EffectCategory::Hold, "hold");
            applyActive(EffectCategory::Hover, "hover");
        }
        if (activeChanged) {
            PersistConfig();
        }

        if (p.contains("ui_language") && p["ui_language"].is_string()) {
            SetUiLanguage(p["ui_language"].get<std::string>());
        }

        if (p.contains("text_content") && p["text_content"].is_string()) {
            std::vector<std::wstring> texts;
            std::string raw = p["text_content"].get<std::string>();
            size_t start = 0;
            size_t end = raw.find(',');
            while (end != std::string::npos) {
                std::string token = TrimAscii(raw.substr(start, end - start));
                if (!token.empty()) texts.push_back(Utf8ToWString(token));
                start = end + 1;
                end = raw.find(',', start);
            }
            std::string last = TrimAscii(raw.substr(start));
            if (!last.empty()) texts.push_back(Utf8ToWString(last));
            SetTextEffectContent(texts);
        }

        if (p.contains("hold_follow_mode") && p["hold_follow_mode"].is_string()) {
            SetHoldFollowMode(p["hold_follow_mode"].get<std::string>());
        }
        if (p.contains("render_backend") && p["render_backend"].is_string()) {
            SetRenderBackend(p["render_backend"].get<std::string>());
        }
        if (p.contains("gpu_bridge_mode_request") && p["gpu_bridge_mode_request"].is_string()) {
            SetGpuBridgeModeRequest(p["gpu_bridge_mode_request"].get<std::string>());
        }

        // Trail tuning (optional fields).
        bool trailTouched = false;
        std::string style = config_.trailStyle.empty() ? "default" : config_.trailStyle;
        TrailProfilesConfig profiles = config_.trailProfiles;
        TrailRendererParamsConfig params = config_.trailParams;

        if (p.contains("trail_style") && p["trail_style"].is_string()) {
            style = p["trail_style"].get<std::string>();
            trailTouched = true;
        }

        auto applyProfile = [&](const char* key, TrailHistoryProfile& dst) {
            if (!p.contains("trail_profiles") || !p["trail_profiles"].is_object()) return;
            const json& tp = p["trail_profiles"];
            if (!tp.contains(key) || !tp[key].is_object()) return;
            const json& o = tp[key];
            if (o.contains("duration_ms") && o["duration_ms"].is_number_integer()) {
                dst.durationMs = ClampInt(o["duration_ms"].get<int>(), 80, 2000);
                trailTouched = true;
            }
            if (o.contains("max_points") && o["max_points"].is_number_integer()) {
                dst.maxPoints = ClampInt(o["max_points"].get<int>(), 2, 240);
                trailTouched = true;
            }
        };

        applyProfile("line", profiles.line);
        applyProfile("streamer", profiles.streamer);
        applyProfile("electric", profiles.electric);
        applyProfile("meteor", profiles.meteor);
        applyProfile("tubes", profiles.tubes);

        if (p.contains("trail_params") && p["trail_params"].is_object()) {
            const json& k = p["trail_params"];
            if (k.contains("streamer") && k["streamer"].is_object()) {
                const json& s = k["streamer"];
                if (s.contains("glow_width_scale") && s["glow_width_scale"].is_number()) {
                    params.streamer.glowWidthScale = ClampFloat(s["glow_width_scale"].get<float>(), 0.5f, 4.0f);
                    trailTouched = true;
                }
                if (s.contains("core_width_scale") && s["core_width_scale"].is_number()) {
                    params.streamer.coreWidthScale = ClampFloat(s["core_width_scale"].get<float>(), 0.2f, 2.0f);
                    trailTouched = true;
                }
                if (s.contains("head_power") && s["head_power"].is_number()) {
                    params.streamer.headPower = ClampFloat(s["head_power"].get<float>(), 0.8f, 3.0f);
                    trailTouched = true;
                }
            }
            if (k.contains("electric") && k["electric"].is_object()) {
                const json& e = k["electric"];
                if (e.contains("amplitude_scale") && e["amplitude_scale"].is_number()) {
                    params.electric.amplitudeScale = ClampFloat(e["amplitude_scale"].get<float>(), 0.2f, 3.0f);
                    trailTouched = true;
                }
                if (e.contains("fork_chance") && e["fork_chance"].is_number()) {
                    params.electric.forkChance = ClampFloat(e["fork_chance"].get<float>(), 0.0f, 0.5f);
                    trailTouched = true;
                }
            }
            if (k.contains("meteor") && k["meteor"].is_object()) {
                const json& m = k["meteor"];
                if (m.contains("spark_rate_scale") && m["spark_rate_scale"].is_number()) {
                    params.meteor.sparkRateScale = ClampFloat(m["spark_rate_scale"].get<float>(), 0.2f, 4.0f);
                    trailTouched = true;
                }
                if (m.contains("spark_speed_scale") && m["spark_speed_scale"].is_number()) {
                    params.meteor.sparkSpeedScale = ClampFloat(m["spark_speed_scale"].get<float>(), 0.2f, 4.0f);
                    trailTouched = true;
                }
            }
            if (k.contains("idle_fade_start_ms") && k["idle_fade_start_ms"].is_number_integer()) {
                params.idleFade.startMs = ClampInt(k["idle_fade_start_ms"].get<int>(), 0, 3000);
                trailTouched = true;
            }
            if (k.contains("idle_fade_end_ms") && k["idle_fade_end_ms"].is_number_integer()) {
                params.idleFade.endMs = ClampInt(k["idle_fade_end_ms"].get<int>(), 0, 6000);
                trailTouched = true;
            }
        }

        if (trailTouched) {
            SetTrailTuning(style, profiles, params);
        }

        // Theme last (recreates themed effects).
        if (p.contains("theme") && p["theme"].is_string()) {
            SetTheme(p["theme"].get<std::string>());
        }
    }
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
        return self->OnDispatchMessage(hwnd, msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT AppController::OnDispatchMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    // Reset idle timer on any mouse input
    if (msg == WM_MFX_CLICK || msg == WM_MFX_MOVE || msg == WM_MFX_SCROLL || 
        msg == WM_MFX_BUTTON_DOWN || msg == WM_MFX_BUTTON_UP) 
    {
        lastInputTime_ = GetTickCount64();
        if (hovering_) {
            hovering_ = false;
            if (auto* effect = GetEffect(EffectCategory::Hover)) {
                effect->OnHoverEnd();
            }
        }
    }

    if (msg == WM_MFX_CLICK) {
        if (ignoreNextClick_) {
            ignoreNextClick_ = false;
            return 0; // Suppress click after a long hold
        }

        auto* ev = reinterpret_cast<ClickEvent*>(lParam);
        if (ev) {
#ifdef _DEBUG
            if (debugClickCount_ < 5) {
                debugClickCount_++;
                wchar_t buf[256]{};
                wsprintfW(buf, L"MouseFx: click received (%u) pt=(%ld,%ld) button=%u\n",
                    debugClickCount_, ev->pt.x, ev->pt.y, (unsigned)ev->button);
                OutputDebugStringW(buf);
            }
#endif
            // Dispatch to Click category effect
            if (auto* effect = GetEffect(EffectCategory::Click)) {
                effect->OnClick(*ev);
            }
            delete ev;
            OverlayHostService::Instance().RequestImmediateFrame();
        }
        return 0;
    } 
    
    if (msg == WM_MFX_MOVE) {
        POINT pt{};
        if (!hook_.ConsumeLatestMove(pt)) {
            pt.x = static_cast<LONG>(wParam);
            pt.y = static_cast<LONG>(lParam);
        }
        lastMovePt_ = pt;
        hasLastMovePt_ = true;

        const uint64_t nowTick = GetTickCount64();
        const bool dragLikeMove = holdButtonDown_ && IsLikelySystemWindowDrag(hwnd);
        const bool allowMoveDispatch = !dragLikeMove || ShouldDispatchDragMove(nowTick);
        const bool holdLatencyPriority = ShouldPrioritizeHoldLatency();
        SetTrailLatencyPriorityMode(holdLatencyPriority);
        const bool allowTrailDispatch = allowMoveDispatch;

        // Dispatch to Trail category effect
        if (allowTrailDispatch) {
            if (auto* effect = GetEffect(EffectCategory::Trail)) {
                effect->OnMouseMove(pt);
            }
        }
        // Dispatch to Hold category effect (to update position if following mouse).
        // During drag-like move, both trail and hold follow the same move gating.
        if (allowMoveDispatch) {
            if (auto* effect = GetEffect(EffectCategory::Hold)) {
                DWORD holdMs = 0;
                if (holdButtonDown_ && holdDownTick_ != 0) {
                    const uint64_t delta = (nowTick >= holdDownTick_) ? (nowTick - holdDownTick_) : 0;
                    holdMs = (DWORD)std::min<uint64_t>(delta, 0xFFFFFFFFu);
                }
                effect->OnHoldUpdate(pt, holdMs);
            }
        } else if (holdButtonDown_) {
            // Consume suppressed move quietly during drag throttling.
        }
        OverlayHostService::Instance().RequestImmediateFrame();
        return 0;
    }

    if (msg == WM_MFX_SCROLL) {
        short delta = static_cast<short>(wParam);
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        // Dispatch to Scroll category effect
        if (auto* effect = GetEffect(EffectCategory::Scroll)) {
            ScrollEvent ev{};
            ev.pt = pt;
            ev.delta = delta;
            ev.horizontal = false;
            effect->OnScroll(ev);
        }
        OverlayHostService::Instance().RequestImmediateFrame();
        return 0;
    }

    if (msg == WM_MFX_BUTTON_DOWN) {
        int button = static_cast<int>(wParam);
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);

        holdButtonDown_ = true;
        holdEffectRunning_ = false;
        holdDownTick_ = GetTickCount64();
        
        // Start delayed hold
        pendingHold_.pt = pt;
        pendingHold_.button = button;
        pendingHold_.active = true;
        ignoreNextClick_ = false; // Reset for new interaction
        SetTimer(hwnd, kHoldTimerId, kHoldDelayMs, nullptr);
        
        return 0;
    }

    if (msg == WM_MFX_BUTTON_UP) {
        holdButtonDown_ = false;
        holdEffectRunning_ = false;
        holdDownTick_ = 0;
        SetTrailLatencyPriorityMode(false);

        // Cancel pending hold if quick click
        if (pendingHold_.active) {
            KillTimer(hwnd, kHoldTimerId);
            pendingHold_.active = false;
        }

        // End hold effect if it was started
        if (auto* effect = GetEffect(EffectCategory::Hold)) {
            effect->OnHoldEnd();
        }
        OverlayHostService::Instance().RequestImmediateFrame();
        return 0;
    }

    if (msg == WM_TIMER) {
        if (wParam == kDeferredBackendTimerId) {
            KillTimer(hwnd, kDeferredBackendTimerId);
            if (deferredBackendApplyPending_) {
                deferredBackendApplyPending_ = false;
                OverlayHostService::Instance().SetGpuBridgeModeRequest(config_.gpuBridgeModeRequest);
                const bool shouldRecreateNow =
                    OverlayHostService::Instance().SetRenderBackendPreference(config_.renderBackend);
                if (config_.renderBackend != "cpu") {
                    if (shouldRecreateNow) {
                        if (IsBackendSwitchIdleWindow()) {
                            RecreateActiveEffects();
                        } else {
                            deferredDawnUpgradePending_ = true;
                            deferredDawnUpgradeRetryCount_ = 0;
                            SetTimer(hwnd, kDeferredBackendTimerId, 80, nullptr);
                        }
                    }
                    if (!OverlayHostService::Instance().IsDawnQueueReady()) {
                        deferredDawnUpgradePending_ = true;
                        deferredDawnUpgradeRetryCount_ = 0;
                        SetTimer(hwnd, kDeferredBackendTimerId, 200, nullptr);
                    } else {
                        deferredDawnUpgradePending_ = false;
                        deferredDawnUpgradeRetryCount_ = 0;
                    }
                } else {
                    deferredDawnUpgradePending_ = false;
                    deferredDawnUpgradeRetryCount_ = 0;
                }
                return 0;
            }
            if (deferredDawnUpgradePending_) {
                if (OverlayHostService::Instance().IsDawnQueueReady()) {
                    (void)OverlayHostService::Instance().SetRenderBackendPreference(config_.renderBackend);
                    deferredDawnUpgradePending_ = false;
                    deferredDawnUpgradeRetryCount_ = 0;
                } else {
                    ++deferredDawnUpgradeRetryCount_;
                    if (deferredDawnUpgradeRetryCount_ < 50) {
                        SetTimer(hwnd, kDeferredBackendTimerId, 200, nullptr);
                    } else {
                        deferredDawnUpgradePending_ = false;
                        deferredDawnUpgradeRetryCount_ = 0;
                    }
                }
                return 0;
            }
            return 0;
        }
        if (wParam == kHoverTimerId) {
            if (!hovering_) {
                uint64_t elapsed = GetTickCount64() - lastInputTime_;
                if (elapsed >= kHoverThresholdMs) {
                    hovering_ = true;
                    if (auto* effect = GetEffect(EffectCategory::Hover)) {
                        POINT pt;
                        GetCursorPos(&pt);
                        effect->OnHoverStart(pt);
                    }
                }
            }
            return 0;
        }
        
        if (wParam == kHoldTimerId) {
            KillTimer(hwnd, kHoldTimerId);
            if (pendingHold_.active) {
                if (auto* trailEffect = GetEffect(EffectCategory::Trail)) {
                    // Seed trail path when hold is promoted, reducing early hold-only frames.
                    trailEffect->OnMouseMove(pendingHold_.pt);
                    bool seededSecondPoint = false;
                    if (hasLastMovePt_ &&
                        (lastMovePt_.x != pendingHold_.pt.x || lastMovePt_.y != pendingHold_.pt.y)) {
                        trailEffect->OnMouseMove(lastMovePt_);
                        seededSecondPoint = true;
                    }
                    POINT nowPt{};
                    if (!seededSecondPoint &&
                        GetCursorPos(&nowPt) &&
                        (nowPt.x != pendingHold_.pt.x || nowPt.y != pendingHold_.pt.y)) {
                        trailEffect->OnMouseMove(nowPt);
                    }
                }
                // Timer elapsed, this is a real hold: trigger effect
                if (auto* effect = GetEffect(EffectCategory::Hold)) {
                    effect->OnHoldStart(pendingHold_.pt, pendingHold_.button);
                    holdEffectRunning_ = true;
                }
                OverlayHostService::Instance().RequestImmediateFrame();
                // Keep active true so we know we triggered it (moved/up logic might use it)
                // But actually once triggered, the Effect manages state independently.
                // We just mark it inactive here to avoid double Trigger? No, logic is fine.
                // Actually if we keep active=true, UP handler knows we just finished a hold timing.
                // But UP checks 'active' to kill timer. If timer already dead, UP just calls OnHoldEnd.
                // So we set active=false to say "timer/pending phase is over".
                pendingHold_.active = false;
                ignoreNextClick_ = true; // Timer fired = Hold triggered = Ignore next click
            }
            return 0;
        }
#ifdef _DEBUG
        if (wParam == kSelfTestTimerId) {
            KillTimer(dispatchHwnd_, kSelfTestTimerId);
            ClickEvent ev{};
            GetCursorPos(&ev.pt);
            ev.button = MouseButton::Left;
            if (auto* effect = GetEffect(EffectCategory::Click)) {
                effect->OnClick(ev);
            }
            OutputDebugStringW(L"MouseFx: self-test ripple fired.\n");
            return 0;
        }
#endif
    }

    if (msg == WM_MFX_EXEC_CMD) {
        auto* cmdStr = reinterpret_cast<const std::string*>(lParam);
        if (cmdStr) {
            HandleCommand(*cmdStr);
        }
        return 0;
    }

    if (msg == WM_MFX_GET_CONFIG) {
        auto* out = reinterpret_cast<EffectConfig*>(lParam);
        if (out) {
            *out = config_;
        }
        return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace mousefx

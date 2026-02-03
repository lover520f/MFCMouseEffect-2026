// AppController.cpp

#include "pch.h"

#include "AppController.h"
#include "MouseFxMessages.h"
#include "ConfigPathResolver.h"
#include "EffectFactory.h"
#include "JsonLite.h"

#include <new>
#include <windowsx.h>  // For GET_X_LPARAM, GET_Y_LPARAM

namespace mousefx {

static const wchar_t* kDispatchClassName = L"MouseFxDispatchWindow";
static constexpr UINT_PTR kSelfTestTimerId = 0x4D46;

AppController::AppController() = default;

AppController::~AppController() {
    Stop();
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

    // Load config from the best available directory (AppData preferred)
    configDir_ = ResolveConfigDirectory();
    config_ = EffectConfig::Load(configDir_);

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

//#ifdef _DEBUG
//    SetTimer(dispatchHwnd_, kSelfTestTimerId, 250, nullptr);
//#endif
    return true;
}

void AppController::Stop() {
    hook_.Stop();
    for (auto& effect : effects_) {
        if (effect) {
            effect->Shutdown();
            effect.reset();
        }
    }
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
        }
        return 0;
    } 
    
    if (msg == WM_MFX_MOVE) {
        POINT pt;
        pt.x = static_cast<LONG>(wParam);
        pt.y = static_cast<LONG>(lParam);
        // Dispatch to Trail category effect
        if (auto* effect = GetEffect(EffectCategory::Trail)) {
            effect->OnMouseMove(pt);
        }
        // Dispatch to Hold category effect (to update position if following mouse)
        if (auto* effect = GetEffect(EffectCategory::Hold)) {
             // Pass 0 duration for now as valid delta tracking requires more state
            effect->OnHoldUpdate(pt, 0);
        }
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
        return 0;
    }

    if (msg == WM_MFX_BUTTON_DOWN) {
        int button = static_cast<int>(wParam);
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        
        // Start delayed hold
        pendingHold_.pt = pt;
        pendingHold_.button = button;
        pendingHold_.active = true;
        ignoreNextClick_ = false; // Reset for new interaction
        SetTimer(hwnd, kHoldTimerId, kHoldDelayMs, nullptr);
        
        return 0;
    }

    if (msg == WM_MFX_BUTTON_UP) {
        // Cancel pending hold if quick click
        if (pendingHold_.active) {
            KillTimer(hwnd, kHoldTimerId);
            pendingHold_.active = false;
        }

        // End hold effect if it was started
        if (auto* effect = GetEffect(EffectCategory::Hold)) {
            effect->OnHoldEnd();
        }
        return 0;
    }

    if (msg == WM_TIMER) {
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
                // Timer elapsed, this is a real hold: trigger effect
                if (auto* effect = GetEffect(EffectCategory::Hold)) {
                    effect->OnHoldStart(pendingHold_.pt, pendingHold_.button);
                }
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

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace mousefx

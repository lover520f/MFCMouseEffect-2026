// AppController.cpp

#include "pch.h"

#include "AppController.h"
#include "MouseFxMessages.h"
#include "RippleEffect.h"
#include "TrailEffect.h"
#include "IconEffect.h"
#include "ScrollEffect.h"
#include "HoldEffect.h"
#include "HoverEffect.h"

#include <new>
#include <windowsx.h>  // For GET_X_LPARAM, GET_Y_LPARAM



// Helper: simplistic JSON-like parsing for extracting string values.
static std::string ExtractJsonValue(const std::string& json, const std::string& key) {
	std::string search = "\"" + key + "\"";
	size_t keyPos = json.find(search);
	if (keyPos == std::string::npos) return "";

	size_t startQuote = json.find('"', keyPos + search.length());
	if (startQuote == std::string::npos) {
		startQuote = json.find('"', keyPos + search.length() + 1);
	}
	if (startQuote == std::string::npos) return "";

	size_t endQuote = json.find('"', startQuote + 1);
	if (endQuote == std::string::npos) return "";

	return json.substr(startQuote + 1, endQuote - startQuote - 1);
}

namespace mousefx {

static const wchar_t* kDispatchClassName = L"MouseFxDispatchWindow";
static constexpr UINT_PTR kSelfTestTimerId = 0x4D46;

AppController::AppController() = default;

AppController::~AppController() {
    Stop();
}

// Helper: Get exe directory for config loading
static std::wstring GetExeDirectory() {
    wchar_t path[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, path, MAX_PATH);
    std::wstring p(path);
    size_t pos = p.find_last_of(L"\\/");
    if (pos != std::wstring::npos) {
        return p.substr(0, pos);
    }
    return L".";
}

bool AppController::Start() {
    if (dispatchHwnd_) return true;
    diag_ = {};

    // Load config from EXE directory
    config_ = EffectConfig::Load(GetExeDirectory());

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
    SetEffect(EffectCategory::Click, config_.defaultEffect.empty() ? "ripple" : config_.defaultEffect);
    SetEffect(EffectCategory::Trail, "line");
    SetEffect(EffectCategory::Scroll, "arrow");
    SetEffect(EffectCategory::Hold, "charge");
    SetEffect(EffectCategory::Hover, "glow");

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

#ifdef _DEBUG
    SetTimer(dispatchHwnd_, kSelfTestTimerId, 250, nullptr);
#endif
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

std::unique_ptr<IMouseEffect> AppController::CreateEffect(EffectCategory category, const std::string& type) {
    if (type == "none" || type.empty()) {
        return nullptr;
    }

    switch (category) {
        case EffectCategory::Click:
            if (type == "ripple") return std::make_unique<RippleEffect>();
            if (type == "star")   return std::make_unique<IconEffect>();
            break;
        case EffectCategory::Trail:
            if (type == "line")   return std::make_unique<TrailEffect>();
            break;
        case EffectCategory::Scroll:
            if (type == "arrow")  return std::make_unique<ScrollEffect>();
            break;
        case EffectCategory::Hold:
            if (type == "charge") return std::make_unique<HoldEffect>();
            break;
        case EffectCategory::Hover:
            if (type == "glow")   return std::make_unique<HoverEffect>();
            break;
        case EffectCategory::Edge:
            // TODO: implement these categories
            break;
        default:

            break;
    }

    // Fallback for unknown type
#ifdef _DEBUG
    OutputDebugStringA(("MouseFx: unknown effect type: " + type + "\n").c_str());
#endif
    return nullptr;
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

IMouseEffect* AppController::GetEffect(EffectCategory category) const {
    size_t idx = static_cast<size_t>(category);
    if (idx >= kCategoryCount) return nullptr;
    return effects_[idx].get();
}

void AppController::HandleCommand(const std::string& jsonCmd) {
    std::string cmd = ExtractJsonValue(jsonCmd, "cmd");
    
    if (cmd == "set_effect") {
        std::string category = ExtractJsonValue(jsonCmd, "category");
        std::string type = ExtractJsonValue(jsonCmd, "type");
        
        if (category.empty()) {
            // Legacy format: {"cmd": "set_effect", "type": "ripple"}
            // Assume click category for backward compatibility
            SetEffect(EffectCategory::Click, type);
        } else {
            SetEffect(CategoryFromString(category), type);
        }
    } else if (cmd == "clear_effect") {
        std::string category = ExtractJsonValue(jsonCmd, "category");
        ClearEffect(CategoryFromString(category));
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
        // Dispatch to Hold category effect
        if (auto* effect = GetEffect(EffectCategory::Hold)) {
            effect->OnHoldStart(pt, button);
        }
        return 0;
    }

    if (msg == WM_MFX_BUTTON_UP) {
        // End hold effect
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

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace mousefx


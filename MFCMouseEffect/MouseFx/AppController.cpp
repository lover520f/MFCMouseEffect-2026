// AppController.cpp

#include "pch.h"

#include "AppController.h"
#include "MouseFxMessages.h"
#include "RippleEffect.h"
#include "TrailEffect.h"
#include "IconEffect.h"

#include <new>

// Helper: simplistic JSON-like parsing for "cmd" and "type".
// We assume simple format: {"cmd":"set_effect", "type":"ripple"} (with quotes).
static std::string ExtractJsonValue(const std::string& json, const std::string& key) {
	std::string search = "\"" + key + "\"";
	size_t keyPos = json.find(search);
	if (keyPos == std::string::npos) return "";

	size_t startQuote = json.find('"', keyPos + search.length());
	if (startQuote == std::string::npos) {
		// Maybe it's after a colon?
		startQuote = json.find('"', keyPos + search.length() + 1);
	}
	if (startQuote == std::string::npos) return "";

	size_t endQuote = json.find('"', startQuote + 1);
	if (endQuote == std::string::npos) return "";

	return json.substr(startQuote + 1, endQuote - startQuote - 1);
}

namespace mousefx {

static const wchar_t* kDispatchClassName = L"MouseFxDispatchWindow";
static constexpr UINT_PTR kSelfTestTimerId = 0x4D46; // 'MF' - debug-only

AppController::AppController() {
    // Default effect
    currentEffect_ = std::make_unique<RippleEffect>();
}

AppController::~AppController() {
    Stop();
}

bool AppController::Start() {
    if (dispatchHwnd_) return true;
    diag_ = {};

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

    diag_.stage = StartStage::EffectInit;
    if (currentEffect_ && !currentEffect_->Initialize()) {
#ifdef _DEBUG
        OutputDebugStringW(L"MouseFx: effect init failed.\n");
#endif
        // We allow starting even if effect init fails (it might be fixed later by switching effect).
        // But for diagnosing, we note it.
        diag_.error = GetLastError();
    }

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
    // One-shot self-test: shows a ripple at the current cursor position shortly after start.
    // This helps diagnose "no ripple" issues (hook vs rendering vs wrong exe).
    SetTimer(dispatchHwnd_, kSelfTestTimerId, 250, nullptr);
#endif
    return true;
}

void AppController::Stop() {
    hook_.Stop();
    if (currentEffect_) {
        currentEffect_->Shutdown();
    }
    DestroyDispatchWindow();
    gdiplus_.Shutdown();
}

void AppController::SetEffect(const std::string& type) {
    // 1. Shutdown current
    if (currentEffect_) {
        currentEffect_->Shutdown();
        currentEffect_.reset();
    }

    // 2. Create new
    if (type == "ripple") {
        currentEffect_ = std::make_unique<RippleEffect>();
    } else if (type == "trail") {
        currentEffect_ = std::make_unique<TrailEffect>();
    } else if (type == "icon_star") {
        currentEffect_ = std::make_unique<IconEffect>();
    } else if (type == "none") {
        currentEffect_ = nullptr; // no effect
    } else {
        // Fallback or log error
        currentEffect_ = std::make_unique<RippleEffect>();
    }

    // 3. Initialize new
    if (currentEffect_) {
        // Just in case we are already running (GlobalHook stage), we should Init immediately
        // BUT SetEffect might be called before Start().
        // If we are running (hook active), we must Init.
        // Simple check: if (hook_.IsActive()?) or just always try Init if dispatch window exists.
        // For now, simple approach:
        currentEffect_->Initialize();
    }
}

void AppController::HandleCommand(const std::string& jsonCmd) {
    // In real app, use a JSON library (e.g. nlohmann/json).
    // For now using manual extraction as per light-weight requirements.
    std::string cmd = ExtractJsonValue(jsonCmd, "cmd");
    
    if (cmd == "set_effect") {
        std::string type = ExtractJsonValue(jsonCmd, "type");
        SetEffect(type);
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
        0,
        kDispatchClassName,
        L"",
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        nullptr,
        GetModuleHandleW(nullptr),
        this
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
        // `CreateWindowExW` hasn't returned yet, so `dispatchHwnd_` isn't set.
        // Ensure we never call DefWindowProc with a null/invalid HWND during creation.
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
            if (currentEffect_) {
                currentEffect_->OnClick(*ev);
            }
            delete ev;
        }
        return 0;
    } else if (msg == mousefx::WM_MFX_MOVE) {
        if (currentEffect_) {
            POINT pt;
            pt.x = (LONG)wParam;
            pt.y = (LONG)lParam;
            currentEffect_->OnMouseMove(pt);
        }
        return 0;
    }
#ifdef _DEBUG
    if (msg == WM_TIMER && wParam == kSelfTestTimerId) {
        KillTimer(dispatchHwnd_, kSelfTestTimerId);
        ClickEvent ev{};
        GetCursorPos(&ev.pt);
        ev.button = MouseButton::Left;
        if (currentEffect_) currentEffect_->OnClick(ev);
        OutputDebugStringW(L"MouseFx: self-test ripple fired.\n");
        return 0;
    }
#endif
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace mousefx

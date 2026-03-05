#include "pch.h"
#include "TextWindow.h"
#include "MouseFx/Utils/TimeUtils.h"
#include "Platform/windows/Graphics/Win32D2DFactory.h"
#include "Platform/windows/Overlay/Win32OverlayTimerSupport.h"
#include "Settings/EmojiUtils.h"
#include <algorithm>
#include <cmath>
#include <dxgiformat.h>

namespace mousefx {

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")


static float GetWindowDpi(HWND hwnd) {
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32) {
        using GetDpiForWindowFn = UINT(WINAPI*)(HWND);
        auto* fn = reinterpret_cast<GetDpiForWindowFn>(GetProcAddress(user32, "GetDpiForWindow"));
        if (fn) return (float)fn(hwnd);
    }
    return 96.0f;
}

static std::wstring ResolveFontFamilyName(const TextEffectRenderCommand& command) {
    if (settings::IsEmojiOnlyText(command.text)) {
        return L"Segoe UI Emoji";
    }
    if (!command.fontFamily.empty()) return command.fontFamily;
    return L"Segoe UI";
}

TextWindow::~TextWindow() {
    if (hwnd_) {
        KillTimer(hwnd_, kTimerId);
        frameTimerArmed_ = false;
        DestroyWindow(hwnd_);
        hwnd_ = nullptr;
    }
    DestroySurface();
}

const wchar_t* TextWindow::ClassName() {
    return L"MouseFxTextWindow";
}

bool TextWindow::EnsureClassRegistered() {
    static bool registered = false;
    static bool ok = false;
    if (registered) return ok;
    registered = true;

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = &TextWindow::WndProc;
    wc.hInstance = GetModuleHandleW(nullptr);
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.lpszClassName = ClassName();
    ok = (RegisterClassExW(&wc) != 0) || (GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
    return ok;
}

bool TextWindow::Create() {
    if (hwnd_) return true;
    if (!EnsureClassRegistered()) return false;

    DWORD ex = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_NOACTIVATE;
    hwnd_ = CreateWindowExW(
        ex,
        ClassName(),
        L"",
        WS_POPUP,
        0, 0, 0, 0,
        nullptr,
        nullptr,
        GetModuleHandleW(nullptr),
        this
    );
    if (!hwnd_) return false;

    ShowWindow(hwnd_, SW_HIDE);
    return true;
}

void TextWindow::StartAtComputed(const ScreenPoint& anchorPoint, const TextEffectRenderCommand& command) {
    if (!hwnd_ && !Create()) return;

    command_ = command;
    const int winSize = std::max(1, static_cast<int>(std::lround(command_.panelSizePx)));

    EnsureSurface(winSize, winSize);

    const int left = anchorPoint.x - (winSize / 2);
    const int top = anchorPoint.y - (winSize / 2);
    baseLeft_ = left;
    baseTop_ = top;
    emojiColorMode_ = command_.emojiText;
    emojiFrameReady_ = false;

    SetWindowPos(hwnd_, HWND_TOPMOST, left, top, winSize, winSize, SWP_NOACTIVATE | SWP_SHOWWINDOW);

    startTick_ = NowMs();
    active_ = true;

    const TextEffectRenderFrame firstFrame = ComputeTextEffectRenderFrame(command_, 0.0);

    if (emojiColorMode_) {
        emojiFrameReady_ = RenderEmojiBaseFrame();
        if (emojiFrameReady_) {
            PresentEmojiCachedFrame(firstFrame);
        } else {
            RenderFrame(firstFrame);
        }
    } else {
        RenderFrame(firstFrame);
    }
    UpdateFrameTimerForPoint(anchorPoint, true);
}

LRESULT CALLBACK TextWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    TextWindow* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = reinterpret_cast<TextWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->hwnd_ = hwnd;
    } else {
        self = reinterpret_cast<TextWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self) return self->OnMessage(msg, wParam, lParam);
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT TextWindow::OnMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCHITTEST: return HTTRANSPARENT;
    case WM_TIMER:
        if (wParam == kTimerId) {
            OnTick();
            return 0;
        }
        break;
    case WM_DESTROY:
        KillTimer(hwnd_, kTimerId);
        frameTimerArmed_ = false;
        active_ = false;
        break;
    }
    return DefWindowProcW(hwnd_, msg, wParam, lParam);
}

void TextWindow::UpdateFrameTimerForPoint(const ScreenPoint& anchorPoint, bool force) {
    if (!hwnd_) {
        return;
    }
    int desiredIntervalMs = win32_overlay_timer_support::ResolveTimerIntervalMsForScreenPoint(
        anchorPoint.x,
        anchorPoint.y);
    desiredIntervalMs = std::clamp(desiredIntervalMs, 4, 1000);
    const UINT desiredTimerMs = static_cast<UINT>(desiredIntervalMs);
    if (!force && frameTimerArmed_ && frameTimerIntervalMs_ == desiredTimerMs) {
        return;
    }
    SetTimer(hwnd_, kTimerId, desiredTimerMs, nullptr);
    frameTimerIntervalMs_ = desiredTimerMs;
    frameTimerArmed_ = true;
}

void TextWindow::OnTick() {
    if (!active_) {
        ShowWindow(hwnd_, SW_HIDE);
        KillTimer(hwnd_, kTimerId);
        return;
    }

    const uint64_t elapsed = NowMs() - startTick_;
    const double t = static_cast<double>(elapsed) / static_cast<double>(std::max(command_.durationMs, 1));

    if (t >= 1.0) {
        active_ = false;
        ShowWindow(hwnd_, SW_HIDE);
        KillTimer(hwnd_, kTimerId);
        return;
    }

    const TextEffectRenderFrame frame = ComputeTextEffectRenderFrame(command_, t);

    if (emojiColorMode_ && emojiFrameReady_) {
        PresentEmojiCachedFrame(frame);
    } else {
        RenderFrame(frame);
    }
}

void TextWindow::EnsureSurface(int w, int h) {
    if (width_ == w && height_ == h && memDc_) return;
    DestroySurface();
    width_ = w; height_ = h;
    HDC screen = GetDC(nullptr);
    memDc_ = CreateCompatibleDC(screen);
    BITMAPINFO bmi{};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h; 
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    dib_ = CreateDIBSection(screen, &bmi, DIB_RGB_COLORS, &bits_, nullptr, 0);
    if (dib_) SelectObject(memDc_, dib_);
    ReleaseDC(nullptr, screen);
}

void TextWindow::DestroySurface() {
    if (dib_) DeleteObject(dib_);
    if (memDc_) DeleteDC(memDc_);
    dib_ = nullptr; memDc_ = nullptr; bits_ = nullptr;
    DestroyD2DResources();
}

bool TextWindow::EnsureD2DResources() {
    if (!memDc_) return false;
    ID2D1Factory* d2dFactory = SharedD2D1Factory();
    if (!d2dFactory) return false;
    if (!d2dTarget_) {
        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
            D2D1_RENDER_TARGET_TYPE_DEFAULT,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            0.0f, 0.0f,
            D2D1_RENDER_TARGET_USAGE_NONE,
            D2D1_FEATURE_LEVEL_DEFAULT);
        if (FAILED(d2dFactory->CreateDCRenderTarget(&props, d2dTarget_.GetAddressOf()))) {
            return false;
        }
        d2dTarget_->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
    }
    if (!d2dBrush_) {
        if (FAILED(d2dTarget_->CreateSolidColorBrush(D2D1::ColorF(1, 1, 1, 1), d2dBrush_.GetAddressOf()))) {
            return false;
        }
    }
    return true;
}

bool TextWindow::EnsureTextLayout(float dpi, float widthDip, float heightDip) {
    IDWriteFactory* dwriteFactory = SharedDWriteFactory();
    if (!dwriteFactory) return false;

    const float fontSize = static_cast<float>(std::max(6.0, command_.baseFontSizePx));
    const bool sizeChanged =
        std::abs(layoutDpi_ - dpi) > 0.01f ||
        std::abs(layoutWidthDip_ - widthDip) > 0.01f ||
        std::abs(layoutHeightDip_ - heightDip) > 0.01f;

    if (textLayout_ && textFormat_ && !sizeChanged) {
        return true;
    }

    textLayout_.Reset();
    textFormat_.Reset();

    const bool emojiOnly = settings::IsEmojiOnlyText(command_.text);
    const bool hasEmoji = command_.emojiText;
    const std::wstring fontName = ResolveFontFamilyName(command_);
    const std::wstring emojiFontName = L"Segoe UI Emoji";
    const DWRITE_FONT_WEIGHT baseWeight = emojiOnly ? DWRITE_FONT_WEIGHT_REGULAR : DWRITE_FONT_WEIGHT_BOLD;

    if (FAILED(dwriteFactory->CreateTextFormat(
        fontName.c_str(),
        nullptr,
        baseWeight,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"",
        textFormat_.GetAddressOf()))) {
        return false;
    }

    textFormat_->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    textFormat_->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    textFormat_->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

    if (FAILED(dwriteFactory->CreateTextLayout(
        command_.text.c_str(),
        (UINT32)command_.text.size(),
        textFormat_.Get(),
        (FLOAT)widthDip,
        (FLOAT)heightDip,
        textLayout_.GetAddressOf()))) {
        textFormat_.Reset();
        return false;
    }

    if (hasEmoji) {
        size_t pos = 0;
        while (pos < command_.text.size()) {
            size_t runStart = pos;
            size_t next = pos;
            uint32_t cp = settings::NextCodePointUtf16(command_.text, &next);
            if (cp == 0) break;
            pos = next;
            if (settings::IsEmojiCodePoint(cp)) {
                size_t runEnd = pos;
                while (runEnd < command_.text.size()) {
                    size_t probe = runEnd;
                    uint32_t cp2 = settings::NextCodePointUtf16(command_.text, &probe);
                    if (cp2 == 0) break;
                    if (!settings::IsEmojiComponent(cp2)) break;
                    runEnd = probe;
                }
                DWRITE_TEXT_RANGE range{ (UINT32)runStart, (UINT32)(runEnd - runStart) };
                textLayout_->SetFontFamilyName(emojiFontName.c_str(), range);
                textLayout_->SetFontWeight(DWRITE_FONT_WEIGHT_REGULAR, range);
                pos = runEnd;
            } else {
                size_t runEnd = pos;
                while (runEnd < command_.text.size()) {
                    size_t probe = runEnd;
                    uint32_t cp2 = settings::NextCodePointUtf16(command_.text, &probe);
                    if (cp2 == 0) break;
                    if (settings::IsEmojiCodePoint(cp2)) break;
                    runEnd = probe;
                }
                DWRITE_TEXT_RANGE range{ (UINT32)runStart, (UINT32)(runEnd - runStart) };
                textLayout_->SetFontFamilyName(fontName.c_str(), range);
                textLayout_->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, range);
                pos = runEnd;
            }
        }
    }

    layoutDpi_ = dpi;
    layoutWidthDip_ = widthDip;
    layoutHeightDip_ = heightDip;
    return true;
}

void TextWindow::DestroyD2DResources() {
    textLayout_.Reset();
    textFormat_.Reset();
    layoutDpi_ = 0.0f;
    layoutWidthDip_ = 0.0f;
    layoutHeightDip_ = 0.0f;
    d2dBrush_.Reset();
    d2dTarget_.Reset();
}

void TextWindow::RenderFrame(const TextEffectRenderFrame& frame) {
    if (!hwnd_ || !memDc_ || !bits_) return;

    ZeroMemory(bits_, (size_t)width_ * (size_t)height_ * 4);

    if (!EnsureD2DResources()) return;
    const float dpi = GetWindowDpi(hwnd_);
    const float pxToDip = 96.0f / dpi;
    const float widthDip = (float)width_ * pxToDip;
    const float heightDip = (float)height_ * pxToDip;
    d2dTarget_->SetDpi(dpi, dpi);
    const RECT rc = { 0, 0, width_, height_ };
    if (FAILED(d2dTarget_->BindDC(memDc_, &rc))) return;
    d2dTarget_->BeginDraw();
    d2dTarget_->Clear(D2D1::ColorF(0, 0, 0, 0));

    if (!EnsureTextLayout(dpi, widthDip, heightDip)) {
        d2dTarget_->EndDraw();
        return;
    }

    d2dBrush_->SetColor(D2D1::ColorF(
        (float)((command_.argb >> 16) & 0xFF) / 255.0f,
        (float)((command_.argb >> 8) & 0xFF) / 255.0f,
        (float)(command_.argb & 0xFF) / 255.0f,
        static_cast<float>(std::clamp(frame.alpha, 0.0, 1.0))));

    const float xPosDip = static_cast<float>(frame.offsetXPx) * pxToDip;
    const float yOffsetDip = static_cast<float>(frame.offsetYUpPx) * pxToDip;
    const float centerX = (widthDip / 2.0f) + xPosDip;
    const float centerY = (heightDip / 2.0f) - yOffsetDip;
    const float angle = static_cast<float>(frame.rotationDeg);
    const float scale = static_cast<float>(std::max(0.1, frame.scale));
    D2D1_MATRIX_3X2_F transform =
        D2D1::Matrix3x2F::Translation(centerX, centerY) *
        D2D1::Matrix3x2F::Rotation(angle) *
        D2D1::Matrix3x2F::Scale(D2D1::SizeF(scale, scale), D2D1::Point2F(centerX, centerY)) *
        D2D1::Matrix3x2F::Translation(-centerX, -centerY);
    d2dTarget_->SetTransform(transform);

    // Stability-first: color-font path can trigger heavy _com_error first-chance noise
    // on some systems/drivers under debugger.
    const D2D1_DRAW_TEXT_OPTIONS drawOptions = D2D1_DRAW_TEXT_OPTIONS_NONE;
    d2dTarget_->DrawTextLayout(
        D2D1::Point2F(xPosDip, -yOffsetDip),
        textLayout_.Get(),
        d2dBrush_.Get(),
        drawOptions);

    d2dTarget_->SetTransform(D2D1::Matrix3x2F::Identity());
    d2dTarget_->EndDraw();

    RECT r{};
    GetWindowRect(hwnd_, &r);
    PresentBackbuffer(r.left, r.top, 255);
}

bool TextWindow::RenderEmojiBaseFrame() {
    if (!hwnd_ || !memDc_ || !bits_) return false;

    ZeroMemory(bits_, (size_t)width_ * (size_t)height_ * 4);

    if (!EnsureD2DResources()) return false;
    const float dpi = GetWindowDpi(hwnd_);
    const float pxToDip = 96.0f / dpi;
    const float widthDip = (float)width_ * pxToDip;
    const float heightDip = (float)height_ * pxToDip;
    d2dTarget_->SetDpi(dpi, dpi);
    const RECT rc = { 0, 0, width_, height_ };
    if (FAILED(d2dTarget_->BindDC(memDc_, &rc))) return false;
    d2dTarget_->BeginDraw();
    d2dTarget_->Clear(D2D1::ColorF(0, 0, 0, 0));

    if (!EnsureTextLayout(dpi, widthDip, heightDip)) {
        d2dTarget_->EndDraw();
        return false;
    }

    d2dBrush_->SetColor(D2D1::ColorF(
        (float)((command_.argb >> 16) & 0xFF) / 255.0f,
        (float)((command_.argb >> 8) & 0xFF) / 255.0f,
        (float)(command_.argb & 0xFF) / 255.0f,
        1.0f));

    d2dTarget_->SetTransform(D2D1::Matrix3x2F::Identity());
    d2dTarget_->DrawTextLayout(
        D2D1::Point2F(0.0f, 0.0f),
        textLayout_.Get(),
        d2dBrush_.Get(),
        D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
    d2dTarget_->EndDraw();
    return true;
}

void TextWindow::PresentEmojiCachedFrame(const TextEffectRenderFrame& frame) {
    const int left = baseLeft_ + static_cast<int>(std::lround(frame.offsetXPx));
    const int top = baseTop_ - static_cast<int>(std::lround(frame.offsetYUpPx));
    SetWindowPos(hwnd_, HWND_TOPMOST, left, top, width_, height_, SWP_NOACTIVATE | SWP_NOSIZE | SWP_SHOWWINDOW);
    const double alpha = std::clamp(frame.alpha, 0.0, 1.0);
    PresentBackbuffer(left, top, static_cast<BYTE>(std::lround(alpha * 255.0)));
}

void TextWindow::PresentBackbuffer(int left, int top, BYTE alpha) {
    POINT ptSrc{ 0, 0 };
    SIZE sizeWnd{ width_, height_ };
    POINT ptDst{ left, top };
    BLENDFUNCTION bf{ AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA };
    UpdateLayeredWindow(hwnd_, nullptr, &ptDst, &sizeWnd, memDc_, &ptSrc, 0, &bf, ULW_ALPHA);
}

} // namespace mousefx

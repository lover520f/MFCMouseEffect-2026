#include "pch.h"
#include "TextWindow.h"
#include <algorithm>

namespace mousefx {

static uint64_t NowMs() {
    return GetTickCount64();
}

static Gdiplus::Color ToGdiPlus(Argb c, BYTE alpha) {
    return Gdiplus::Color(alpha, (BYTE)((c.value >> 16) & 0xFF), (BYTE)((c.value >> 8) & 0xFF), (BYTE)(c.value & 0xFF));
}

TextWindow::~TextWindow() {
    if (hwnd_) {
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

void TextWindow::StartAt(const POINT& pt, const std::wstring& text, Argb color, const TextConfig& config) {
    if (!hwnd_ && !Create()) return;

    startPt_ = pt;
    text_ = text;
    color_ = color;
    config_ = config;

    // Estimate window size (roughly 100x100 or based on text)
    // For simplicity, fixed size with center alignment
    int winSize = (int)(config.fontSize * 8); 
    if (winSize < 200) winSize = 200;

    EnsureSurface(winSize, winSize);

    const int left = pt.x - (winSize / 2);
    const int top = pt.y - (winSize / 2);

    SetWindowPos(hwnd_, HWND_TOPMOST, left, top, winSize, winSize, SWP_NOACTIVATE | SWP_SHOWWINDOW);

    startTick_ = NowMs();
    active_ = true;

    RenderFrame(0.0f);
    SetTimer(hwnd_, kTimerId, 16, nullptr);
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
        active_ = false;
        break;
    }
    return DefWindowProcW(hwnd_, msg, wParam, lParam);
}

void TextWindow::OnTick() {
    if (!active_) {
        ShowWindow(hwnd_, SW_HIDE);
        KillTimer(hwnd_, kTimerId);
        return;
    }

    uint64_t elapsed = NowMs() - startTick_;
    float t = (float)elapsed / (float)config_.durationMs;

    if (t >= 1.0f) {
        active_ = false;
        ShowWindow(hwnd_, SW_HIDE);
        KillTimer(hwnd_, kTimerId);
        return;
    }

    RenderFrame(t);
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
}

void TextWindow::RenderFrame(float t) {
    if (!hwnd_ || !memDc_ || !bits_) return;

    ZeroMemory(bits_, (size_t)width_ * (size_t)height_ * 4);

    Gdiplus::Bitmap bmp(width_, height_, width_ * 4, PixelFormat32bppPARGB, static_cast<BYTE*>(bits_));
    Gdiplus::Graphics g(&bmp);
    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

    // Float UP: 0 at t=0, floatDistance at t=1
    float yOffset = t * config_.floatDistance;
    float alpha = 1.0f - t;
    if (t > 0.5f) {
        // Faster fade in second half? Or just linear.
        // Let's stick to linear for now.
    }

    Gdiplus::FontFamily fontFamily(config_.fontFamily.c_str());
    Gdiplus::Font font(&fontFamily, config_.fontSize, Gdiplus::FontStyleBold);
    Gdiplus::SolidBrush brush(ToGdiPlus(color_, (BYTE)(alpha * 255)));

    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
    format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

    // Center in window, then subtract yOffset
    Gdiplus::RectF rect(0.0f, -yOffset, (float)width_, (float)height_);
    g.DrawString(text_.c_str(), -1, &font, rect, &format, &brush);

    // Push to screen
    POINT ptSrc{ 0, 0 };
    SIZE sizeWnd{ width_, height_ };
    RECT r{};
    GetWindowRect(hwnd_, &r);
    POINT ptDst{ r.left, r.top };
    BLENDFUNCTION bf{ AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    UpdateLayeredWindow(hwnd_, nullptr, &ptDst, &sizeWnd, memDc_, &ptSrc, 0, &bf, ULW_ALPHA);
}

} // namespace mousefx

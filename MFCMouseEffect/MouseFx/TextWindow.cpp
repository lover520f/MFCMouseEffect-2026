#include "pch.h"
#include "TextWindow.h"
#include <algorithm>
#include <cmath>
#include <memory>

namespace mousefx {

static uint64_t NowMs() {
    return GetTickCount64();
}

static Gdiplus::Color ToGdiPlus(Argb c, BYTE alpha) {
    return Gdiplus::Color(alpha, (BYTE)((c.value >> 16) & 0xFF), (BYTE)((c.value >> 8) & 0xFF), (BYTE)(c.value & 0xFF));
}

static uint32_t NextCodePoint(const std::wstring& text, size_t* i) {
    if (!i || *i >= text.size()) return 0;
    wchar_t lead = text[*i];
    (*i)++;
    if (lead >= 0xD800 && lead <= 0xDBFF) {
        if (*i < text.size()) {
            wchar_t trail = text[*i];
            if (trail >= 0xDC00 && trail <= 0xDFFF) {
                (*i)++;
                return (((uint32_t)lead - 0xD800) << 10) + ((uint32_t)trail - 0xDC00) + 0x10000;
            }
        }
    }
    return (uint32_t)lead;
}

static bool IsEmojiCodePoint(uint32_t cp) {
    if (cp >= 0x1F300 && cp <= 0x1F5FF) return true; // Misc Symbols & Pictographs
    if (cp >= 0x1F600 && cp <= 0x1F64F) return true; // Emoticons
    if (cp >= 0x1F680 && cp <= 0x1F6FF) return true; // Transport & Map
    if (cp >= 0x1F700 && cp <= 0x1F77F) return true; // Alchemical Symbols
    if (cp >= 0x1F900 && cp <= 0x1F9FF) return true; // Supplemental Symbols & Pictographs
    if (cp >= 0x1FA70 && cp <= 0x1FAFF) return true; // Symbols & Pictographs Extended-A
    if (cp >= 0x2600 && cp <= 0x27BF) return true;   // Misc symbols
    if (cp >= 0x1F1E6 && cp <= 0x1F1FF) return true; // Flags
    return false;
}

static bool IsEmojiComponent(uint32_t cp) {
    if (IsEmojiCodePoint(cp)) return true;
    if (cp == 0xFE0F || cp == 0xFE0E || cp == 0x200D) return true;
    if (cp >= 0x1F3FB && cp <= 0x1F3FF) return true;
    return false;
}

static bool IsEmojiOnlyText(const std::wstring& text) {
    bool hasEmoji = false;
    for (size_t i = 0; i < text.size(); ) {
        uint32_t cp = NextCodePoint(text, &i);
        if (cp == 0) break;
        if (cp == 0xFE0F || cp == 0xFE0E || cp == 0x200D) continue; // VS16/VS15/ZWJ
        if (cp >= 0x1F3FB && cp <= 0x1F3FF) continue; // skin tone modifiers
        if (IsEmojiCodePoint(cp)) {
            hasEmoji = true;
            continue;
        }
        return false;
    }
    return hasEmoji;
}

static bool HasEmojiStarter(const std::wstring& text) {
    for (size_t i = 0; i < text.size(); ) {
        uint32_t cp = NextCodePoint(text, &i);
        if (cp == 0) break;
        if (IsEmojiCodePoint(cp)) return true;
    }
    return false;
}

static std::wstring ResolveFontFamilyName(const TextConfig& config, const std::wstring& text) {
    if (IsEmojiOnlyText(text)) {
        return L"Segoe UI Emoji";
    }
    if (!config.fontFamily.empty()) return config.fontFamily;
    return L"Segoe UI";
}

static std::unique_ptr<Gdiplus::FontFamily> CreateAvailableFamily(const std::wstring& name) {
    auto family = std::make_unique<Gdiplus::FontFamily>(name.c_str());
    if (family->IsAvailable()) return family;
    family = std::make_unique<Gdiplus::FontFamily>(L"Segoe UI");
    if (family->IsAvailable()) return family;
    return std::make_unique<Gdiplus::FontFamily>(L"Arial");
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

    // Randomize path characteristics
    driftX_ = (float)(rand() % 100 - 50); // Drift -50 to 50 pixels horizontally
    swayFreq_ = 1.0f + (float)(rand() % 200) / 100.0f; // 1.0 to 3.0 frequency
    swayAmp_ = 5.0f + (float)(rand() % 100) / 10.0f;   // 5.0 to 15.0 px amplitude

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

static float EaseOutCubic(float t) {
    float u = 1.0f - t;
    return 1.0f - (u * u * u);
}

void TextWindow::RenderFrame(float t) {
    if (!hwnd_ || !memDc_ || !bits_) return;

    ZeroMemory(bits_, (size_t)width_ * (size_t)height_ * 4);

    Gdiplus::Bitmap bmp(width_, height_, width_ * 4, PixelFormat32bppPARGB, static_cast<BYTE*>(bits_));
    Gdiplus::Graphics g(&bmp);
    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintAntiAlias);

    // Elegant movement: Non-linear path
    float eased = EaseOutCubic(t);
    float yOffset = eased * config_.floatDistance;
    
    // Combine drift and sway for a curved path
    float xPos = (t * driftX_) + std::sin(t * 3.14159f * swayFreq_) * swayAmp_;
    
    // Scale: pop up slightly at start
    float scale = 1.0f;
    if (t < 0.3f) scale = 0.8f + (t / 0.3f) * 0.4f;
    else scale = 1.2f - ((t - 0.3f) / 0.7f) * 0.2f;

    // Alpha
    float alpha = 1.0f;
    if (t < 0.15f) alpha = t / 0.15f; 
    else if (t > 0.6f) alpha = 1.0f - (t - 0.6f) / 0.4f;

    const bool emojiOnly = IsEmojiOnlyText(text_);
    const bool hasEmoji = HasEmojiStarter(text_);

    Gdiplus::SolidBrush brush(ToGdiPlus(color_, (BYTE)(alpha * 255)));

    // Center in window, apply curved path and a slight rotation
    float centerX = (float)width_ / 2.0f + xPos;
    float centerY = (float)height_ / 2.0f - yOffset;

    g.TranslateTransform(centerX, centerY);
    g.RotateTransform(xPos * 0.2f);
    g.TranslateTransform(-centerX, -centerY);

    if (!hasEmoji) {
        const std::wstring fontName = ResolveFontFamilyName(config_, text_);
        auto fontFamily = CreateAvailableFamily(fontName);
        const int fontStyle = emojiOnly ? Gdiplus::FontStyleRegular : Gdiplus::FontStyleBold;
        Gdiplus::Font font(fontFamily.get(), config_.fontSize * scale, fontStyle);

        Gdiplus::StringFormat format;
        format.SetAlignment(Gdiplus::StringAlignmentCenter);
        format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

        Gdiplus::RectF rect(xPos, -yOffset, (float)width_, (float)height_);
        g.DrawString(text_.c_str(), -1, &font, rect, &format, &brush);
    } else {
        const std::wstring normalName = ResolveFontFamilyName(config_, L"");
        auto normalFamily = CreateAvailableFamily(normalName);
        auto emojiFamily = CreateAvailableFamily(L"Segoe UI Emoji");
        Gdiplus::Font normalFont(normalFamily.get(), config_.fontSize * scale, Gdiplus::FontStyleBold);
        Gdiplus::Font emojiFont(emojiFamily.get(), config_.fontSize * scale, Gdiplus::FontStyleRegular);

        Gdiplus::StringFormat format(Gdiplus::StringFormat::GenericTypographic());
        format.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
        format.SetAlignment(Gdiplus::StringAlignmentNear);
        format.SetLineAlignment(Gdiplus::StringAlignmentCenter);

        struct Run {
            size_t start;
            size_t len;
            bool emoji;
        };
        std::vector<Run> runs;
        for (size_t i = 0; i < text_.size(); ) {
            size_t runStart = i;
            size_t next = i;
            uint32_t cp = NextCodePoint(text_, &next);
            if (cp == 0) break;
            i = next;
            if (IsEmojiCodePoint(cp)) {
                size_t runEnd = i;
                while (runEnd < text_.size()) {
                    size_t probe = runEnd;
                    uint32_t cp2 = NextCodePoint(text_, &probe);
                    if (cp2 == 0) break;
                    if (!IsEmojiComponent(cp2)) break;
                    runEnd = probe;
                }
                runs.push_back({ runStart, runEnd - runStart, true });
                i = runEnd;
            } else {
                size_t runEnd = i;
                while (runEnd < text_.size()) {
                    size_t probe = runEnd;
                    uint32_t cp2 = NextCodePoint(text_, &probe);
                    if (cp2 == 0) break;
                    if (IsEmojiCodePoint(cp2)) break;
                    runEnd = probe;
                }
                runs.push_back({ runStart, runEnd - runStart, false });
                i = runEnd;
            }
        }

        float totalW = 0.0f;
        float maxH = 0.0f;
        for (const auto& r : runs) {
            std::wstring s = text_.substr(r.start, r.len);
            Gdiplus::RectF bounds;
            Gdiplus::Font* f = r.emoji ? &emojiFont : &normalFont;
            g.MeasureString(s.c_str(), -1, f, Gdiplus::PointF(0.0f, 0.0f), &format, &bounds);
            totalW += bounds.Width;
            if (bounds.Height > maxH) maxH = bounds.Height;
        }
        float startX = centerX - (totalW / 2.0f);
        float y = centerY - (maxH / 2.0f);
        for (const auto& r : runs) {
            std::wstring s = text_.substr(r.start, r.len);
            Gdiplus::RectF bounds;
            Gdiplus::Font* f = r.emoji ? &emojiFont : &normalFont;
            g.MeasureString(s.c_str(), -1, f, Gdiplus::PointF(0.0f, 0.0f), &format, &bounds);
            Gdiplus::RectF rect(startX, y, bounds.Width, maxH);
            g.DrawString(s.c_str(), -1, f, rect, &format, &brush);
            startX += bounds.Width;
        }
    }

    g.ResetTransform();

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

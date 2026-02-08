#include "pch.h"
#include "EmojiPreviewWnd.h"

#include <dxgiformat.h>
#include <vector>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

namespace {

static float GetWindowDpi(HWND hwnd) {
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32) {
        using GetDpiForWindowFn = UINT(WINAPI*)(HWND);
        auto* fn = reinterpret_cast<GetDpiForWindowFn>(GetProcAddress(user32, "GetDpiForWindow"));
        if (fn) return (float)fn(hwnd);
    }
    return 96.0f;
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
    if (cp >= 0x1F300 && cp <= 0x1F5FF) return true;
    if (cp >= 0x1F600 && cp <= 0x1F64F) return true;
    if (cp >= 0x1F680 && cp <= 0x1F6FF) return true;
    if (cp >= 0x1F700 && cp <= 0x1F77F) return true;
    if (cp >= 0x1F900 && cp <= 0x1F9FF) return true;
    if (cp >= 0x1FA70 && cp <= 0x1FAFF) return true;
    if (cp >= 0x2600 && cp <= 0x27BF) return true;
    if (cp >= 0x1F1E6 && cp <= 0x1F1FF) return true;
    return false;
}

static bool IsEmojiComponent(uint32_t cp) {
    if (IsEmojiCodePoint(cp)) return true;
    if (cp == 0xFE0F || cp == 0xFE0E || cp == 0x200D) return true;
    if (cp >= 0x1F3FB && cp <= 0x1F3FF) return true;
    return false;
}

} // namespace

BEGIN_MESSAGE_MAP(EmojiPreviewWnd, CWnd)
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_NCHITTEST()
END_MESSAGE_MAP()

bool EmojiPreviewWnd::Create(CWnd* parent, const CRect& rc) {
    if (!parent || !::IsWindow(parent->GetSafeHwnd())) return false;
    const CString cls = AfxRegisterWndClass(0, LoadCursor(nullptr, IDC_IBEAM));
    DWORD style = WS_CHILD | WS_VISIBLE;
    DWORD ex = WS_EX_TRANSPARENT | WS_EX_NOACTIVATE;
    return CreateEx(ex, cls, L"", style, rc, parent, 0);
}

void EmojiPreviewWnd::SetText(const std::wstring& text) {
    if (text_ == text) return;
    text_ = text;
    Render();
}

void EmojiPreviewWnd::SetFontSizePx(float px) {
    if (px <= 1.0f) return;
    if (fontSizePx_ == px) return;
    fontSizePx_ = px;
    Render();
}

void EmojiPreviewWnd::SetTextColor(COLORREF color) {
    if (textColor_ == color) return;
    textColor_ = color;
    Render();
}

void EmojiPreviewWnd::SetPaddingPx(int left, int top) {
    padLeftPx_ = left;
    padTopPx_ = top;
    Render();
}

void EmojiPreviewWnd::SetTargetRect(const CRect& rc) {
    if (!::IsWindow(GetSafeHwnd())) return;
    SetWindowPos(&wndTop, rc.left, rc.top, rc.Width(), rc.Height(),
        SWP_NOACTIVATE | SWP_SHOWWINDOW);
    if (d2dTarget_) {
        d2dTarget_->Resize(D2D1::SizeU(rc.Width(), rc.Height()));
    }
    Invalidate(FALSE);
}

BOOL EmojiPreviewWnd::OnEraseBkgnd(CDC* /*pDC*/) {
    return TRUE;
}

LRESULT EmojiPreviewWnd::OnNcHitTest(CPoint /*point*/) {
    return HTTRANSPARENT;
}

void EmojiPreviewWnd::OnPaint() {
    CPaintDC dc(this);
    Render();
}

bool EmojiPreviewWnd::EnsureD2DResources() {
    if (!d2dFactory_) {
        if (FAILED(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2dFactory_.GetAddressOf()))) {
            return false;
        }
    }
    if (!dwriteFactory_) {
        if (FAILED(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(dwriteFactory_.GetAddressOf())))) {
            return false;
        }
    }
    if (!d2dTarget_) {
        CRect rc;
        GetClientRect(&rc);
        D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties();
        D2D1_HWND_RENDER_TARGET_PROPERTIES hwndProps =
            D2D1::HwndRenderTargetProperties(GetSafeHwnd(), D2D1::SizeU(rc.Width(), rc.Height()));
        if (FAILED(d2dFactory_->CreateHwndRenderTarget(props, hwndProps, d2dTarget_.GetAddressOf()))) {
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

void EmojiPreviewWnd::DestroyD2DResources() {
    d2dBrush_.Reset();
    d2dTarget_.Reset();
}

void EmojiPreviewWnd::Render() {
    if (!::IsWindow(GetSafeHwnd())) return;
    CRect rc;
    GetClientRect(&rc);
    if (rc.Width() <= 0 || rc.Height() <= 0) return;
    if (!EnsureD2DResources()) return;

    const float dpi = GetWindowDpi(GetSafeHwnd());
    const float pxToDip = 96.0f / dpi;
    const float widthDip = (float)rc.Width() * pxToDip;
    const float heightDip = (float)rc.Height() * pxToDip;
    d2dTarget_->SetDpi(dpi, dpi);

    d2dTarget_->BeginDraw();
    d2dTarget_->Clear(D2D1::ColorF(
        (float)GetRValue(::GetSysColor(COLOR_WINDOW)) / 255.0f,
        (float)GetGValue(::GetSysColor(COLOR_WINDOW)) / 255.0f,
        (float)GetBValue(::GetSysColor(COLOR_WINDOW)) / 255.0f,
        1.0f));

    const std::wstring baseFont = L"Segoe UI";
    const std::wstring emojiFont = L"Segoe UI Emoji";
    const float fontSize = fontSizePx_ * pxToDip;

    Microsoft::WRL::ComPtr<IDWriteTextFormat> format;
    if (SUCCEEDED(dwriteFactory_->CreateTextFormat(
        baseFont.c_str(),
        nullptr,
        DWRITE_FONT_WEIGHT_REGULAR,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        fontSize,
        L"",
        format.GetAddressOf()))) {
        format->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
        format->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
        format->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
    } else {
        d2dTarget_->EndDraw();
        return;
    }

    Microsoft::WRL::ComPtr<IDWriteTextLayout> layout;
    if (FAILED(dwriteFactory_->CreateTextLayout(
        text_.c_str(),
        (UINT32)text_.size(),
        format.Get(),
        (FLOAT)widthDip,
        (FLOAT)heightDip,
        layout.GetAddressOf()))) {
        d2dTarget_->EndDraw();
        return;
    }

    size_t pos = 0;
    while (pos < text_.size()) {
        size_t runStart = pos;
        size_t next = pos;
        uint32_t cp = NextCodePoint(text_, &next);
        if (cp == 0) break;
        pos = next;
        if (IsEmojiCodePoint(cp)) {
            size_t runEnd = pos;
            while (runEnd < text_.size()) {
                size_t probe = runEnd;
                uint32_t cp2 = NextCodePoint(text_, &probe);
                if (cp2 == 0) break;
                if (!IsEmojiComponent(cp2)) break;
                runEnd = probe;
            }
            DWRITE_TEXT_RANGE range{ (UINT32)runStart, (UINT32)(runEnd - runStart) };
            layout->SetFontFamilyName(emojiFont.c_str(), range);
            layout->SetFontWeight(DWRITE_FONT_WEIGHT_REGULAR, range);
            pos = runEnd;
        } else {
            size_t runEnd = pos;
            while (runEnd < text_.size()) {
                size_t probe = runEnd;
                uint32_t cp2 = NextCodePoint(text_, &probe);
                if (cp2 == 0) break;
                if (IsEmojiCodePoint(cp2)) break;
                runEnd = probe;
            }
            DWRITE_TEXT_RANGE range{ (UINT32)runStart, (UINT32)(runEnd - runStart) };
            layout->SetFontFamilyName(baseFont.c_str(), range);
            layout->SetFontWeight(DWRITE_FONT_WEIGHT_REGULAR, range);
            pos = runEnd;
        }
    }

    d2dBrush_->SetColor(D2D1::ColorF(
        (float)GetRValue(textColor_) / 255.0f,
        (float)GetGValue(textColor_) / 255.0f,
        (float)GetBValue(textColor_) / 255.0f,
        1.0f));

    const float x = padLeftPx_ * pxToDip;
    const float y = padTopPx_ * pxToDip;

    // Stability-first: avoid color-font path here as well, otherwise VS debugger
    // can spam first-chance _com_error while settings preview repaints.
    d2dTarget_->DrawTextLayout(
        D2D1::Point2F(x, y),
        layout.Get(),
        d2dBrush_.Get(),
        D2D1_DRAW_TEXT_OPTIONS_NONE);

    d2dTarget_->EndDraw();
}

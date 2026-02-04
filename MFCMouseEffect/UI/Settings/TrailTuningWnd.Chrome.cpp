#include "pch.h"
#include "UI/Settings/TrailTuningWnd.h"

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

namespace {

static int ClampIntLocal(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

} // namespace

void CTrailTuningWnd::OnPaint() {
    CPaintDC dc(this);
    CRect rc = Client();

    CDC mem;
    mem.CreateCompatibleDC(&dc);
    CBitmap bmp;
    bmp.CreateCompatibleBitmap(&dc, rc.Width(), rc.Height());
    auto* oldBmp = mem.SelectObject(&bmp);

    Gdiplus::Graphics g(mem);
    g.SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    g.SetTextRenderingHint(Gdiplus::TextRenderingHintClearTypeGridFit);

    const COLORREF sysWindow = ::GetSysColor(COLOR_WINDOW);
    const COLORREF sysText = ::GetSysColor(COLOR_WINDOWTEXT);
    const COLORREF sysMuted = ::GetSysColor(COLOR_GRAYTEXT);
    const COLORREF sysFace = ::GetSysColor(COLOR_BTNFACE);
    g.Clear(Gdiplus::Color(255, GetRValue(sysWindow), GetGValue(sysWindow), GetBValue(sysWindow)));

    // Header (match SettingsWnd chrome)
    {
        CRect h = RcHeader();
        DrawRoundRect(g, CRect(h.left + S(8), h.top + S(8), h.right - S(8), h.bottom), S(14),
            Gdiplus::Color(255, GetRValue(sysFace), GetGValue(sysFace), GetBValue(sysFace)));

        CRect title = h;
        title.left += S(18);
        title.top += S(8);
        title.bottom = title.top + S(34);
        const wchar_t* titleText = IsZh() ? L"\u62D6\u5C3E\u9AD8\u7EA7\u8C03\u53C2" : L"Trail Tuning";
        DrawText(g, titleText, title, S(16), true, Gdiplus::Color(255, GetRValue(sysText), GetGValue(sysText), GetBValue(sysText)), Gdiplus::StringAlignmentNear);

        CRect sub = h;
        sub.left += S(18);
        sub.top += S(44);
        sub.bottom = h.bottom;
        const wchar_t* subText = IsZh()
            ? L"\u8C03\u6574\u62D6\u5C3E\u957F\u5EA6/\u5BC6\u5EA6\u4EE5\u53CA\u53D1\u5149/\u5206\u53C9/\u706B\u82B1"
            : L"Tune tail length/density and glow/forks/sparks";
        DrawText(g, subText, sub, S(11), false, Gdiplus::Color(255, GetRValue(sysMuted), GetGValue(sysMuted), GetBValue(sysMuted)), Gdiplus::StringAlignmentNear);

        CRect x = RcCloseBtn();
        const bool hov = (hover_ == Hit::Close);
        DrawRoundRect(g, x, S(10), hov ? Gdiplus::Color(255, 210, 215, 223) : Gdiplus::Color(255, 228, 233, 240));
        DrawText(g, L"\u00D7", x, S(16), true, Gdiplus::Color(255, GetRValue(sysText), GetGValue(sysText), GetBValue(sysText)));
    }

    dc.BitBlt(0, 0, rc.Width(), rc.Height(), &mem, 0, 0, SRCCOPY);
    mem.SelectObject(oldBmp);
}

BOOL CTrailTuningWnd::OnEraseBkgnd(CDC* /*pDC*/) {
    return TRUE;
}

void CTrailTuningWnd::OnLButtonDown(UINT nFlags, CPoint point) {
    CWnd::OnLButtonDown(nFlags, point);
    down_ = HitTest(point).kind;
}

void CTrailTuningWnd::OnLButtonUp(UINT nFlags, CPoint point) {
    CWnd::OnLButtonUp(nFlags, point);
    const Hit h = HitTest(point);
    if (h.kind == Hit::Close && down_ == Hit::Close) {
        PostMessage(WM_CLOSE);
    }
    down_ = Hit::None;
}

void CTrailTuningWnd::OnMouseMove(UINT nFlags, CPoint point) {
    CWnd::OnMouseMove(nFlags, point);
    const Hit h = HitTest(point);
    if (hover_ != h.kind) {
        hover_ = h.kind;
        Invalidate(FALSE);
    }
}

LRESULT CTrailTuningWnd::OnNcHitTest(CPoint point) {
    CPoint client = point;
    ScreenToClient(&client);
    if (RcCloseBtn().PtInRect(client)) return HTCLIENT;
    if (RcHeader().PtInRect(client)) return HTCAPTION;
    return CWnd::OnNcHitTest(point);
}

int CTrailTuningWnd::Dpi() const {
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32) {
        using GetDpiForWindowFn = UINT(WINAPI*)(HWND);
        auto* fn = (GetDpiForWindowFn)GetProcAddress(user32, "GetDpiForWindow");
        if (fn) return (int)fn(GetSafeHwnd());
    }
    return 96;
}

int CTrailTuningWnd::S(int px) const {
    return (px * Dpi() + 48) / 96;
}

CRect CTrailTuningWnd::Client() const {
    CRect rc;
    GetClientRect(&rc);
    return rc;
}

CRect CTrailTuningWnd::RcHeader() const {
    CRect rc = Client();
    rc.bottom = S(64);
    return rc;
}

CRect CTrailTuningWnd::RcCloseBtn() const {
    CRect rc = RcHeader();
    const int sz = S(28);
    rc.left = rc.right - S(16) - sz;
    rc.top = rc.top + S(12);
    rc.right = rc.left + sz;
    rc.bottom = rc.top + sz;
    return rc;
}

CRect CTrailTuningWnd::RcContent() const {
    CRect rc = Client();
    rc.DeflateRect(S(16), S(72), S(16), S(72));
    return rc;
}

CRect CTrailTuningWnd::RcFooter() const {
    CRect rc = Client();
    rc.top = rc.bottom - S(56);
    return rc;
}

CTrailTuningWnd::Hit CTrailTuningWnd::HitTest(const CPoint& pt) const {
    Hit h;
    const CRect close = RcCloseBtn();
    if (close.PtInRect(pt)) {
        h.kind = Hit::Close;
        h.rc = close;
        return h;
    }
    return h;
}

void CTrailTuningWnd::DrawRoundRect(Gdiplus::Graphics& g, const CRect& rc, int radius, const Gdiplus::Color& fill) {
    const int r = ClampIntLocal(radius, 0, 32);
    Gdiplus::GraphicsPath path;
    const int d = r * 2;

    path.AddArc((float)rc.left, (float)rc.top, (float)d, (float)d, 180.0f, 90.0f);
    path.AddArc((float)rc.right - d, (float)rc.top, (float)d, (float)d, 270.0f, 90.0f);
    path.AddArc((float)rc.right - d, (float)rc.bottom - d, (float)d, (float)d, 0.0f, 90.0f);
    path.AddArc((float)rc.left, (float)rc.bottom - d, (float)d, (float)d, 90.0f, 90.0f);
    path.CloseFigure();

    Gdiplus::SolidBrush b(fill);
    g.FillPath(&b, &path);
}

void CTrailTuningWnd::DrawText(Gdiplus::Graphics& g, const wchar_t* text, const CRect& rc, int sizePx, bool bold, const Gdiplus::Color& c, Gdiplus::StringAlignment align) {
    if (!text) return;
    Gdiplus::FontFamily ff(L"Segoe UI");
    const int style = bold ? Gdiplus::FontStyleBold : Gdiplus::FontStyleRegular;
    Gdiplus::Font font(&ff, (Gdiplus::REAL)sizePx, style, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush b(c);

    Gdiplus::RectF r((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.top, (Gdiplus::REAL)rc.Width(), (Gdiplus::REAL)rc.Height());
    Gdiplus::StringFormat fmt;
    fmt.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
    fmt.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
    fmt.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    fmt.SetAlignment(align);
    g.DrawString(text, -1, &font, r, &fmt, &b);
}

bool CTrailTuningWnd::IsZh() const {
    return uiLanguage_.empty() || uiLanguage_ == "zh-CN";
}


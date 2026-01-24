#include "pch.h"
#include "SettingsWnd.h"

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#include "MFCMouseEffect.h"
#include "Settings/SettingsBackend.h"
#include "Settings/SettingsOptions.h"

namespace {

static Gdiplus::Color C(BYTE a, BYTE r, BYTE g, BYTE b) {
    return Gdiplus::Color(a, r, g, b);
}

static int ClampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

static const SettingOption* FindByValue(const SettingOption* opts, size_t n, const std::string& value) {
    for (size_t i = 0; i < n; ++i) {
        if (value == opts[i].value) return &opts[i];
    }
    return nullptr;
}

static std::string GetValueFromItemData(DWORD_PTR data) {
    const char* v = reinterpret_cast<const char*>(data);
    return v ? std::string(v) : std::string();
}

} // namespace

BEGIN_MESSAGE_MAP(CSettingsWnd, CWnd)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_NCHITTEST()
    ON_WM_CLOSE()
    ON_WM_DESTROY()
    ON_BN_CLICKED(10001, &CSettingsWnd::OnCommandApply)
    ON_BN_CLICKED(10002, &CSettingsWnd::OnCommandClose)
    ON_CBN_SELCHANGE(11000, &CSettingsWnd::OnSelChange)
    ON_CBN_SELCHANGE(11001, &CSettingsWnd::OnSelChange)
    ON_CBN_SELCHANGE(11002, &CSettingsWnd::OnSelChange)
    ON_CBN_SELCHANGE(11003, &CSettingsWnd::OnSelChange)
    ON_CBN_SELCHANGE(11004, &CSettingsWnd::OnSelChange)
    ON_CBN_SELCHANGE(11005, &CSettingsWnd::OnSelChange)
    ON_CBN_SELCHANGE(11006, &CSettingsWnd::OnSelChange)
END_MESSAGE_MAP()

bool CSettingsWnd::CreateAndShow(CWnd* parent, std::unique_ptr<ISettingsBackend> backend) {
    backend_ = std::move(backend);
    if (!backend_) return false;

    const CString cls = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LoadCursor(nullptr, IDC_ARROW));

    // Custom frame only; internal controls use native styles.
    DWORD style = WS_POPUP | WS_VISIBLE;
    DWORD ex = WS_EX_APPWINDOW | WS_EX_COMPOSITED;

    CRect rc(0, 0, 560, 520);
    if (parent && ::IsWindow(parent->GetSafeHwnd())) {
        parent->GetWindowRect(&rc);
        rc.OffsetRect(40, 40);
        rc.right = rc.left + 560;
        rc.bottom = rc.top + 520;
    } else {
        rc = CRect(200, 160, 760, 680);
    }

    if (!CreateEx(ex, cls, L"", style, rc, parent, 0)) {
        return false;
    }

    ShowWindow(SW_SHOW);
    UpdateWindow();
    return true;
}

int CSettingsWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CWnd::OnCreate(lpCreateStruct) == -1) return -1;

    model_ = backend_->Load();
    if (model_.uiLanguage.empty()) model_.uiLanguage = "zh-CN";

    NONCLIENTMETRICS ncm{};
    ncm.cbSize = sizeof(ncm);
    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0)) {
        font_.CreateFontIndirect(&ncm.lfMessageFont);
    }

    const CRect content = RcContent();
    const int pad = S(14);
    const int top = content.top;
    const int labelW = S(120);
    const int boxW = content.Width() - labelW - pad * 3;
    const int sectionGap = S(10);
    const int availableH = content.Height();
    const int rowH = ClampInt(availableH / 7, S(26), S(34));

    auto rowY = [&](int row) {
        // Add a small gap after the general section (2 rows).
        const int extra = (row >= 2) ? sectionGap : 0;
        return top + row * rowH + extra;
    };
    const int left = content.left + pad;

    auto mkLabel = [&](CStatic& s, int row) {
        const int y = rowY(row);
        CRect rc(left, y, left + labelW, y + rowH);
        s.Create(L"", WS_CHILD | WS_VISIBLE, rc, this);
        if (font_.GetSafeHandle()) s.SetFont(&font_);
    };
    auto mkCombo = [&](CComboBox& c, int row, UINT id) {
        const int dropH = S(140);
        const int y = rowY(row);
        CRect rc(left + labelW + S(8), y - S(2),
                 left + labelW + S(8) + boxW, y + rowH + dropH);
        c.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, rc, this, id);
        if (font_.GetSafeHandle()) c.SetFont(&font_);
    };

    // General section (2 rows), then a small gap, then Effects (5 rows)
    mkLabel(lblLang_, 0);   mkCombo(cmbLang_, 0, 11000);
    mkLabel(lblTheme_, 1);  mkCombo(cmbTheme_, 1, 11001);
    mkLabel(lblClick_, 2);  mkCombo(cmbClick_, 2, 11002);
    mkLabel(lblTrail_, 3);  mkCombo(cmbTrail_, 3, 11003);
    mkLabel(lblScroll_, 4); mkCombo(cmbScroll_, 4, 11004);
    mkLabel(lblHold_, 5);   mkCombo(cmbHold_, 5, 11005);
    mkLabel(lblHover_, 6);  mkCombo(cmbHover_, 6, 11006);

    CRect footer = RcFooter();
    const int btnW = S(120);
    const int btnH = S(30);
    CRect rcApply(footer.right - pad - btnW, footer.top + S(10), footer.right - pad, footer.top + S(10) + btnH);
    CRect rcClose = rcApply;
    rcClose.OffsetRect(-(btnW + S(12)), 0);
    btnClose_.Create(L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rcClose, this, 10002);
    btnApply_.Create(L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rcApply, this, 10001);
    if (font_.GetSafeHandle()) {
        btnClose_.SetFont(&font_);
        btnApply_.SetFont(&font_);
    }

    ApplyLanguageToControls();
    SetComboValue(cmbLang_, model_.uiLanguage);
    SetComboValue(cmbTheme_, model_.theme);
    SetComboValue(cmbClick_, model_.click);
    SetComboValue(cmbTrail_, model_.trail);
    SetComboValue(cmbScroll_, model_.scroll);
    SetComboValue(cmbHold_, model_.hold);
    SetComboValue(cmbHover_, model_.hover);

    return 0;
}

void CSettingsWnd::OnDestroy() {
    CWnd::OnDestroy();
    auto* app = dynamic_cast<CMFCMouseEffectApp*>(AfxGetApp());
    if (app) {
        app->NotifySettingsWndDestroyed((CSettingsWnd*)this);
    }
}

void CSettingsWnd::OnClose() {
    DestroyWindow();
}

void CSettingsWnd::OnCommandApply() {
    Apply();
}

void CSettingsWnd::OnCommandClose() {
    OnClose();
}

void CSettingsWnd::OnSelChange() {
    if (updating_) return;
    model_.uiLanguage = GetComboValue(cmbLang_);
    ApplyLanguageToControls();

    model_.theme = GetComboValue(cmbTheme_);
    model_.click = GetComboValue(cmbClick_);
    model_.trail = GetComboValue(cmbTrail_);
    model_.scroll = GetComboValue(cmbScroll_);
    model_.hold = GetComboValue(cmbHold_);
    model_.hover = GetComboValue(cmbHover_);

    Apply();
}

int CSettingsWnd::Dpi() const {
    HMODULE user32 = GetModuleHandleW(L"user32.dll");
    if (user32) {
        using GetDpiForWindowFn = UINT(WINAPI*)(HWND);
        auto* fn = (GetDpiForWindowFn)GetProcAddress(user32, "GetDpiForWindow");
        if (fn) {
            return (int)fn(GetSafeHwnd());
        }
    }
    return 96;
}

int CSettingsWnd::S(int px) const {
    return (px * Dpi() + 48) / 96;
}

CRect CSettingsWnd::Client() const {
    CRect rc;
    GetClientRect(&rc);
    return rc;
}

CRect CSettingsWnd::RcHeader() const {
    CRect rc = Client();
    rc.bottom = S(64);
    return rc;
}

CRect CSettingsWnd::RcCloseBtn() const {
    CRect rc = RcHeader();
    const int sz = S(28);
    rc.left = rc.right - S(16) - sz;
    rc.top = rc.top + S(12);
    rc.right = rc.left + sz;
    rc.bottom = rc.top + sz;
    return rc;
}

CRect CSettingsWnd::RcContent() const {
    CRect rc = Client();
    rc.DeflateRect(S(16), S(72), S(16), S(72));
    return rc;
}

CRect CSettingsWnd::RcFooter() const {
    CRect rc = Client();
    rc.top = rc.bottom - S(56);
    return rc;
}

CRect CSettingsWnd::RcApplyBtn() const {
    CRect rc = RcFooter();
    const int w = S(120);
    const int h = S(32);
    rc.right -= S(16);
    rc.left = rc.right - w;
    rc.top = rc.top + S(12);
    rc.bottom = rc.top + h;
    rc.right = rc.left + w;
    return rc;
}

CRect CSettingsWnd::RcCloseBtn2() const {
    CRect rc = RcApplyBtn();
    rc.OffsetRect(-(S(120) + S(12)), 0);
    return rc;
}

CSettingsWnd::Hit CSettingsWnd::HitTest(const CPoint& pt) const {
    Hit h;
    const CRect close = RcCloseBtn();
    if (close.PtInRect(pt)) {
        h.kind = Hit::Close;
        h.rc = close;
        return h;
    }
    return h;
}

bool CSettingsWnd::IsZh() const {
    return model_.uiLanguage.empty() || model_.uiLanguage == "zh-CN";
}

void CSettingsWnd::FillCombo(CComboBox& box, const SettingOption* opts, size_t count, const std::string& current) {
    box.ResetContent();
    int sel = 0;
    for (size_t i = 0; i < count; ++i) {
        const int idx = box.AddString(opts[i].display);
        box.SetItemDataPtr(idx, (void*)opts[i].value);
        if (current == opts[i].value) sel = idx;
    }
    box.SetCurSel(sel);
}

std::string CSettingsWnd::GetComboValue(const CComboBox& box) const {
    const int i = box.GetCurSel();
    if (i < 0) return {};
    return GetValueFromItemData(box.GetItemData(i));
}

void CSettingsWnd::SetComboValue(CComboBox& box, const std::string& value) {
    const int count = box.GetCount();
    for (int i = 0; i < count; ++i) {
        if (GetValueFromItemData(box.GetItemData(i)) == value) {
            box.SetCurSel(i);
            return;
        }
    }
    box.SetCurSel(0);
}

void CSettingsWnd::ApplyLanguageToControls() {
    updating_ = true;
    size_t n = 0;

    const SettingsText& t = IsZh() ? TextZh() : TextEn();

    SetWindowTextW(t.title);

    lblLang_.SetWindowTextW(t.labelLanguage);
    lblTheme_.SetWindowTextW(t.labelTheme);
    lblClick_.SetWindowTextW(t.labelClick);
    lblTrail_.SetWindowTextW(t.labelTrail);
    lblScroll_.SetWindowTextW(t.labelScroll);
    lblHold_.SetWindowTextW(t.labelHold);
    lblHover_.SetWindowTextW(t.labelHover);

    btnClose_.SetWindowTextW(t.btnClose);
    btnApply_.SetWindowTextW(t.btnApply);

    const SettingOption* langOpts = LangOptions(n);
    FillCombo(cmbLang_, langOpts, n, model_.uiLanguage);

    const SettingOption* themeOpts = ThemeOptions(IsZh(), n);
    FillCombo(cmbTheme_, themeOpts, n, model_.theme);

    const SettingOption* clickOpts = ClickOptions(IsZh(), n);
    FillCombo(cmbClick_, clickOpts, n, model_.click);

    const SettingOption* trailOpts = TrailOptions(IsZh(), n);
    FillCombo(cmbTrail_, trailOpts, n, model_.trail);

    const SettingOption* scrollOpts = ScrollOptions(IsZh(), n);
    FillCombo(cmbScroll_, scrollOpts, n, model_.scroll);

    const SettingOption* holdOpts = HoldOptions(IsZh(), n);
    FillCombo(cmbHold_, holdOpts, n, model_.hold);

    const SettingOption* hoverOpts = HoverOptions(IsZh(), n);
    FillCombo(cmbHover_, hoverOpts, n, model_.hover);
    updating_ = false;
}

void CSettingsWnd::Apply() {
    if (!backend_) return;
    backend_->Apply(model_);
    Invalidate(FALSE);
}

void CSettingsWnd::SyncFromBackend() {
    if (!backend_) return;
    model_ = backend_->Load();
    if (model_.uiLanguage.empty()) model_.uiLanguage = "zh-CN";
    ApplyLanguageToControls();
    Invalidate(FALSE);
}

void CSettingsWnd::OnLButtonDown(UINT nFlags, CPoint point) {
    CWnd::OnLButtonDown(nFlags, point);
    down_ = HitTest(point).kind;
}

void CSettingsWnd::OnLButtonUp(UINT nFlags, CPoint point) {
    CWnd::OnLButtonUp(nFlags, point);
    const Hit h = HitTest(point);
    if (h.kind == Hit::Close && down_ == Hit::Close) {
        OnClose();
    }
    down_ = Hit::None;
}

void CSettingsWnd::OnMouseMove(UINT nFlags, CPoint point) {
    CWnd::OnMouseMove(nFlags, point);
    const Hit h = HitTest(point);
    if (hover_ != h.kind) {
        hover_ = h.kind;
        Invalidate(FALSE);
    }
}

LRESULT CSettingsWnd::OnNcHitTest(CPoint point) {
    CPoint client = point;
    ScreenToClient(&client);
    if (RcCloseBtn().PtInRect(client)) {
        return HTCLIENT;
    }
    if (RcHeader().PtInRect(client)) {
        return HTCAPTION;
    }
    return CWnd::OnNcHitTest(point);
}

void CSettingsWnd::DrawRoundRect(Gdiplus::Graphics& g, const CRect& rc, int radius, const Gdiplus::Color& fill) {
    const int r = ClampInt(radius, 0, 32);
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

void CSettingsWnd::DrawText(Gdiplus::Graphics& g, const wchar_t* text, const CRect& rc, int sizePx, bool bold, const Gdiplus::Color& c) {
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

    g.DrawString(text, -1, &font, r, &fmt, &b);
}

void CSettingsWnd::OnPaint() {
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

    // Background (system default)
    const COLORREF sysWindow = ::GetSysColor(COLOR_WINDOW);
    const COLORREF sysText = ::GetSysColor(COLOR_WINDOWTEXT);
    const COLORREF sysMuted = ::GetSysColor(COLOR_GRAYTEXT);
    const COLORREF sysFace = ::GetSysColor(COLOR_BTNFACE);
    g.Clear(C(255, GetRValue(sysWindow), GetGValue(sysWindow), GetBValue(sysWindow)));

    // Header
    {
        CRect h = RcHeader();
        DrawRoundRect(g, CRect(h.left + S(8), h.top + S(8), h.right - S(8), h.bottom), S(14),
            C(255, GetRValue(sysFace), GetGValue(sysFace), GetBValue(sysFace)));

        const SettingsText& t = IsZh() ? TextZh() : TextEn();
        CRect title = h;
        title.left += S(18);
        title.top += S(8);
        title.bottom = title.top + S(34);
        DrawText(g, t.title, title, S(16), true, C(255, GetRValue(sysText), GetGValue(sysText), GetBValue(sysText)));

        CRect sub = h;
        sub.left += S(18);
        sub.top += S(40);
        sub.bottom = h.bottom;
        DrawText(g, t.subtitle, sub, S(11), false, C(255, GetRValue(sysMuted), GetGValue(sysMuted), GetBValue(sysMuted)));

        CRect x = RcCloseBtn();
        const bool hov = (hover_ == Hit::Close);
        DrawRoundRect(g, x, S(10), hov ? C(255, 210, 215, 223) : C(255, 228, 233, 240));
        DrawText(g, L"\u00D7", x, S(16), true, C(255, GetRValue(sysText), GetGValue(sysText), GetBValue(sysText)));
    }

    dc.BitBlt(0, 0, rc.Width(), rc.Height(), &mem, 0, 0, SRCCOPY);
    mem.SelectObject(oldBmp);
}

BOOL CSettingsWnd::OnEraseBkgnd(CDC* /*pDC*/) {
    return TRUE;
}

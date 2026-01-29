#include "pch.h"
#include "SettingsWnd.h"

#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

#include "MFCMouseEffect.h"
#include "Settings/SettingsBackend.h"
#include "Settings/SettingsOptions.h"

#include <vector>

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
    ON_BN_CLICKED(10003, &CSettingsWnd::OnCommandReset) // Reset button
    ON_CBN_SELCHANGE(11000, &CSettingsWnd::OnSelChange)
    ON_CBN_SELCHANGE(11001, &CSettingsWnd::OnSelChange)
    ON_CBN_SELCHANGE(11002, &CSettingsWnd::OnSelChange)
    ON_CBN_SELCHANGE(11003, &CSettingsWnd::OnSelChange)
    ON_CBN_SELCHANGE(11004, &CSettingsWnd::OnSelChange)
    ON_CBN_SELCHANGE(11005, &CSettingsWnd::OnSelChange)
    ON_CBN_SELCHANGE(11006, &CSettingsWnd::OnSelChange)
    ON_EN_CHANGE(11007, &CSettingsWnd::OnTextChange)
    ON_EN_SETFOCUS(11007, &CSettingsWnd::OnTextFocus)
    ON_EN_KILLFOCUS(11007, &CSettingsWnd::OnTextKillFocus)
    ON_WM_SETCURSOR()
END_MESSAGE_MAP()

bool CSettingsWnd::CreateAndShow(CWnd* parent, std::unique_ptr<ISettingsBackend> backend) {
    backend_ = std::move(backend);
    if (!backend_) return false;

    const CString cls = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LoadCursor(nullptr, IDC_ARROW));

    // Custom frame only; internal controls use native styles.
    DWORD style = WS_POPUP | WS_VISIBLE;
    DWORD ex = WS_EX_APPWINDOW | WS_EX_COMPOSITED;

    CRect rc(0, 0, 560, 560);
    CPoint ptCenter(0, 0);

    if (parent && ::IsWindow(parent->GetSafeHwnd())) {
        // Center relative to parent monitor
        HMONITOR hMonitor = MonitorFromWindow(parent->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        if (GetMonitorInfo(hMonitor, &mi)) {
            ptCenter = CRect(mi.rcWork).CenterPoint();
        } else {
            parent->GetWindowRect(&rc);
            ptCenter = rc.CenterPoint();
        }
    } else {
        // Center on primary monitor or nearest to cursor
        POINT cursorPt;
        GetCursorPos(&cursorPt);
        HMONITOR hMonitor = MonitorFromPoint(cursorPt, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        if (GetMonitorInfo(hMonitor, &mi)) {
            ptCenter = CRect(mi.rcWork).CenterPoint();
        } else {
            ptCenter.x = GetSystemMetrics(SM_CXSCREEN) / 2;
            ptCenter.y = GetSystemMetrics(SM_CYSCREEN) / 2;
        }
    }

    rc.left = ptCenter.x - 280;
    rc.top = ptCenter.y - 280;
    rc.right = rc.left + 560;
    rc.bottom = rc.top + 560;

    if (!CreateEx(ex, cls, L"", style, rc, parent, 0)) {
        return false;
    }

    ShowWindow(SW_SHOW);
    UpdateWindow();
    return true;
}

int CSettingsWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CWnd::OnCreate(lpCreateStruct) == -1) return -1;

    AfxInitRichEdit2();
    richeditModule_ = LoadLibraryW(L"Msftedit.dll");

    model_ = backend_->Load();
    if (model_.uiLanguage.empty()) model_.uiLanguage = "zh-CN";

    NONCLIENTMETRICS ncm{};
    ncm.cbSize = sizeof(ncm);
    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0)) {
        font_.CreateFontIndirect(&ncm.lfMessageFont);
        LOGFONT lf = ncm.lfMessageFont;
        wcscpy_s(lf.lfFaceName, _countof(lf.lfFaceName), L"Segoe UI Emoji");
        fontEdit_.CreateFontIndirect(&lf);
    }

    const CRect content = RcContent();
    const int pad = S(14);
    const int top = content.top;
    const int labelW = S(120);
    const int boxW = content.Width() - labelW - pad * 3;
    const int sectionGap = S(10);
    const int availableH = content.Height();
    const int rowH = ClampInt(availableH / 8, S(26), S(34)); // Divided by 8 rows now

    auto rowY = [&](int row) {
        // Add a small gap after the general section (2 rows).
        const int extra = (row >= 2) ? sectionGap : 0;
        return top + row * rowH + extra;
    };
    const int left = content.left + pad;

    auto mkLabel = [&](CStatic& s, int row) {
        const int y = rowY(row);
        CRect rc(left + S(8), y, left + labelW, y + rowH);
        s.Create(L"", WS_CHILD | WS_VISIBLE | SS_CENTERIMAGE | SS_LEFT, rc, this);
        if (font_.GetSafeHandle()) s.SetFont(&font_);
    };
    auto mkCombo = [&](CComboBox& c, int row, UINT id) {
        const int dropH = S(140);
        const int y = rowY(row);
        CRect rc(left + labelW + S(8), y + S(2),
                 left + labelW + S(8) + boxW, y + rowH + dropH);
        c.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, rc, this, id);
        if (font_.GetSafeHandle()) c.SetFont(&font_);
    };
    auto mkEdit = [&](CRichEditCtrl& e, int row, UINT id) {
        const int y = rowY(row);
        CRect rc(left + labelW + S(8), y + S(2),
                 left + labelW + S(8) + boxW, y + rowH);
        HWND hEdit = CreateWindowExW(
            0,
            L"RICHEDIT50W",
            L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
            rc.left, rc.top, rc.Width(), rc.Height(),
            GetSafeHwnd(),
            (HMENU)(UINT_PTR)id,
            AfxGetInstanceHandle(),
            nullptr);
        if (hEdit) {
            e.SubclassWindow(hEdit);
            ::SendMessageW(hEdit, EM_SETEDITSTYLE, SES_USECTF, SES_USECTF);
        }
        if (fontEdit_.GetSafeHandle()) e.SetFont(&fontEdit_);
        else if (font_.GetSafeHandle()) e.SetFont(&font_);
    };

    // General section (2 rows)
    mkLabel(lblLang_, 0);   mkCombo(cmbLang_, 0, 11000);
    mkLabel(lblTheme_, 1);  mkCombo(cmbTheme_, 1, 11001);
    
    // Effects section
    mkLabel(lblClick_, 2);  mkCombo(cmbClick_, 2, 11002);
    mkLabel(lblTrail_, 3);  mkCombo(cmbTrail_, 3, 11003);
    mkLabel(lblScroll_, 4); mkCombo(cmbScroll_, 4, 11004);
    mkLabel(lblHold_, 5);   mkCombo(cmbHold_, 5, 11005);
    mkLabel(lblHover_, 6);  mkCombo(cmbHover_, 6, 11006);
    
    // Text content
    mkLabel(lblTexts_, 7);  mkEdit(edtTexts_, 7, 11007);
    {
        CRect rc;
        edtTexts_.GetWindowRect(&rc);
        ScreenToClient(&rc);
        emojiPreview_.Create(this, rc);
        emojiPreview_.SetFontSizePx(GetEditFontPx());
        emojiPreview_.SetTextColor(::GetSysColor(COLOR_WINDOWTEXT));
        emojiPreview_.ShowWindow(SW_HIDE);
    }

    CRect footer = RcFooter();
    const int btnW = S(120);
    const int btnH = S(30);
    
    // Layout: Reset (Left), Close (Center), Apply (Right)
    
    // Left side: Reset
    CRect rcReset(footer.left + pad, footer.top + S(10), footer.left + pad + btnW, footer.top + S(10) + btnH);

    // Center: Close
    int centerX = footer.CenterPoint().x;
    CRect rcClose(centerX - btnW / 2, footer.top + S(10), centerX + btnW / 2, footer.top + S(10) + btnH);

    // Right side: Apply
    CRect rcApply(footer.right - pad - btnW, footer.top + S(10), footer.right - pad, footer.top + S(10) + btnH);

    btnClose_.Create(L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rcClose, this, 10002);
    btnApply_.Create(L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rcApply, this, 10001);
    btnReset_.Create(L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rcReset, this, 10003);

    if (font_.GetSafeHandle()) {
        btnClose_.SetFont(&font_);
        btnApply_.SetFont(&font_);
        btnReset_.SetFont(&font_);
    }

    ApplyLanguageToControls();
    
    SetComboValue(cmbLang_, model_.uiLanguage);
    SetComboValue(cmbTheme_, model_.theme);
    SetComboValue(cmbClick_, model_.click);
    SetComboValue(cmbTrail_, model_.trail);
    SetComboValue(cmbScroll_, model_.scroll);
    SetComboValue(cmbHold_, model_.hold);
    SetComboValue(cmbHover_, model_.hover);
    
    // Set edit text from UTF-8 string model
    CString wText;
    int len = MultiByteToWideChar(CP_UTF8, 0, model_.textContent.c_str(), -1, nullptr, 0);
    if (len > 0) {
        std::vector<wchar_t> buf(len);
        MultiByteToWideChar(CP_UTF8, 0, model_.textContent.c_str(), -1, buf.data(), len);
        wText = buf.data();
    }
    {
        CHARFORMAT2W cf{};
        cf.cbSize = sizeof(cf);
        cf.dwMask = CFM_FACE;
        wcscpy_s(cf.szFaceName, _countof(cf.szFaceName), L"Segoe UI");
        edtTexts_.SetDefaultCharFormat(cf);
    }
    edtTexts_.SetWindowTextW(wText);
    ApplyEmojiFormatting();
    UpdatePreviewFromEdit();

    return 0;
}

void CSettingsWnd::OnDestroy() {
    CWnd::OnDestroy();
    auto* app = dynamic_cast<CMFCMouseEffectApp*>(AfxGetApp());
    if (app) {
        app->NotifySettingsWndDestroyed((CSettingsWnd*)this);
    }
    if (::IsWindow(emojiPreview_.GetSafeHwnd())) {
        emojiPreview_.DestroyWindow();
    }
    if (richeditModule_) {
        FreeLibrary(richeditModule_);
        richeditModule_ = nullptr;
    }
}

void CSettingsWnd::OnClose() {
    DestroyWindow();
}

// Helper: capture UI state to model
void CSettingsWnd::CaptureUI() {
    model_.uiLanguage = GetComboValue(cmbLang_);
    model_.theme = GetComboValue(cmbTheme_);
    model_.click = GetComboValue(cmbClick_);
    model_.trail = GetComboValue(cmbTrail_);
    model_.scroll = GetComboValue(cmbScroll_);
    model_.hold = GetComboValue(cmbHold_);
    model_.hover = GetComboValue(cmbHover_);
    
    // Capture text content
    CString wText;
    edtTexts_.GetWindowTextW(wText);
    if (wText.IsEmpty()) {
        model_.textContent.clear();
    } else {
        int len = WideCharToMultiByte(CP_UTF8, 0, wText, -1, nullptr, 0, nullptr, nullptr);
        if (len > 0) {
            std::string utf8(len - 1, 0);
            WideCharToMultiByte(CP_UTF8, 0, wText, -1, &utf8[0], len, nullptr, nullptr);
            model_.textContent = utf8;
        } else {
            model_.textContent.clear();
        }
    }
}

void CSettingsWnd::OnCommandApply() {
    CaptureUI();
    Apply();
}

void CSettingsWnd::OnCommandReset() {
    if (backend_) {
        backend_->ResetToDefaults();
        SyncFromBackend();
    }
}

void CSettingsWnd::OnCommandClose() {
    OnClose();
}

void CSettingsWnd::OnSelChange() {
    if (updating_) return;

    std::string oldLang = model_.uiLanguage;

    CaptureUI();

    if (model_.uiLanguage != oldLang) {
        ApplyLanguageToControls();
    }

    Apply();
}

void CSettingsWnd::OnTextChange() {
    ApplyEmojiFormatting();
    UpdatePreviewFromEdit();
}

void CSettingsWnd::OnTextFocus() {
    if (::IsWindow(emojiPreview_.GetSafeHwnd())) {
        emojiPreview_.ShowWindow(SW_HIDE);
    }
}

void CSettingsWnd::OnTextKillFocus() {
    UpdatePreviewFromEdit();
    if (::IsWindow(emojiPreview_.GetSafeHwnd())) {
        emojiPreview_.ShowWindow(SW_SHOW);
    }
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

CRect CSettingsWnd::RcStarLink() const {
    CRect close = RcCloseBtn();
    CRect rc = close;
    const int w = S(120);
    rc.right = close.left - S(10);
    rc.left = rc.right - w;
    // same top/bottom as close
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
    const int h = S(32); // Note: OnCreate uses S(30), keeping this S(32) for hit/draw slightly larger is fine, or match it. Match S(30)? 
                         // Previous code had S(32). Let's stick to OnCreate logic logic roughly but use helper logic.
                         // Actually OnCreate uses pad S(14).
    // Let's match OnCreate logic exactly for consistency. 
    // OnCreate: top + S(10), height S(30).
    // Helper: top + S(12), height S(32).
    // Small discrepancy exists in original code. I will align them to the new logic.
    const int pad = S(14);
    
    rc.right -= pad;
    rc.left = rc.right - w;
    rc.top = rc.top + S(10);
    rc.bottom = rc.top + S(30);
    return rc;
}

CRect CSettingsWnd::RcResetBtn() const {
    CRect rc = RcFooter();
    const int w = S(120);
    const int pad = S(14);
    
    rc.left += pad;
    rc.right = rc.left + w;
    rc.top = rc.top + S(10);
    rc.bottom = rc.top + S(30);
    return rc;
}

CRect CSettingsWnd::RcCloseBtn2() const {
    CRect rc = RcFooter();
    const int w = S(120);
    int centerX = rc.CenterPoint().x;
    
    rc.left = centerX - w / 2;
    rc.right = centerX + w / 2;
    rc.top = rc.top + S(10);
    rc.bottom = rc.top + S(30);
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
    const CRect star = RcStarLink();
    if (star.PtInRect(pt)) {
        h.kind = Hit::Star;
        h.rc = star;
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
    lblTexts_.SetWindowTextW(t.labelTextsEntry);

    btnClose_.SetWindowTextW(t.btnClose);
    btnApply_.SetWindowTextW(t.btnApply);
    btnReset_.SetWindowTextW(t.btnReset);

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
    
    // Convert UTF-8 model.textContent to wstring
    CString wText;
    int len = MultiByteToWideChar(CP_UTF8, 0, model_.textContent.c_str(), -1, nullptr, 0);
    if (len > 0) {
        std::vector<wchar_t> buf(len);
        MultiByteToWideChar(CP_UTF8, 0, model_.textContent.c_str(), -1, buf.data(), len);
        wText = buf.data();
    }
    edtTexts_.SetWindowTextW(wText);
    ApplyEmojiFormatting();
    UpdatePreviewFromEdit();

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
    } else if (h.kind == Hit::Star && down_ == Hit::Star) {
        ShellExecute(nullptr, L"open", L"https://github.com/sqmw/MFCMouseEffect", nullptr, nullptr, SW_SHOWNORMAL);
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
    if (RcStarLink().PtInRect(client)) {
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

void CSettingsWnd::DrawText(Gdiplus::Graphics& g, const wchar_t* text, const CRect& rc, int sizePx, bool bold, const Gdiplus::Color& c, Gdiplus::StringAlignment align /*= Gdiplus::StringAlignmentCenter*/) {
    if (!text) return;
    Gdiplus::FontFamily ff(L"Segoe UI");
    const int style = bold ? Gdiplus::FontStyleBold : Gdiplus::FontStyleRegular;
    Gdiplus::Font font(&ff, (Gdiplus::REAL)sizePx, style, Gdiplus::UnitPixel);
    Gdiplus::SolidBrush b(c);

    Gdiplus::RectF r((Gdiplus::REAL)rc.left, (Gdiplus::REAL)rc.top, (Gdiplus::REAL)rc.Width(), (Gdiplus::REAL)rc.Height());
    Gdiplus::StringFormat fmt;
    fmt.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
    fmt.SetFormatFlags(Gdiplus::StringFormatFlagsNoWrap);
    fmt.SetLineAlignment(Gdiplus::StringAlignmentCenter); // Vertical center always
    fmt.SetAlignment(align);

    g.DrawString(text, -1, &font, r, &fmt, &b);
}

void CSettingsWnd::ApplyEmojiFormatting() {
    if (updatingText_) return;
    updatingText_ = true;

    CString wText;
    edtTexts_.GetWindowTextW(wText);
    const std::wstring text = wText.GetString();

    CHARRANGE sel{};
    edtTexts_.GetSel(sel);

    CHARFORMAT2W normal{};
    normal.cbSize = sizeof(normal);
    normal.dwMask = CFM_FACE;
    wcscpy_s(normal.szFaceName, _countof(normal.szFaceName), L"Segoe UI");

    CHARFORMAT2W emoji{};
    emoji.cbSize = sizeof(emoji);
    emoji.dwMask = CFM_FACE;
    wcscpy_s(emoji.szFaceName, _countof(emoji.szFaceName), L"Segoe UI Emoji");

    edtTexts_.SetSel(0, -1);
    edtTexts_.SetSelectionCharFormat(normal);

    for (size_t i = 0; i < text.size(); ) {
        size_t runStart = i;
        size_t next = i;
        uint32_t cp = NextCodePoint(text, &next);
        if (cp == 0) break;
        i = next;
        if (!IsEmojiCodePoint(cp)) continue;

        size_t runEnd = i;
        while (runEnd < text.size()) {
            size_t probe = runEnd;
            uint32_t cp2 = NextCodePoint(text, &probe);
            if (cp2 == 0) break;
            if (!IsEmojiComponent(cp2)) break;
            runEnd = probe;
        }
        edtTexts_.SetSel((long)runStart, (long)runEnd);
        edtTexts_.SetSelectionCharFormat(emoji);
        i = runEnd;
    }

    edtTexts_.SetSel(sel);
    updatingText_ = false;
}

float CSettingsWnd::GetEditFontPx() const {
    if (!::IsWindow(edtTexts_.GetSafeHwnd())) return 14.0f;
    CClientDC dc((CWnd*)&edtTexts_);
    CFont* old = nullptr;
    if (fontEdit_.GetSafeHandle()) {
        old = dc.SelectObject((CFont*)&fontEdit_);
    } else if (font_.GetSafeHandle()) {
        old = dc.SelectObject((CFont*)&font_);
    }
    TEXTMETRIC tm{};
    dc.GetTextMetrics(&tm);
    if (old) dc.SelectObject(old);
    const int px = tm.tmHeight - tm.tmInternalLeading;
    return (float)((px > 0) ? px : tm.tmHeight);
}

void CSettingsWnd::UpdatePreviewFromEdit() {
    if (!::IsWindow(emojiPreview_.GetSafeHwnd())) return;
    CString wText;
    edtTexts_.GetWindowTextW(wText);
    emojiPreview_.SetText(wText.GetString());
    emojiPreview_.SetFontSizePx(GetEditFontPx());
    CRect rc;
    edtTexts_.GetWindowRect(&rc);
    ScreenToClient(&rc);
    emojiPreview_.SetTargetRect(rc);
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
        DrawText(g, t.title, title, S(16), true, C(255, GetRValue(sysText), GetGValue(sysText), GetBValue(sysText)), Gdiplus::StringAlignmentNear);

        CRect sub = h;
        sub.left += S(18);
        sub.top += S(44); // Lowered slightly for better gap from title
        sub.bottom = h.bottom;
        DrawText(g, t.subtitle, sub, S(11), false, C(255, GetRValue(sysMuted), GetGValue(sysMuted), GetBValue(sysMuted)), Gdiplus::StringAlignmentNear);

        CRect x = RcCloseBtn();
        const bool hov = (hover_ == Hit::Close);
        DrawRoundRect(g, x, S(10), hov ? C(255, 210, 215, 223) : C(255, 228, 233, 240));
        DrawText(g, L"\u00D7", x, S(16), true, C(255, GetRValue(sysText), GetGValue(sysText), GetBValue(sysText)));

        // Star link
        CRect star = RcStarLink();
        const bool hoverStar = (hover_ == Hit::Star);
        // Use a gold-ish color for the star, text color for text, or just a nice link color?
        // Let's use Gold for the star icon part if we could separate, but we pass full string.
        // Let's just use a distinct color.
        // Gold: 255, 215, 0 (Web Gold).
        // Blue link: 0, 102, 204.
        Gdiplus::Color linkColor = hoverStar ? C(255, 0, 102, 204) : C(255, 120, 120, 120);
        // Actually, maybe just keep it subtle until hover?
        // Let's match the Close button style -> Text color but maybe styled?
        // User wants attention. Let's make it Gold/Orange?
        // C(255, 255, 140, 0) DarkOrange.
        DrawText(g, t.btnStar, star, S(12), true, C(255, 255, 140, 0));
    }

    dc.BitBlt(0, 0, rc.Width(), rc.Height(), &mem, 0, 0, SRCCOPY);
    mem.SelectObject(oldBmp);
}

BOOL CSettingsWnd::OnEraseBkgnd(CDC* /*pDC*/) {
    return TRUE;
}

BOOL CSettingsWnd::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) {
    if (hover_ == Hit::Star) {
        ::SetCursor(::LoadCursor(nullptr, IDC_HAND));
        return TRUE;
    }
    return CWnd::OnSetCursor(pWnd, nHitTest, message);
}

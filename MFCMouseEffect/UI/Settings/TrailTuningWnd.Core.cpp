#include "pch.h"
#include "UI/Settings/TrailTuningWnd.h"

#include "Settings/SettingsBackend.h"
#include "Settings/SettingsOptions.h"

namespace {

static int ClampIntLocal(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

} // namespace

BEGIN_MESSAGE_MAP(CTrailTuningWnd, CWnd)
    ON_WM_CREATE()
    ON_WM_PAINT()
    ON_WM_ERASEBKGND()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_NCHITTEST()
    ON_WM_CLOSE()
    ON_WM_DESTROY()
    ON_BN_CLICKED(kIdApply, &CTrailTuningWnd::OnCommandApply)
    ON_BN_CLICKED(kIdReset, &CTrailTuningWnd::OnCommandReset)
    ON_BN_CLICKED(kIdClose, &CTrailTuningWnd::OnClose)
    ON_CBN_SELCHANGE(kIdPreset, &CTrailTuningWnd::OnSelChangePreset)
END_MESSAGE_MAP()

bool CTrailTuningWnd::CreateAndShow(CWnd* parent, ISettingsBackend* backend) {
    backend_ = backend;
    if (!backend_) return false;

    const CString cls = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LoadCursor(nullptr, IDC_ARROW));
    DWORD style = WS_POPUP | WS_VISIBLE;
    DWORD ex = WS_EX_APPWINDOW | WS_EX_COMPOSITED;

    CRect rc(0, 0, 860, 560);
    CPoint ptCenter(0, 0);

    if (parent && ::IsWindow(parent->GetSafeHwnd())) {
        HMONITOR hMonitor = MonitorFromWindow(parent->GetSafeHwnd(), MONITOR_DEFAULTTONEAREST);
        MONITORINFO mi = { sizeof(mi) };
        if (GetMonitorInfo(hMonitor, &mi)) {
            ptCenter = CRect(mi.rcWork).CenterPoint();
        } else {
            CRect prc;
            parent->GetWindowRect(&prc);
            ptCenter = prc.CenterPoint();
        }
    } else {
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

    rc.left = ptCenter.x - rc.Width() / 2;
    rc.top = ptCenter.y - rc.Height() / 2;
    rc.right = rc.left + 860;
    rc.bottom = rc.top + 560;

    if (!CreateEx(ex, cls, L"", style, rc, parent, 0)) {
        return false;
    }

    ShowWindow(SW_SHOW);
    UpdateWindow();
    return true;
}

int CTrailTuningWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CWnd::OnCreate(lpCreateStruct) == -1) return -1;

    if (backend_) {
        SettingsModel sm = backend_->Load();
        if (!sm.uiLanguage.empty()) uiLanguage_ = sm.uiLanguage;
    }

    NONCLIENTMETRICS ncm{};
    ncm.cbSize = sizeof(ncm);
    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0)) {
        font_.CreateFontIndirect(&ncm.lfMessageFont);
    }

    const CRect content = RcContent();
    const int pad = S(14);
    const int left = content.left + pad;
    const int top = content.top + pad;
    const int w = content.Width() - pad * 2;
    const int h = content.Height() - pad * 2;
    const int colGap = S(12);
    const int colW = (w - colGap) / 2;
    const int leftX = left;
    const int rightX = left + colW + colGap;
    const int rowH = ClampIntLocal(h / 9, S(24), S(30));

    auto mkStatic = [&](CStatic& s, int xx, int yy, int ww, int hh) {
        s.Create(L"", WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, CRect(xx, yy, xx + ww, yy + hh), this);
        if (font_.GetSafeHandle()) s.SetFont(&font_);
    };
    auto mkEdit = [&](CEdit& e, int xx, int yy, int ww, int hh) {
        e.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, CRect(xx, yy, xx + ww, yy + hh), this, 0);
        if (font_.GetSafeHandle()) e.SetFont(&font_);
    };

    int y = top;

    mkStatic(lblPreset_, leftX, y, S(150), rowH);
    cmbPreset_.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, CRect(leftX + S(160), y, leftX + S(460), y + S(220)), this, kIdPreset);
    if (font_.GetSafeHandle()) cmbPreset_.SetFont(&font_);
    cmbPreset_.AddString(L"Default");
    cmbPreset_.AddString(L"Snappy");
    cmbPreset_.AddString(L"Long");
    cmbPreset_.AddString(L"Cinematic");
    cmbPreset_.AddString(L"Custom");

    y += rowH + S(10);

    // Left: profiles
    mkStatic(lblProfiles_, leftX, y, colW, rowH);
    int yL = y + rowH + S(6);

    const int profileLabelW = S(180);
    const int editW = (colW - profileLabelW - S(12)) / 2;
    mkStatic(lblType_, leftX, yL, profileLabelW, rowH);
    mkStatic(lblDuration_, leftX + profileLabelW, yL, editW, rowH);
    mkStatic(lblMaxPoints_, leftX + profileLabelW + editW + S(8), yL, editW, rowH);
    yL += rowH + S(4);

    auto mkProfileRow = [&](CStatic& label, ProfileControls& pc) {
        mkStatic(label, leftX, yL, profileLabelW, rowH);
        mkEdit(pc.duration, leftX + profileLabelW, yL, editW, rowH);
        mkEdit(pc.maxPoints, leftX + profileLabelW + editW + S(8), yL, editW, rowH);
        yL += rowH + S(6);
    };
    mkProfileRow(lblLine_, line_);
    mkProfileRow(lblStreamer_, streamer_);
    mkProfileRow(lblElectric_, electric_);
    mkProfileRow(lblMeteor_, meteor_);
    mkProfileRow(lblTubes_, tubes_);

    // Right: params
    mkStatic(lblParams_, rightX, y, colW, rowH);
    int yR = y + rowH + S(6);
    const int paramLabelW = S(260);
    const int paramEditW = colW - paramLabelW - S(8);

    auto mkParamRow = [&](CStatic& label, CEdit& edit) {
        mkStatic(label, rightX, yR, paramLabelW, rowH);
        mkEdit(edit, rightX + paramLabelW + S(8), yR, paramEditW, rowH);
        yR += rowH + S(6);
    };
    mkParamRow(lblStreamerGlow_, edtStreamerGlow_);
    mkParamRow(lblStreamerCore_, edtStreamerCore_);
    mkParamRow(lblStreamerHead_, edtStreamerHead_);
    mkParamRow(lblElectricFork_, edtElectricFork_);
    mkParamRow(lblElectricAmp_, edtElectricAmp_);
    mkParamRow(lblMeteorRate_, edtMeteorRate_);
    mkParamRow(lblMeteorSpeed_, edtMeteorSpeed_);

    // Footer buttons (match SettingsWnd layout)
    CRect footer = RcFooter();
    const int btnW = S(120);
    const int btnH = S(30);
    const int by = footer.top + S(10);
    const int padF = S(14);

    btnReset_.Create(L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(footer.left + padF, by, footer.left + padF + btnW, by + btnH), this, kIdReset);
    btnClose_.Create(L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(footer.CenterPoint().x - btnW / 2, by, footer.CenterPoint().x + btnW / 2, by + btnH), this, kIdClose);
    btnApply_.Create(L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(footer.right - padF - btnW, by, footer.right - padF, by + btnH), this, kIdApply);
    if (font_.GetSafeHandle()) {
        btnReset_.SetFont(&font_);
        btnClose_.SetFont(&font_);
        btnApply_.SetFont(&font_);
    }

    ApplyLanguageToControls();
    LoadFromBackend();
    return 0;
}

void CTrailTuningWnd::OnClose() {
    DestroyWindow();
}

void CTrailTuningWnd::OnDestroy() {
    CWnd::OnDestroy();
}

void CTrailTuningWnd::PostNcDestroy() {
    delete this;
}

void CTrailTuningWnd::ApplyLanguageToControls() {
    const bool zh = IsZh();

    lblPreset_.SetWindowTextW(zh ? L"\u9884\u8bbe (Preset)" : L"Preset");
    lblProfiles_.SetWindowTextW(zh ? L"\u5386\u53F2\u7A97\u53E3 (Profiles)" : L"Profiles");
    lblParams_.SetWindowTextW(zh ? L"\u53C2\u6570 (Params)" : L"Params");

    lblType_.SetWindowTextW(zh ? L"\u7C7B\u578B" : L"Type");
    lblDuration_.SetWindowTextW(zh ? L"\u65F6\u957F(ms)" : L"Duration(ms)");
    lblMaxPoints_.SetWindowTextW(zh ? L"\u70B9\u6570" : L"Max");

    lblLine_.SetWindowTextW(zh ? L"\u666E\u901A\u7EBF\u6761" : L"Line");
    lblStreamer_.SetWindowTextW(zh ? L"\u9713\u8679\u6D41\u5149" : L"Streamer");
    lblElectric_.SetWindowTextW(zh ? L"\u8D5B\u535A\u7535\u5F27" : L"Electric");
    lblMeteor_.SetWindowTextW(zh ? L"\u7D62\u4E3D\u6D41\u661F" : L"Meteor");
    lblTubes_.SetWindowTextW(zh ? L"\u79D1\u5E7B\u7BA1\u9053" : L"Tubes");

    lblStreamerGlow_.SetWindowTextW(zh ? L"Streamer glow \u7F29\u653E" : L"Streamer glow scale");
    lblStreamerCore_.SetWindowTextW(zh ? L"Streamer \u5185\u6838\u7F29\u653E" : L"Streamer core scale");
    lblStreamerHead_.SetWindowTextW(zh ? L"Streamer \u5934\u90E8\u6307\u6570" : L"Streamer head power");
    lblElectricFork_.SetWindowTextW(zh ? L"Electric \u5206\u53C9\u6982\u7387" : L"Electric fork chance");
    lblElectricAmp_.SetWindowTextW(zh ? L"Electric \u5E45\u5EA6\u7F29\u653E" : L"Electric amplitude scale");
    lblMeteorRate_.SetWindowTextW(zh ? L"Meteor \u706B\u82B1\u91CF" : L"Meteor spark rate");
    lblMeteorSpeed_.SetWindowTextW(zh ? L"Meteor \u706B\u82B1\u901F\u5EA6" : L"Meteor spark speed");

    const SettingsText& t = zh ? TextZh() : TextEn();
    btnApply_.SetWindowTextW(t.btnApply);
    btnReset_.SetWindowTextW(t.btnReset);
    btnClose_.SetWindowTextW(t.btnClose);
}


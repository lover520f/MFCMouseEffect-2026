#include "pch.h"
#include "TrailTuningWnd.h"

#include "Settings/SettingsBackend.h"
#include <cmath>

namespace {

static std::string ToLowerAscii(std::string s) {
    for (char& c : s) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return s;
}

static TrailTuningModel MakePresetModel(const std::string& preset) {
    TrailTuningModel m;
    std::string p = ToLowerAscii(preset);

    if (p == "snappy") {
        m.style = "snappy";
        m.profiles.line = {220, 24};
        m.profiles.streamer = {320, 36};
        m.profiles.electric = {220, 18};
        m.profiles.meteor = {380, 44};
        m.profiles.tubes = {280, 32};

        m.params.electricForkChance = 0.14f;
        m.params.electricAmplitudeScale = 1.15f;
        m.params.streamerGlowWidthScale = 1.6f;
        m.params.meteorSparkRateScale = 1.1f;
        return m;
    }
    if (p == "long") {
        m.style = "long";
        m.profiles.line = {380, 44};
        m.profiles.streamer = {520, 64};
        m.profiles.electric = {360, 36};
        m.profiles.meteor = {720, 90};
        m.profiles.tubes = {520, 56};

        m.params.streamerGlowWidthScale = 2.1f;
        m.params.streamerHeadPower = 1.4f;
        m.params.electricForkChance = 0.08f;
        m.params.meteorSparkRateScale = 1.25f;
        return m;
    }
    if (p == "cinematic") {
        m.style = "cinematic";
        m.profiles.line = {460, 58};
        m.profiles.streamer = {640, 88};
        m.profiles.electric = {380, 44};
        m.profiles.meteor = {900, 120};
        m.profiles.tubes = {620, 72};

        m.params.streamerGlowWidthScale = 2.4f;
        m.params.streamerHeadPower = 1.25f;
        m.params.electricForkChance = 0.06f;
        m.params.electricAmplitudeScale = 1.05f;
        m.params.meteorSparkRateScale = 1.45f;
        m.params.meteorSparkSpeedScale = 1.15f;
        return m;
    }

    m.style = "default";
    return m;
}

} // namespace

BEGIN_MESSAGE_MAP(CTrailTuningWnd, CWnd)
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_WM_DESTROY()
    ON_BN_CLICKED(kIdApply, &CTrailTuningWnd::OnCommandApply)
    ON_BN_CLICKED(kIdReset, &CTrailTuningWnd::OnCommandReset)
    ON_CBN_SELCHANGE(kIdPreset, &CTrailTuningWnd::OnSelChangePreset)
END_MESSAGE_MAP()

bool CTrailTuningWnd::CreateAndShow(CWnd* parent, ISettingsBackend* backend) {
    backend_ = backend;
    if (!backend_) return false;

    const CString cls = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LoadCursor(nullptr, IDC_ARROW));
    DWORD style = WS_POPUP | WS_VISIBLE | WS_CAPTION | WS_SYSMENU;
    DWORD ex = WS_EX_APPWINDOW;

    CRect rc(0, 0, 620, 520);
    CPoint pt(0, 0);
    if (parent && ::IsWindow(parent->GetSafeHwnd())) {
        CRect prc;
        parent->GetWindowRect(&prc);
        pt = prc.CenterPoint();
    } else {
        pt.x = GetSystemMetrics(SM_CXSCREEN) / 2;
        pt.y = GetSystemMetrics(SM_CYSCREEN) / 2;
    }
    rc.OffsetRect(pt.x - rc.Width() / 2, pt.y - rc.Height() / 2);

    if (!CreateEx(ex, cls, L"\u62D6\u5C3E\u8C03\u53C2 / Trail Tuning", style, rc, parent, 0)) {
        return false;
    }
    ShowWindow(SW_SHOW);
    UpdateWindow();
    return true;
}

int CTrailTuningWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CWnd::OnCreate(lpCreateStruct) == -1) return -1;

    NONCLIENTMETRICS ncm{};
    ncm.cbSize = sizeof(ncm);
    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0)) {
        font_.CreateFontIndirect(&ncm.lfMessageFont);
    }

    const int pad = S(12);
    CRect rc;
    GetClientRect(&rc);

    int x = pad;
    int y = pad;
    int w = rc.Width() - pad * 2;

    auto mkStatic = [&](CStatic& s, const wchar_t* text, int xx, int yy, int ww, int hh) {
        s.Create(text, WS_CHILD | WS_VISIBLE | SS_LEFT | SS_CENTERIMAGE, CRect(xx, yy, xx + ww, yy + hh), this);
        if (font_.GetSafeHandle()) s.SetFont(&font_);
    };
    auto mkEdit = [&](CEdit& e, int xx, int yy, int ww, int hh) {
        e.Create(WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, CRect(xx, yy, xx + ww, yy + hh), this, 0);
        if (font_.GetSafeHandle()) e.SetFont(&font_);
    };

    mkStatic(lblPreset_, L"\u9884\u8bbe (Preset):", x, y, S(120), S(26));
    cmbPreset_.Create(WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP, CRect(x + S(130), y, x + S(320), y + S(220)), this, kIdPreset);
    if (font_.GetSafeHandle()) cmbPreset_.SetFont(&font_);
    cmbPreset_.AddString(L"Default");
    cmbPreset_.AddString(L"Snappy");
    cmbPreset_.AddString(L"Long");
    cmbPreset_.AddString(L"Cinematic");
    cmbPreset_.AddString(L"Custom");

    y += S(36);

    mkStatic(lblType_, L"\u7C7B\u578B (Type)", x, y, S(160), S(24));
    mkStatic(lblDuration_, L"\u65F6\u957Fms (Duration)", x + S(180), y, S(140), S(24));
    mkStatic(lblMaxPoints_, L"\u70B9\u6570 (Max points)", x + S(320), y, S(160), S(24));
    y += S(28);

    auto mkRow = [&](CStatic& label, const wchar_t* text, ProfileControls& pc) {
        mkStatic(label, text, x, y, S(150), S(26));
        mkEdit(pc.duration, x + S(180), y, S(110), S(26));
        mkEdit(pc.maxPoints, x + S(320), y, S(110), S(26));
        y += S(32);
    };

    mkRow(lblLine_, L"\u666E\u901A\u7EBF\u6761 (Line)", line_);
    mkRow(lblStreamer_, L"\u9713\u8679\u6D41\u5149 (Streamer)", streamer_);
    mkRow(lblElectric_, L"\u8D5B\u535A\u7535\u5F27 (Electric)", electric_);
    mkRow(lblMeteor_, L"\u7D62\u4E3D\u6D41\u661F (Meteor)", meteor_);
    mkRow(lblTubes_, L"\u79D1\u5E7B\u7BA1\u9053 (Tubes/Sci-Fi)", tubes_);

    y += S(6);
    mkStatic(lblParams_, L"Renderer \u53C2\u6570 (Params)", x, y, w, S(24));
    y += S(28);

    auto mkParamRow = [&](CStatic& label, const wchar_t* text, CEdit& edit) {
        mkStatic(label, text, x, y, S(240), S(26));
        mkEdit(edit, x + S(260), y, S(120), S(26));
        y += S(32);
    };

    mkParamRow(lblStreamerGlow_, L"Streamer glow \u7F29\u653E (glow_width_scale)", edtStreamerGlow_);
    mkParamRow(lblStreamerHead_, L"Streamer \u5934\u90E8\u6307\u6570 (head_power)", edtStreamerHead_);
    mkParamRow(lblElectricFork_, L"Electric \u5206\u53C9\u6982\u7387 (fork_chance 0-0.5)", edtElectricFork_);
    mkParamRow(lblElectricAmp_, L"Electric \u5E45\u5EA6\u7F29\u653E (amplitude_scale)", edtElectricAmp_);
    mkParamRow(lblMeteorRate_, L"Meteor \u706B\u82B1\u91CF (spark_rate_scale)", edtMeteorRate_);
    mkParamRow(lblMeteorSpeed_, L"Meteor \u706B\u82B1\u901F\u5EA6 (spark_speed_scale)", edtMeteorSpeed_);

    // Buttons
    int btnW = S(120);
    int btnH = S(30);
    int by = rc.bottom - pad - btnH;
    btnReset_.Create(L"\u6062\u590D\u9ED8\u8BA4 (Reset)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(x, by, x + btnW, by + btnH), this, kIdReset);
    btnApply_.Create(L"\u5E94\u7528 (Apply)", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(rc.right - pad - btnW, by, rc.right - pad, by + btnH), this, kIdApply);
    if (font_.GetSafeHandle()) {
        btnReset_.SetFont(&font_);
        btnApply_.SetFont(&font_);
    }

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

void CTrailTuningWnd::OnCommandApply() {
    if (!backend_) return;
    if (!SyncModelFromControls()) return;
    backend_->ApplyTrailTuning(model_);
}

void CTrailTuningWnd::OnCommandReset() {
    ApplyPreset("default");
    SyncControlsFromModel();
}

void CTrailTuningWnd::OnSelChangePreset() {
    int sel = cmbPreset_.GetCurSel();
    CString s;
    if (sel >= 0) cmbPreset_.GetLBText(sel, s);
    std::string preset;
    preset.reserve((size_t)s.GetLength());
    for (int i = 0; i < s.GetLength(); ++i) {
        wchar_t ch = s[i];
        if (ch >= L'A' && ch <= L'Z') ch = (wchar_t)(ch - L'A' + L'a');
        preset.push_back((ch >= 0 && ch <= 0x7F) ? (char)ch : '?');
    }
    ApplyPreset(preset);
    SyncControlsFromModel();
}

int CTrailTuningWnd::Dpi() const {
    HDC hdc = ::GetDC(nullptr);
    int dpi = GetDeviceCaps(hdc, LOGPIXELSX);
    ::ReleaseDC(nullptr, hdc);
    return dpi > 0 ? dpi : 96;
}

int CTrailTuningWnd::S(int px) const {
    return MulDiv(px, Dpi(), 96);
}

int CTrailTuningWnd::ClampInt(int v, int lo, int hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

float CTrailTuningWnd::ClampFloat(float v, float lo, float hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

int CTrailTuningWnd::ParseInt(const CString& s, int fallback) {
    int v = fallback;
    if (swscanf_s(s, L"%d", &v) == 1) return v;
    return fallback;
}

float CTrailTuningWnd::ParseFloat(const CString& s, float fallback) {
    float v = fallback;
    if (swscanf_s(s, L"%f", &v) == 1) return v;
    return fallback;
}

void CTrailTuningWnd::LoadFromBackend() {
    if (!backend_) return;
    model_ = backend_->LoadTrailTuning();
    SyncControlsFromModel();

    std::string p = ToLowerAscii(model_.style);
    int sel = 0;
    if (p == "snappy") sel = 1;
    else if (p == "long") sel = 2;
    else if (p == "cinematic") sel = 3;
    else if (p == "custom") sel = 4;
    cmbPreset_.SetCurSel(sel);
}

void CTrailTuningWnd::ApplyPreset(const std::string& preset) {
    if (ToLowerAscii(preset) == "custom") {
        model_.style = "custom";
        return;
    }
    if (ToLowerAscii(preset) == "default") {
        model_ = TrailTuningModel{};
        model_.style = "default";
        return;
    }
    model_ = MakePresetModel(preset);
}

void CTrailTuningWnd::SyncControlsFromModel() {
    auto setProfile = [&](ProfileControls& pc, const TrailHistoryProfileModel& p) {
        CString a, b;
        a.Format(L"%d", p.durationMs);
        b.Format(L"%d", p.maxPoints);
        pc.duration.SetWindowTextW(a);
        pc.maxPoints.SetWindowTextW(b);
    };
    setProfile(line_, model_.profiles.line);
    setProfile(streamer_, model_.profiles.streamer);
    setProfile(electric_, model_.profiles.electric);
    setProfile(meteor_, model_.profiles.meteor);
    setProfile(tubes_, model_.profiles.tubes);

    auto setF = [&](CEdit& e, float v) {
        CString s;
        s.Format(L"%.3f", v);
        e.SetWindowTextW(s);
    };
    setF(edtStreamerGlow_, model_.params.streamerGlowWidthScale);
    setF(edtStreamerHead_, model_.params.streamerHeadPower);
    setF(edtElectricFork_, model_.params.electricForkChance);
    setF(edtElectricAmp_, model_.params.electricAmplitudeScale);
    setF(edtMeteorRate_, model_.params.meteorSparkRateScale);
    setF(edtMeteorSpeed_, model_.params.meteorSparkSpeedScale);
}

bool CTrailTuningWnd::SyncModelFromControls() {
    auto getText = [](CWnd& w) -> CString {
        CString s;
        w.GetWindowTextW(s);
        return s;
    };
    auto readProfile = [&](ProfileControls& pc, TrailHistoryProfileModel& p) {
        p.durationMs = ClampInt(ParseInt(getText(pc.duration), p.durationMs), 80, 2000);
        p.maxPoints = ClampInt(ParseInt(getText(pc.maxPoints), p.maxPoints), 2, 240);
    };

    readProfile(line_, model_.profiles.line);
    readProfile(streamer_, model_.profiles.streamer);
    readProfile(electric_, model_.profiles.electric);
    readProfile(meteor_, model_.profiles.meteor);
    readProfile(tubes_, model_.profiles.tubes);

    model_.params.streamerGlowWidthScale = ClampFloat(ParseFloat(getText(edtStreamerGlow_), model_.params.streamerGlowWidthScale), 0.5f, 4.0f);
    model_.params.streamerHeadPower = ClampFloat(ParseFloat(getText(edtStreamerHead_), model_.params.streamerHeadPower), 0.8f, 3.0f);
    model_.params.electricForkChance = ClampFloat(ParseFloat(getText(edtElectricFork_), model_.params.electricForkChance), 0.0f, 0.5f);
    model_.params.electricAmplitudeScale = ClampFloat(ParseFloat(getText(edtElectricAmp_), model_.params.electricAmplitudeScale), 0.2f, 3.0f);
    model_.params.meteorSparkRateScale = ClampFloat(ParseFloat(getText(edtMeteorRate_), model_.params.meteorSparkRateScale), 0.2f, 4.0f);
    model_.params.meteorSparkSpeedScale = ClampFloat(ParseFloat(getText(edtMeteorSpeed_), model_.params.meteorSparkSpeedScale), 0.2f, 4.0f);

    // Style: if values deviate from the selected preset, mark as custom.
    int sel = cmbPreset_.GetCurSel();
    if (sel < 0) sel = 4;
    if (sel == 0) model_.style = "default";
    else if (sel == 1) model_.style = "snappy";
    else if (sel == 2) model_.style = "long";
    else if (sel == 3) model_.style = "cinematic";
    else model_.style = "custom";

    auto sameI = [](int a, int b) { return a == b; };
    auto sameF = [](float a, float b) { return std::fabs(a - b) < 0.0005f; };
    auto sameP = [&](const TrailHistoryProfileModel& a, const TrailHistoryProfileModel& b) {
        return sameI(a.durationMs, b.durationMs) && sameI(a.maxPoints, b.maxPoints);
    };

    if (model_.style != "custom") {
        TrailTuningModel preset = MakePresetModel(model_.style);
        bool same =
            sameP(model_.profiles.line, preset.profiles.line) &&
            sameP(model_.profiles.streamer, preset.profiles.streamer) &&
            sameP(model_.profiles.electric, preset.profiles.electric) &&
            sameP(model_.profiles.meteor, preset.profiles.meteor) &&
            sameP(model_.profiles.tubes, preset.profiles.tubes) &&
            sameF(model_.params.streamerGlowWidthScale, preset.params.streamerGlowWidthScale) &&
            sameF(model_.params.streamerCoreWidthScale, preset.params.streamerCoreWidthScale) &&
            sameF(model_.params.streamerHeadPower, preset.params.streamerHeadPower) &&
            sameF(model_.params.electricAmplitudeScale, preset.params.electricAmplitudeScale) &&
            sameF(model_.params.electricForkChance, preset.params.electricForkChance) &&
            sameF(model_.params.meteorSparkRateScale, preset.params.meteorSparkRateScale) &&
            sameF(model_.params.meteorSparkSpeedScale, preset.params.meteorSparkSpeedScale);
        if (!same) model_.style = "custom";
    }
    return true;
}

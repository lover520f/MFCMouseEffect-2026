#include "pch.h"
#include "SettingsWnd.h"

#include "MFCMouseEffect.h"
#include "MouseFx/AppController.h"

namespace {

struct UiUpdateGuard {
    explicit UiUpdateGuard(int& depth) : depth_(depth) { ++depth_; }
    ~UiUpdateGuard() { --depth_; }
    UiUpdateGuard(const UiUpdateGuard&) = delete;
    UiUpdateGuard& operator=(const UiUpdateGuard&) = delete;

private:
    int& depth_;
};

struct Option {
    const wchar_t* display;
    const char* value;
};

static CString W(const wchar_t* s) {
    return CString(s);
}

static const Option kLangOptions[] = {
    {L"\u4E2D\u6587", "zh-CN"},
    {L"English", "en-US"},
};

static const Option kThemeOptionsZh[] = {
    {L"\u9724\u8679", "neon"},
    {L"\u79D1\u5E7B", "scifi"},
    {L"\u6781\u7B80", "minimal"},
    {L"\u6E38\u620F\u611F", "game"},
};

static const Option kThemeOptionsEn[] = {
    {L"Neon", "neon"},
    {L"Sci-Fi", "scifi"},
    {L"Minimal", "minimal"},
    {L"Game", "game"},
};

static const Option kClickOptionsZh[] = {
    {L"\u6C34\u6CE2\u7EB9", "ripple"},
    {L"\u661F\u661F", "star"},
    {L"\u98D8\u6D6E\u6587\u5B57", "text"},
    {L"\u65E0", "none"},
};

static const Option kClickOptionsEn[] = {
    {L"Ripple", "ripple"},
    {L"Star", "star"},
    {L"Text", "text"},
    {L"None", "none"},
};

static const Option kTrailOptionsZh[] = {
    {L"\u5F69\u8679\u7C92\u5B50", "particle"},
    {L"\u666E\u901A\u7EBF\u6761", "line"},
    {L"\u65E0", "none"},
};

static const Option kTrailOptionsEn[] = {
    {L"Particle", "particle"},
    {L"Line", "line"},
    {L"None", "none"},
};

static const Option kScrollOptionsZh[] = {
    {L"\u65B9\u5411\u6307\u793A", "arrow"},
    {L"\u65E0", "none"},
};

static const Option kScrollOptionsEn[] = {
    {L"Arrow", "arrow"},
    {L"None", "none"},
};

static const Option kHoldOptionsZh[] = {
    {L"\u84C4\u529B", "charge"},
    {L"\u65E0", "none"},
};

static const Option kHoldOptionsEn[] = {
    {L"Charge", "charge"},
    {L"None", "none"},
};

static const Option kHoverOptionsZh[] = {
    {L"\u547C\u5438\u706F", "glow"},
    {L"\u65E0", "none"},
};

static const Option kHoverOptionsEn[] = {
    {L"Glow", "glow"},
    {L"None", "none"},
};

static const Option* FindByValue(const Option* opts, size_t n, const std::string& value) {
    for (size_t i = 0; i < n; ++i) {
        if (value == opts[i].value) return &opts[i];
    }
    return nullptr;
}

static const Option* FindByDisplay(const Option* opts, size_t n, const CString& display) {
    for (size_t i = 0; i < n; ++i) {
        if (display == opts[i].display) return &opts[i];
    }
    return nullptr;
}

static void AddOptions(CMFCPropertyGridProperty& p, const Option* opts, size_t n) {
    p.RemoveAllOptions();
    for (size_t i = 0; i < n; ++i) {
        p.AddOption(opts[i].display);
    }
    p.AllowEdit(FALSE);
}

static void ApplyEffectCmd(mousefx::AppController* fx, const char* category, const std::string& type) {
    if (!fx) return;
    if (type == "none") {
        std::string cmd = std::string("{\"cmd\":\"clear_effect\",\"category\":\"") + category + "\"}";
        fx->HandleCommand(cmd);
        return;
    }
    std::string cmd = std::string("{\"cmd\":\"set_effect\",\"category\":\"") + category +
        "\",\"type\":\"" + type + "\"}";
    fx->HandleCommand(cmd);
}

} // namespace

BEGIN_MESSAGE_MAP(CSettingsWnd, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_BN_CLICKED(10001, &CSettingsWnd::OnApply)
    ON_BN_CLICKED(10002, &CSettingsWnd::OnCloseBtn)
    ON_REGISTERED_MESSAGE(AFX_WM_PROPERTY_CHANGED, &CSettingsWnd::OnPropertyChanged)
END_MESSAGE_MAP()

bool CSettingsWnd::CreateAndShow(CWnd* parent) {
    if (GetSafeHwnd()) {
        ShowWindow(SW_SHOW);
        SetForegroundWindow();
        SyncFromApp();
        return true;
    }

    const CString cls = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);

    if (!CreateEx(WS_EX_APPWINDOW, cls, L"MFCMouseEffect", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
            CW_USEDEFAULT, CW_USEDEFAULT, 560, 520, parent ? parent->GetSafeHwnd() : nullptr, nullptr)) {
        return false;
    }

    ShowWindow(SW_SHOW);
    UpdateWindow();
    return true;
}

int CSettingsWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) {
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1) return -1;

    NONCLIENTMETRICS ncm{};
    ncm.cbSize = sizeof(ncm);
    if (SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(ncm), &ncm, 0)) {
        font_.CreateFontIndirect(&ncm.lfMessageFont);
    }

    CreateControls();
    SyncFromApp();
    return 0;
}

void CSettingsWnd::OnClose() {
    DestroyWindow();
}

void CSettingsWnd::PostNcDestroy() {
    auto* app = dynamic_cast<CMFCMouseEffectApp*>(AfxGetApp());
    if (app) {
        app->NotifySettingsWndDestroyed(this);
    }
    delete this;
}

void CSettingsWnd::OnApply() {
    ApplyToApp();
}

void CSettingsWnd::OnCloseBtn() {
    OnClose();
}

LRESULT CSettingsWnd::OnPropertyChanged(WPARAM, LPARAM lParam) {
    if (updatingUiDepth_ > 0) return 0;

    auto* pProp = reinterpret_cast<CMFCPropertyGridProperty*>(lParam);
    if (!pProp) return 0;

    // If language changes, rebuild the grid in the new language.
    if (pProp == propLang_) {
        const std::string lang = GetPropOptionValueAscii(*propLang_);
        if (!lang.empty()) {
            PersistUiLanguage(lang);
            currentLang_ = lang;
            ApplyLanguageToControls();
        }
    }

    ApplyToApp();
    return 0;
}

void CSettingsWnd::CreateControls() {
    if (controlsCreated_) return;
    controlsCreated_ = true;

    CRect rc;
    GetClientRect(&rc);

    const int pad = 14;
    const int btnH = 30;
    const int btnW = 96;

    // Property grid
    CRect rcGrid = rc;
    rcGrid.DeflateRect(pad, pad, pad, pad + btnH + 10);

    grid_.Create(WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER, rcGrid, this, 12001);
    grid_.EnableHeaderCtrl(FALSE);
    grid_.EnableDescriptionArea(TRUE);
    grid_.SetVSDotNetLook(TRUE);
    grid_.SetGroupNameFullWidth(TRUE);
    if (font_.GetSafeHandle()) {
        grid_.SetFont(&font_);
    }

    // Buttons
    const int y = rc.bottom - pad - btnH;
    btnApply_.Create(L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(rc.right - pad - btnW * 2 - 10, y, rc.right - pad - btnW - 10, y + btnH), this, 10001);
    btnClose_.Create(L"", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, CRect(rc.right - pad - btnW, y, rc.right - pad, y + btnH), this, 10002);
    if (font_.GetSafeHandle()) {
        btnApply_.SetFont(&font_);
        btnClose_.SetFont(&font_);
    }

    ApplyLanguageToControls();
}

bool CSettingsWnd::IsZh() const {
    return currentLang_.empty() || currentLang_ == "zh-CN";
}

void CSettingsWnd::RebuildPropertyGrid() {
    UiUpdateGuard guard(updatingUiDepth_);

    grid_.RemoveAll();

    const bool zh = IsZh();

    // --- General ---
    CMFCPropertyGridProperty* grpGeneral = new CMFCPropertyGridProperty(zh ? L"\u4E00\u822C" : L"General");

    propLang_ = new CMFCPropertyGridProperty(zh ? L"\u8BED\u8A00" : L"Language", W(L""), zh ? L"\u8BBE\u7F6E\u7A97\u53E3\u8BED\u8A00" : L"Settings window language");
    AddOptions(*propLang_, kLangOptions, _countof(kLangOptions));
    grpGeneral->AddSubItem(propLang_);

    propTheme_ = new CMFCPropertyGridProperty(zh ? L"\u4E3B\u9898" : L"Theme", W(L""), zh ? L"\u7279\u6548\u914D\u8272\u4E0E\u98CE\u683C" : L"Effect palette and style");
    AddOptions(*propTheme_, zh ? kThemeOptionsZh : kThemeOptionsEn, zh ? _countof(kThemeOptionsZh) : _countof(kThemeOptionsEn));
    grpGeneral->AddSubItem(propTheme_);

    // --- Effects ---
    CMFCPropertyGridProperty* grpFx = new CMFCPropertyGridProperty(zh ? L"\u7279\u6548" : L"Effects");

    propClick_ = new CMFCPropertyGridProperty(zh ? L"\u70B9\u51FB" : L"Click", W(L""));
    AddOptions(*propClick_, zh ? kClickOptionsZh : kClickOptionsEn, zh ? _countof(kClickOptionsZh) : _countof(kClickOptionsEn));
    grpFx->AddSubItem(propClick_);

    propTrail_ = new CMFCPropertyGridProperty(zh ? L"\u62D6\u5C3E" : L"Trail", W(L""));
    AddOptions(*propTrail_, zh ? kTrailOptionsZh : kTrailOptionsEn, zh ? _countof(kTrailOptionsZh) : _countof(kTrailOptionsEn));
    grpFx->AddSubItem(propTrail_);

    propScroll_ = new CMFCPropertyGridProperty(zh ? L"\u6EDA\u8F6E" : L"Scroll", W(L""));
    AddOptions(*propScroll_, zh ? kScrollOptionsZh : kScrollOptionsEn, zh ? _countof(kScrollOptionsZh) : _countof(kScrollOptionsEn));
    grpFx->AddSubItem(propScroll_);

    propHold_ = new CMFCPropertyGridProperty(zh ? L"\u957F\u6309" : L"Hold", W(L""));
    AddOptions(*propHold_, zh ? kHoldOptionsZh : kHoldOptionsEn, zh ? _countof(kHoldOptionsZh) : _countof(kHoldOptionsEn));
    grpFx->AddSubItem(propHold_);

    propHover_ = new CMFCPropertyGridProperty(zh ? L"\u60AC\u505C" : L"Hover", W(L""));
    AddOptions(*propHover_, zh ? kHoverOptionsZh : kHoverOptionsEn, zh ? _countof(kHoverOptionsZh) : _countof(kHoverOptionsEn));
    grpFx->AddSubItem(propHover_);

    grid_.AddProperty(grpGeneral);
    grid_.AddProperty(grpFx);

    // Expand for discoverability.
    grpGeneral->Expand(TRUE);
    grpFx->Expand(TRUE);

    SyncToPropsFromConfig();
}

void CSettingsWnd::SyncToPropsFromConfig() {
    auto* app = dynamic_cast<CMFCMouseEffectApp*>(AfxGetApp());
    auto* fx = app ? app->mouseFx_.get() : nullptr;
    if (!fx) return;

    const auto& cfg = fx->Config();

    // Language
    {
        const std::string lang = cfg.uiLanguage.empty() ? std::string("zh-CN") : cfg.uiLanguage;
        currentLang_ = lang;
        const Option* o = FindByValue(kLangOptions, _countof(kLangOptions), lang);
        if (o) propLang_->SetValue((_variant_t)o->display);
    }

    // Theme
    {
        const Option* opts = IsZh() ? kThemeOptionsZh : kThemeOptionsEn;
        const size_t n = IsZh() ? _countof(kThemeOptionsZh) : _countof(kThemeOptionsEn);
        const Option* o = FindByValue(opts, n, cfg.theme);
        if (o) propTheme_->SetValue((_variant_t)o->display);
    }

    // Effects
    {
        const Option* opts = IsZh() ? kClickOptionsZh : kClickOptionsEn;
        const size_t n = IsZh() ? _countof(kClickOptionsZh) : _countof(kClickOptionsEn);
        const Option* o = FindByValue(opts, n, cfg.active.click);
        if (o) propClick_->SetValue((_variant_t)o->display);
    }
    {
        const Option* opts = IsZh() ? kTrailOptionsZh : kTrailOptionsEn;
        const size_t n = IsZh() ? _countof(kTrailOptionsZh) : _countof(kTrailOptionsEn);
        const Option* o = FindByValue(opts, n, cfg.active.trail);
        if (o) propTrail_->SetValue((_variant_t)o->display);
    }
    {
        const Option* opts = IsZh() ? kScrollOptionsZh : kScrollOptionsEn;
        const size_t n = IsZh() ? _countof(kScrollOptionsZh) : _countof(kScrollOptionsEn);
        const Option* o = FindByValue(opts, n, cfg.active.scroll);
        if (o) propScroll_->SetValue((_variant_t)o->display);
    }
    {
        const Option* opts = IsZh() ? kHoldOptionsZh : kHoldOptionsEn;
        const size_t n = IsZh() ? _countof(kHoldOptionsZh) : _countof(kHoldOptionsEn);
        const Option* o = FindByValue(opts, n, cfg.active.hold);
        if (o) propHold_->SetValue((_variant_t)o->display);
    }
    {
        const Option* opts = IsZh() ? kHoverOptionsZh : kHoverOptionsEn;
        const size_t n = IsZh() ? _countof(kHoverOptionsZh) : _countof(kHoverOptionsEn);
        const Option* o = FindByValue(opts, n, cfg.active.hover);
        if (o) propHover_->SetValue((_variant_t)o->display);
    }
}

std::string CSettingsWnd::GetPropOptionValueAscii(const CMFCPropertyGridProperty& p) const {
    const CString display = p.GetValue();

    if (&p == propLang_) {
        const Option* o = FindByDisplay(kLangOptions, _countof(kLangOptions), display);
        return o ? std::string(o->value) : std::string();
    }

    const bool zh = IsZh();

    if (&p == propTheme_) {
        const Option* o = FindByDisplay(zh ? kThemeOptionsZh : kThemeOptionsEn,
            zh ? _countof(kThemeOptionsZh) : _countof(kThemeOptionsEn), display);
        return o ? std::string(o->value) : std::string();
    }
    if (&p == propClick_) {
        const Option* o = FindByDisplay(zh ? kClickOptionsZh : kClickOptionsEn,
            zh ? _countof(kClickOptionsZh) : _countof(kClickOptionsEn), display);
        return o ? std::string(o->value) : std::string();
    }
    if (&p == propTrail_) {
        const Option* o = FindByDisplay(zh ? kTrailOptionsZh : kTrailOptionsEn,
            zh ? _countof(kTrailOptionsZh) : _countof(kTrailOptionsEn), display);
        return o ? std::string(o->value) : std::string();
    }
    if (&p == propScroll_) {
        const Option* o = FindByDisplay(zh ? kScrollOptionsZh : kScrollOptionsEn,
            zh ? _countof(kScrollOptionsZh) : _countof(kScrollOptionsEn), display);
        return o ? std::string(o->value) : std::string();
    }
    if (&p == propHold_) {
        const Option* o = FindByDisplay(zh ? kHoldOptionsZh : kHoldOptionsEn,
            zh ? _countof(kHoldOptionsZh) : _countof(kHoldOptionsEn), display);
        return o ? std::string(o->value) : std::string();
    }
    if (&p == propHover_) {
        const Option* o = FindByDisplay(zh ? kHoverOptionsZh : kHoverOptionsEn,
            zh ? _countof(kHoverOptionsZh) : _countof(kHoverOptionsEn), display);
        return o ? std::string(o->value) : std::string();
    }

    return {};
}

void CSettingsWnd::ApplyLanguageToControls() {
    UiUpdateGuard guard(updatingUiDepth_);

    // Update title/buttons.
    if (IsZh()) {
        SetWindowTextW(L"MFCMouseEffect \u8BBE\u7F6E");
        btnApply_.SetWindowTextW(L"\u5E94\u7528");
        btnClose_.SetWindowTextW(L"\u5173\u95ED");
    } else {
        SetWindowTextW(L"MFCMouseEffect Settings");
        btnApply_.SetWindowTextW(L"Apply");
        btnClose_.SetWindowTextW(L"Close");
    }

    RebuildPropertyGrid();
}

void CSettingsWnd::PersistUiLanguage(const std::string& lang) {
    auto* app = dynamic_cast<CMFCMouseEffectApp*>(AfxGetApp());
    auto* fx = app ? app->mouseFx_.get() : nullptr;
    if (!fx) return;
    fx->SetUiLanguage(lang);
}

void CSettingsWnd::SyncFromApp() {
    auto* app = dynamic_cast<CMFCMouseEffectApp*>(AfxGetApp());
    auto* fx = app ? app->mouseFx_.get() : nullptr;
    if (!fx) return;

    const auto& cfg = fx->Config();
    currentLang_ = cfg.uiLanguage.empty() ? std::string("zh-CN") : cfg.uiLanguage;

    ApplyLanguageToControls();
}

void CSettingsWnd::ApplyToApp() {
    auto* app = dynamic_cast<CMFCMouseEffectApp*>(AfxGetApp());
    auto* fx = app ? app->mouseFx_.get() : nullptr;
    if (!fx) return;

    const std::string lang = GetPropOptionValueAscii(*propLang_);
    if (!lang.empty() && lang != currentLang_) {
        currentLang_ = lang;
        fx->SetUiLanguage(lang);
    }

    const std::string theme = GetPropOptionValueAscii(*propTheme_);
    if (!theme.empty()) {
        fx->SetTheme(theme);
    }

    ApplyEffectCmd(fx, "click", GetPropOptionValueAscii(*propClick_));
    ApplyEffectCmd(fx, "trail", GetPropOptionValueAscii(*propTrail_));
    ApplyEffectCmd(fx, "scroll", GetPropOptionValueAscii(*propScroll_));
    ApplyEffectCmd(fx, "hold", GetPropOptionValueAscii(*propHold_));
    ApplyEffectCmd(fx, "hover", GetPropOptionValueAscii(*propHover_));
}


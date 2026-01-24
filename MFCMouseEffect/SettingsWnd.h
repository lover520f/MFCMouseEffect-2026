#pragma once

#include <afxwin.h>
#include <afxcontrolbars.h>
#include <string>

class CSettingsWnd final : public CFrameWnd
{
public:
    CSettingsWnd() = default;
    ~CSettingsWnd() override = default;

    CSettingsWnd(const CSettingsWnd&) = delete;
    CSettingsWnd& operator=(const CSettingsWnd&) = delete;

    bool CreateAndShow(CWnd* parent);
    void SyncFromApp();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnClose();
    afx_msg void OnApply();
    afx_msg void OnCloseBtn();
    afx_msg LRESULT OnPropertyChanged(WPARAM wParam, LPARAM lParam);

    void PostNcDestroy() override;

    DECLARE_MESSAGE_MAP()

private:
    void CreateControls();
    void ApplyToApp();
    void ApplyLanguageToControls();
    bool IsZh() const;
    void RebuildPropertyGrid();
    std::string GetPropOptionValueAscii(const CMFCPropertyGridProperty& p) const;
    void SyncToPropsFromConfig();
    void PersistUiLanguage(const std::string& lang);

    CFont font_;

    CButton btnApply_;
    CButton btnClose_;

    CMFCPropertyGridCtrl grid_;
    CMFCPropertyGridProperty* propLang_ = nullptr;
    CMFCPropertyGridProperty* propTheme_ = nullptr;
    CMFCPropertyGridProperty* propClick_ = nullptr;
    CMFCPropertyGridProperty* propTrail_ = nullptr;
    CMFCPropertyGridProperty* propScroll_ = nullptr;
    CMFCPropertyGridProperty* propHold_ = nullptr;
    CMFCPropertyGridProperty* propHover_ = nullptr;

    bool controlsCreated_ = false;
    int updatingUiDepth_ = 0;
    std::string currentLang_ = "zh-CN";
};

#pragma once

#include <afxwin.h>
#include <memory>
#include <gdiplus.h>

#include "Settings/SettingsModel.h"
#include "Settings/SettingsOptions.h"

class ISettingsBackend;

class CSettingsWnd final : public CWnd
{
public:
    CSettingsWnd() = default;
    ~CSettingsWnd() override = default;

    CSettingsWnd(const CSettingsWnd&) = delete;
    CSettingsWnd& operator=(const CSettingsWnd&) = delete;

    bool CreateAndShow(CWnd* parent, std::unique_ptr<ISettingsBackend> backend);
    void SyncFromBackend();

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg void OnCommandApply();
    afx_msg void OnCommandClose();
    afx_msg void OnSelChange();
    afx_msg void OnClose();
    afx_msg void OnDestroy();

    DECLARE_MESSAGE_MAP()

private:
    struct Hit {
        enum Kind {
            None,
            Close,
        } kind = None;
        CRect rc;
    };

    void Apply();
    Hit HitTest(const CPoint& pt) const;

    bool IsZh() const;
    void ApplyLanguageToControls();
    void FillCombo(CComboBox& box, const SettingOption* opts, size_t count, const std::string& current);
    std::string GetComboValue(const CComboBox& box) const;
    void SetComboValue(CComboBox& box, const std::string& value);

    // Layout helpers
    int Dpi() const;
    int S(int px) const; // scale
    CRect Client() const;

    CRect RcHeader() const;
    CRect RcCloseBtn() const;

    CRect RcContent() const;
    CRect RcFooter() const;
    CRect RcApplyBtn() const;
    CRect RcCloseBtn2() const;

    // Drawing
    void DrawRoundRect(Gdiplus::Graphics& g, const CRect& rc, int radius, const Gdiplus::Color& fill);
    void DrawText(Gdiplus::Graphics& g, const wchar_t* text, const CRect& rc, int sizePx, bool bold, const Gdiplus::Color& c);

    std::unique_ptr<ISettingsBackend> backend_;
    SettingsModel model_;

    Hit::Kind hover_ = Hit::None;
    Hit::Kind down_ = Hit::None;

    CFont font_;
    CStatic lblLang_{};
    CStatic lblTheme_{};
    CStatic lblClick_{};
    CStatic lblTrail_{};
    CStatic lblScroll_{};
    CStatic lblHold_{};
    CStatic lblHover_{};

    CComboBox cmbLang_{};
    CComboBox cmbTheme_{};
    CComboBox cmbClick_{};
    CComboBox cmbTrail_{};
    CComboBox cmbScroll_{};
    CComboBox cmbHold_{};
    CComboBox cmbHover_{};

    CButton btnApply_{};
    CButton btnClose_{};

    bool updating_ = false;
};

#pragma once

#include <afxwin.h>
#include <memory>
#include <gdiplus.h>

#include "Settings/TrailTuningModel.h"

class ISettingsBackend;

class CTrailTuningWnd final : public CWnd {
public:
    CTrailTuningWnd() = default;
    ~CTrailTuningWnd() override = default;

    CTrailTuningWnd(const CTrailTuningWnd&) = delete;
    CTrailTuningWnd& operator=(const CTrailTuningWnd&) = delete;

    bool CreateAndShow(CWnd* parent, ISettingsBackend* backend);

protected:
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg void OnClose();
    afx_msg void OnDestroy();
    afx_msg void OnCommandApply();
    afx_msg void OnCommandReset();
    afx_msg void OnSelChangePreset();
    DECLARE_MESSAGE_MAP()

private:
    static constexpr UINT kIdPreset = 21000;
    static constexpr UINT kIdApply = 21001;
    static constexpr UINT kIdReset = 21002;
    static constexpr UINT kIdClose = 21003;

    struct Hit {
        enum Kind {
            None,
            Close,
        } kind = None;
        CRect rc;
    };

    struct ProfileControls {
        CEdit duration;
        CEdit maxPoints;
    };

    int Dpi() const;
    int S(int px) const;
    void PostNcDestroy() override;

    CRect Client() const;
    CRect RcHeader() const;
    CRect RcCloseBtn() const;
    CRect RcContent() const;
    CRect RcFooter() const;

    Hit HitTest(const CPoint& pt) const;
    void DrawRoundRect(Gdiplus::Graphics& g, const CRect& rc, int radius, const Gdiplus::Color& fill);
    void DrawText(Gdiplus::Graphics& g, const wchar_t* text, const CRect& rc, int sizePx, bool bold, const Gdiplus::Color& c, Gdiplus::StringAlignment align = Gdiplus::StringAlignmentCenter);

    bool IsZh() const;
    void ApplyLanguageToControls();

    void LoadFromBackend();
    void ApplyPreset(const std::string& preset);
    void SyncControlsFromModel();
    bool SyncModelFromControls();

    static int ParseInt(const CString& s, int fallback);
    static float ParseFloat(const CString& s, float fallback);
    static int ClampInt(int v, int lo, int hi);
    static float ClampFloat(float v, float lo, float hi);

    ISettingsBackend* backend_ = nullptr;
    TrailTuningModel model_{};
    std::string uiLanguage_ = "zh-CN";

    Hit::Kind hover_ = Hit::None;
    Hit::Kind down_ = Hit::None;

    CFont font_;
    CStatic lblPreset_{};
    CComboBox cmbPreset_{};

    // Profiles (left panel)
    CStatic lblProfiles_{};
    CStatic lblType_{};
    CStatic lblDuration_{};
    CStatic lblMaxPoints_{};

    CStatic lblLine_{};
    CStatic lblStreamer_{};
    CStatic lblElectric_{};
    CStatic lblMeteor_{};
    CStatic lblTubes_{};

    ProfileControls line_{};
    ProfileControls streamer_{};
    ProfileControls electric_{};
    ProfileControls meteor_{};
    ProfileControls tubes_{};

    // Params (right panel)
    CStatic lblParams_{};
    CStatic lblStreamerGlow_{};
    CStatic lblStreamerCore_{};
    CStatic lblStreamerHead_{};
    CStatic lblElectricFork_{};
    CStatic lblElectricAmp_{};
    CStatic lblMeteorRate_{};
    CStatic lblMeteorSpeed_{};

    CEdit edtStreamerGlow_{};
    CEdit edtStreamerCore_{};
    CEdit edtStreamerHead_{};
    CEdit edtElectricFork_{};
    CEdit edtElectricAmp_{};
    CEdit edtMeteorRate_{};
    CEdit edtMeteorSpeed_{};

    CButton btnApply_{};
    CButton btnReset_{};
    CButton btnClose_{};
};

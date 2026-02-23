#pragma once

#include <afxwin.h>
#include <string>
#include <wrl/client.h>
#include <d2d1.h>
#include <dwrite.h>

class EmojiPreviewWnd final : public CWnd {
public:
    EmojiPreviewWnd() = default;
    ~EmojiPreviewWnd() override = default;

    EmojiPreviewWnd(const EmojiPreviewWnd&) = delete;
    EmojiPreviewWnd& operator=(const EmojiPreviewWnd&) = delete;

    bool Create(CWnd* parent, const CRect& rc);
    void SetText(const std::wstring& text);
    void SetFontSizePx(float px);
    void SetTextColor(COLORREF color);
    void SetPaddingPx(int left, int top);
    void SetTargetRect(const CRect& rc);

protected:
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg LRESULT OnNcHitTest(CPoint point);
    DECLARE_MESSAGE_MAP()

private:
    void Render();
    bool EnsureD2DResources();
    void DestroyD2DResources();

    std::wstring text_;
    float fontSizePx_ = 14.0f;
    COLORREF textColor_ = RGB(0, 0, 0);
    int padLeftPx_ = 4;
    int padTopPx_ = 1;

    Microsoft::WRL::ComPtr<ID2D1Factory> d2dFactory_;
    Microsoft::WRL::ComPtr<IDWriteFactory> dwriteFactory_;
    Microsoft::WRL::ComPtr<ID2D1HwndRenderTarget> d2dTarget_;
    Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> d2dBrush_;
};

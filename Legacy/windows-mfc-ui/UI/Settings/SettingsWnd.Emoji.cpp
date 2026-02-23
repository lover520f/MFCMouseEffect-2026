#include "pch.h"
#include "SettingsWnd.h"

#include "Settings/EmojiUtils.h"

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

    for (size_t i = 0; i < text.size();) {
        const size_t runStart = i;
        size_t next = i;
        const uint32_t cp = settings::NextCodePointUtf16(text, &next);
        if (cp == 0) break;
        i = next;
        if (!settings::IsEmojiCodePoint(cp)) continue;

        size_t runEnd = i;
        while (runEnd < text.size()) {
            size_t probe = runEnd;
            const uint32_t cp2 = settings::NextCodePointUtf16(text, &probe);
            if (cp2 == 0) break;
            if (!settings::IsEmojiComponent(cp2)) break;
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


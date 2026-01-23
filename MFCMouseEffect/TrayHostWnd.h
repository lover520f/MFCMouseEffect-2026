#pragma once

#include <afxwin.h>
#include <shellapi.h>

// A hidden, non-UI host window for the system tray icon.
class CTrayHostWnd final : public CWnd
{
public:
	CTrayHostWnd() = default;
	~CTrayHostWnd() override = default;

	BOOL CreateHost(bool showTrayIcon = true);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	afx_msg LRESULT OnTrayNotify(WPARAM wp, LPARAM lp);
	afx_msg void OnTrayExit();
	DECLARE_MESSAGE_MAP()

private:
	static constexpr UINT kTrayMsg = WM_APP + 1;
	static constexpr UINT kTrayIconId = 1;
    static const int kCmdTrayExit = 1001;
    static const int kCmdTrayEffectRipple = 1002;
    static const int kCmdTrayEffectNone = 1003;
    static const int kCmdTrayEffectTrail = 1004;
    static const int kCmdTrayEffectIconStar = 1005;

	NOTIFYICONDATA m_trayIcon{};
	bool m_showTrayIcon{ true };
};

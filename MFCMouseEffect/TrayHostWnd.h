#pragma once

#include <afxwin.h>
#include <shellapi.h>

#include "TrayMenuCommands.h"

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

	NOTIFYICONDATA m_trayIcon{};
	bool m_showTrayIcon{ true };
};

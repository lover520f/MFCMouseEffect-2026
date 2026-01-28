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

	// Menu command IDs
	enum MenuCmd {
		kCmdTrayExit = 1001,
		kCmdTraySettings = 1002,
		kCmdStarRepo = 1003,
		// Click category
		kCmdClickRipple = 2001,
		kCmdClickStar = 2002,
		kCmdClickText = 2003,
		kCmdClickNone = 2004,
		// Trail category
		kCmdTrailLine = 3001,
		kCmdTrailParticle = 3002,
		kCmdTrailNone = 3003,
		// Hover category
		kCmdHoverGlow = 4001,
		kCmdHoverNone = 4002,
		// Scroll category
		kCmdScrollArrow = 5001,
		kCmdScrollNone = 5002,
		// Edge category
		kCmdEdgeNone = 6001,
		// Hold category
		kCmdHoldCharge = 7001,
		kCmdHoldLightning = 7002,
		kCmdHoldNone = 7003,
		// Theme
		// Theme
		kCmdThemeSciFi = 8001,
		kCmdThemeNeon = 8002,
		kCmdThemeMinimal = 8003,
		kCmdThemeGame = 8004,
	};

	NOTIFYICONDATA m_trayIcon{};
	bool m_showTrayIcon{ true };
};

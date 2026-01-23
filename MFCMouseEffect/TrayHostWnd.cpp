#include "pch.h"
#include "framework.h"

#include "TrayHostWnd.h"
#include "resource.h"
#include "MFCMouseEffect.h"
#include "MouseFx/AppController.h"

BEGIN_MESSAGE_MAP(CTrayHostWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_APP + 1, &CTrayHostWnd::OnTrayNotify)
	ON_COMMAND(32772, &CTrayHostWnd::OnTrayExit)
END_MESSAGE_MAP()

BOOL CTrayHostWnd::CreateHost(bool showTrayIcon)
{
	m_showTrayIcon = showTrayIcon;
	const CString className = AfxRegisterWndClass(0);
	// Create a hidden tool window (never shown) to receive tray callbacks.
	return CreateEx(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE,
		className,
		_T("MFCMouseEffectTrayHost"),
		WS_POPUP,
		0, 0, 0, 0,
		nullptr,
		0);
}

int CTrayHostWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	if (m_showTrayIcon)
	{
		HICON hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
		m_trayIcon.cbSize = sizeof(NOTIFYICONDATA);
		m_trayIcon.hWnd = GetSafeHwnd();
		m_trayIcon.uID = kTrayIconId;
		m_trayIcon.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		m_trayIcon.uCallbackMessage = kTrayMsg;
		m_trayIcon.hIcon = hIcon;
		lstrcpyn(m_trayIcon.szTip, _T("MFCMouseEffect（托盘常驻）- 右键退出"), _countof(m_trayIcon.szTip));

		Shell_NotifyIcon(NIM_ADD, &m_trayIcon);
	}
	return 0;
}

void CTrayHostWnd::OnDestroy()
{
	if (m_showTrayIcon)
	{
		Shell_NotifyIcon(NIM_DELETE, &m_trayIcon);
	}
	CWnd::OnDestroy();
}

void CTrayHostWnd::OnClose()
{
	DestroyWindow();
}

LRESULT CTrayHostWnd::OnTrayNotify(WPARAM wp, LPARAM lp)
{
	UNREFERENCED_PARAMETER(wp);

	switch (lp)
	{
	case WM_RBUTTONUP:
	{
		CMenu menu;
		menu.CreatePopupMenu();

		// Effect Submenu
		CMenu subMenu;
		subMenu.CreatePopupMenu();
		subMenu.AppendMenu(MF_STRING, kCmdTrayEffectRipple, _T("水波纹 (Ripple)"));
		subMenu.AppendMenu(MF_STRING, kCmdTrayEffectTrail, _T("拖尾 (Trail)"));
		subMenu.AppendMenu(MF_STRING, kCmdTrayEffectIconStar, _T("星星 (Star)"));
		subMenu.AppendMenu(MF_STRING, kCmdTrayEffectNone, _T("无特效 (None)"));

		menu.AppendMenu(MF_POPUP, (UINT_PTR)subMenu.m_hMenu, _T("切换特效"));
		menu.AppendMenu(MF_SEPARATOR);
		menu.AppendMenu(MF_STRING, kCmdTrayExit, _T("退出"));

		POINT pt{};
		GetCursorPos(&pt);
		SetForegroundWindow();
		
		// TrackPopupMenu returns the selected command ID
		int cmd = menu.TrackPopupMenu(TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, this);
		
		CMFCMouseEffectApp* app = dynamic_cast<CMFCMouseEffectApp*>(AfxGetApp());

		if (cmd == kCmdTrayExit) {
			PostMessage(WM_CLOSE);
		}
		else if (cmd == kCmdTrayEffectRipple) {
			if (app && app->mouseFx_) app->mouseFx_->HandleCommand("{\"cmd\":\"set_effect\",\"type\":\"ripple\"}");
		}
		else if (cmd == kCmdTrayEffectTrail) {
			if (app && app->mouseFx_) app->mouseFx_->HandleCommand("{\"cmd\":\"set_effect\",\"type\":\"trail\"}");
		}
		else if (cmd == kCmdTrayEffectIconStar) {
			if (app && app->mouseFx_) app->mouseFx_->HandleCommand("{\"cmd\":\"set_effect\",\"type\":\"icon_star\"}");
		}
		else if (cmd == kCmdTrayEffectNone) {
			if (app && app->mouseFx_) app->mouseFx_->HandleCommand("{\"cmd\":\"set_effect\",\"type\":\"none\"}");
		}

		PostMessage(WM_NULL);
		break;
	}
	case WM_LBUTTONDBLCLK:
		// Optional: double click also exits (simple behavior).
		PostMessage(WM_COMMAND, kCmdTrayExit);
		break;
	default:
		break;
	}

	return 0;
}

void CTrayHostWnd::OnTrayExit()
{
	PostMessage(WM_CLOSE);
}


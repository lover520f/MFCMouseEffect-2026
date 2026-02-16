#include "pch.h"
#include "framework.h"

#include "TrayHostWnd.h"
#include "resource.h"
#include "MFCMouseEffect.h"
#include "TrayMenuBuilder.h"
#include "TrayMenuCommands.h"
#include "MouseFx/Core/Control/AppController.h"
#include "MouseFx/Interfaces/IMouseEffect.h"

using namespace mousefx;

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
		lstrcpyn(m_trayIcon.szTip, _T("MFCMouseEffect - 右键菜单"), _countof(m_trayIcon.szTip));

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

	if (lp != WM_RBUTTONUP) {
		return 0;
	}

	CMFCMouseEffectApp* app = dynamic_cast<CMFCMouseEffectApp*>(AfxGetApp());
	mousefx::AppController* mouseFx = app ? app->mouseFx_.get() : nullptr;

	CMenu menu;
	TrayMenuBuilder::BuildTrayMenu(menu, mouseFx);

	POINT pt{};
	GetCursorPos(&pt);
	SetForegroundWindow();
	
	int cmd = menu.TrackPopupMenu(TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, this);

	// Handle commands
	if (cmd == kCmdTrayExit) {
		PostMessage(WM_CLOSE);
	}
	else if (cmd == kCmdTraySettings) {
		if (app) {
			app->ShowSettingsWindow();
		}
	}
	else if (cmd == kCmdStarRepo) {
		ShellExecute(nullptr, _T("open"), _T("https://github.com/sqmw/MFCMouseEffect"), nullptr, nullptr, SW_SHOWNORMAL);
	}
	else if (mouseFx) {
		std::string json;
		if (TrayMenuBuilder::TryBuildIpcJson(static_cast<UINT>(cmd), &json)) {
			mouseFx->HandleCommand(json);
		}

		std::string theme;
		if (TrayMenuBuilder::TryBuildTheme(static_cast<UINT>(cmd), &theme)) {
			mouseFx->SetTheme(theme);
		}
	}

	PostMessage(WM_NULL);
	return 0;

}

void CTrayHostWnd::OnTrayExit()
{
	PostMessage(WM_CLOSE);
}

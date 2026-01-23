#include "pch.h"
#include "framework.h"

#include "TrayHostWnd.h"
#include "resource.h"
#include "MFCMouseEffect.h"
#include "MouseFx/AppController.h"
#include "MouseFx/IMouseEffect.h"

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
	menu.CreatePopupMenu();

	// === Click Category Submenu ===
	CMenu clickMenu;
	clickMenu.CreatePopupMenu();
	clickMenu.AppendMenu(MF_STRING, kCmdClickRipple, _T("水波纹 (Ripple)"));
	clickMenu.AppendMenu(MF_STRING, kCmdClickStar, _T("星星 (Star)"));
	clickMenu.AppendMenu(MF_STRING, kCmdClickNone, _T("无 (None)"));
	
	// Check current selection
	if (mouseFx) {
		auto* clickEffect = mouseFx->GetEffect(mousefx::EffectCategory::Click);
		if (clickEffect) {
			std::string typeName = clickEffect->TypeName();
			if (typeName == "ripple") clickMenu.CheckMenuItem(kCmdClickRipple, MF_CHECKED);
			else if (typeName == "star") clickMenu.CheckMenuItem(kCmdClickStar, MF_CHECKED);
		} else {
			clickMenu.CheckMenuItem(kCmdClickNone, MF_CHECKED);
		}
	}
	menu.AppendMenu(MF_POPUP, (UINT_PTR)clickMenu.m_hMenu, _T("点击特效 (Click)"));

	// === Trail Category Submenu ===
	CMenu trailMenu;
	trailMenu.CreatePopupMenu();
	trailMenu.AppendMenu(MF_STRING, kCmdTrailLine, _T("拖尾线 (Line)"));
	trailMenu.AppendMenu(MF_STRING, kCmdTrailNone, _T("无 (None)"));
	
	if (mouseFx) {
		auto* trailEffect = mouseFx->GetEffect(mousefx::EffectCategory::Trail);
		if (trailEffect) {
			trailMenu.CheckMenuItem(kCmdTrailLine, MF_CHECKED);
		} else {
			trailMenu.CheckMenuItem(kCmdTrailNone, MF_CHECKED);
		}
	}
	menu.AppendMenu(MF_POPUP, (UINT_PTR)trailMenu.m_hMenu, _T("拖尾特效 (Trail)"));

	// === Other Categories (Coming Soon) ===
	CMenu hoverMenu;
	hoverMenu.CreatePopupMenu();
	hoverMenu.AppendMenu(MF_STRING | MF_GRAYED, kCmdHoverNone, _T("无 (None)"));
	menu.AppendMenu(MF_POPUP, (UINT_PTR)hoverMenu.m_hMenu, _T("悬停特效 (Hover) - 待实现"));

	CMenu scrollMenu;
	scrollMenu.CreatePopupMenu();
	scrollMenu.AppendMenu(MF_STRING | MF_GRAYED, kCmdScrollNone, _T("无 (None)"));
	menu.AppendMenu(MF_POPUP, (UINT_PTR)scrollMenu.m_hMenu, _T("滚轮特效 (Scroll) - 待实现"));

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, kCmdTrayExit, _T("退出"));

	POINT pt{};
	GetCursorPos(&pt);
	SetForegroundWindow();
	
	int cmd = menu.TrackPopupMenu(TPM_RIGHTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD, pt.x, pt.y, this);

	// Handle commands
	if (cmd == kCmdTrayExit) {
		PostMessage(WM_CLOSE);
	}
	else if (mouseFx) {
		switch (cmd) {
			// Click category
			case kCmdClickRipple:
				mouseFx->SetEffect(mousefx::EffectCategory::Click, "ripple");
				break;
			case kCmdClickStar:
				mouseFx->SetEffect(mousefx::EffectCategory::Click, "star");
				break;
			case kCmdClickNone:
				mouseFx->ClearEffect(mousefx::EffectCategory::Click);
				break;
			// Trail category
			case kCmdTrailLine:
				mouseFx->SetEffect(mousefx::EffectCategory::Trail, "line");
				break;
			case kCmdTrailNone:
				mouseFx->ClearEffect(mousefx::EffectCategory::Trail);
				break;
			default:
				break;
		}
	}

	PostMessage(WM_NULL);
	return 0;
}

void CTrayHostWnd::OnTrayExit()
{
	PostMessage(WM_CLOSE);
}

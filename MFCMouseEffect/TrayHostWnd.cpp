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
	clickMenu.AppendMenu(MF_STRING, kCmdClickText, _T("飘浮文字 (Text)"));
	clickMenu.AppendMenu(MF_STRING, kCmdClickNone, _T("无 (None)"));
	
	// Check current selection
	if (mouseFx) {
		auto* clickEffect = mouseFx->GetEffect(mousefx::EffectCategory::Click);
		if (clickEffect) {
			std::string typeName = clickEffect->TypeName();
			if (typeName == "ripple") clickMenu.CheckMenuItem(kCmdClickRipple, MF_CHECKED);
			else if (typeName == "star") clickMenu.CheckMenuItem(kCmdClickStar, MF_CHECKED);
			else if (typeName == "text") clickMenu.CheckMenuItem(kCmdClickText, MF_CHECKED);
		} else {
			clickMenu.CheckMenuItem(kCmdClickNone, MF_CHECKED);
		}
	}
	menu.AppendMenu(MF_POPUP, (UINT_PTR)clickMenu.m_hMenu, _T("点击特效 (Click)"));

	// === Trail Category Submenu ===
	CMenu trailMenu;
	trailMenu.CreatePopupMenu();
	trailMenu.AppendMenu(MF_STRING, kCmdTrailStreamer, _T("霓虹流光 (Streamer)"));
	trailMenu.AppendMenu(MF_STRING, kCmdTrailElectric, _T("赛博电弧 (Electric)"));
	trailMenu.AppendMenu(MF_STRING, kCmdTrailParticle, _T("彩虹粒子 (Particle)"));
	trailMenu.AppendMenu(MF_STRING, kCmdTrailLine, _T("普通线条 (Line)"));
	trailMenu.AppendMenu(MF_STRING, kCmdTrailNone, _T("无 (None)"));
	
	if (mouseFx) {
		auto* trailEffect = mouseFx->GetEffect(mousefx::EffectCategory::Trail);
		if (trailEffect) {
			std::string typeName = trailEffect->TypeName();
			if (typeName == "particle") trailMenu.CheckMenuItem(kCmdTrailParticle, MF_CHECKED);
			else if (typeName == "line") trailMenu.CheckMenuItem(kCmdTrailLine, MF_CHECKED);
			else if (typeName == "streamer") trailMenu.CheckMenuItem(kCmdTrailStreamer, MF_CHECKED);
			else if (typeName == "electric") trailMenu.CheckMenuItem(kCmdTrailElectric, MF_CHECKED);
		} else {
			trailMenu.CheckMenuItem(kCmdTrailNone, MF_CHECKED);
		}
	}
	menu.AppendMenu(MF_POPUP, (UINT_PTR)trailMenu.m_hMenu, _T("拖尾特效 (Trail)"));

	// === Scroll Category Submenu ===
	CMenu scrollMenu;
	scrollMenu.CreatePopupMenu();
	scrollMenu.AppendMenu(MF_STRING, kCmdScrollArrow, _T("方向指示 (Arrow)"));
	scrollMenu.AppendMenu(MF_STRING, kCmdScrollNone, _T("无 (None)"));
	
	if (mouseFx) {
		auto* scrollEffect = mouseFx->GetEffect(mousefx::EffectCategory::Scroll);
		if (scrollEffect) {
			scrollMenu.CheckMenuItem(kCmdScrollArrow, MF_CHECKED);
		} else {
			scrollMenu.CheckMenuItem(kCmdScrollNone, MF_CHECKED);
		}
	}
	menu.AppendMenu(MF_POPUP, (UINT_PTR)scrollMenu.m_hMenu, _T("滚轮特效 (Scroll)"));

	// === Hold Category Submenu ===
	CMenu holdMenu;
	holdMenu.CreatePopupMenu();
	holdMenu.AppendMenu(MF_STRING, kCmdHoldCharge, _T("蓄力 (Charge)"));
	holdMenu.AppendMenu(MF_STRING, kCmdHoldLightning, _T("闪电 (Lightning)"));
	holdMenu.AppendMenu(MF_STRING, kCmdHoldHex, _T("六边形 (Hex)"));
	holdMenu.AppendMenu(MF_STRING, kCmdHoldTechRing, _T("科技圈 (3D)"));
	holdMenu.AppendMenu(MF_STRING, kCmdHoldSciFi3D, _T("全息投影 (3D)"));
	holdMenu.AppendMenu(MF_STRING, kCmdHoldNone, _T("无 (None)"));
	
	if (mouseFx) {
		auto* holdEffect = mouseFx->GetEffect(mousefx::EffectCategory::Hold);
		if (holdEffect) {
			std::string typeName = holdEffect->TypeName();
			if (typeName == "charge") holdMenu.CheckMenuItem(kCmdHoldCharge, MF_CHECKED);
			else if (typeName == "lightning") holdMenu.CheckMenuItem(kCmdHoldLightning, MF_CHECKED);
			else if (typeName == "hex") holdMenu.CheckMenuItem(kCmdHoldHex, MF_CHECKED);
			else if (typeName == "tech_ring") holdMenu.CheckMenuItem(kCmdHoldTechRing, MF_CHECKED);
			else if (typeName == "hologram" || typeName == "scifi3d") holdMenu.CheckMenuItem(kCmdHoldSciFi3D, MF_CHECKED);
		} else {
			holdMenu.CheckMenuItem(kCmdHoldNone, MF_CHECKED);
		}
	}
	menu.AppendMenu(MF_POPUP, (UINT_PTR)holdMenu.m_hMenu, _T("长按特效 (Hold)"));

	// === Hover Category Submenu ===
	CMenu hoverMenu;
	hoverMenu.CreatePopupMenu();
	hoverMenu.AppendMenu(MF_STRING, kCmdHoverGlow, _T("呼吸灯 (Glow)"));
	hoverMenu.AppendMenu(MF_STRING, kCmdHoverNone, _T("无 (None)"));
	
	if (mouseFx) {
		auto* hoverEffect = mouseFx->GetEffect(mousefx::EffectCategory::Hover);
		if (hoverEffect) {
			hoverMenu.CheckMenuItem(kCmdHoverGlow, MF_CHECKED);
		} else {
			hoverMenu.CheckMenuItem(kCmdHoverNone, MF_CHECKED);
		}
	}
	menu.AppendMenu(MF_POPUP, (UINT_PTR)hoverMenu.m_hMenu, _T("悬停特效 (Hover)"));

	// === Theme Submenu ===
	CMenu themeMenu;
	themeMenu.CreatePopupMenu();
	themeMenu.AppendMenu(MF_STRING, kCmdThemeSciFi, _T("科幻 (Sci-Fi)"));
	themeMenu.AppendMenu(MF_STRING, kCmdThemeNeon, _T("霓虹 (Neon)"));
	themeMenu.AppendMenu(MF_STRING, kCmdThemeMinimal, _T("极简 (Minimal)"));
	themeMenu.AppendMenu(MF_STRING, kCmdThemeGame, _T("游戏感 (Game)"));

	if (mouseFx) {
		std::string theme = mouseFx->Config().theme;
		for (auto& c : theme) {
			if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
		}
		if (theme == "scifi" || theme == "sci-fi" || theme == "sci_fi") themeMenu.CheckMenuItem(kCmdThemeSciFi, MF_CHECKED);
		else if (theme == "minimal") themeMenu.CheckMenuItem(kCmdThemeMinimal, MF_CHECKED);
		else if (theme == "game") themeMenu.CheckMenuItem(kCmdThemeGame, MF_CHECKED);
		else themeMenu.CheckMenuItem(kCmdThemeNeon, MF_CHECKED);
	}
	menu.AppendMenu(MF_POPUP, (UINT_PTR)themeMenu.m_hMenu, _T("主题 (Theme)"));
    
    // Star link
	menu.AppendMenu(MF_STRING, kCmdStarRepo, _T("项目地址 / 支持作者 (Project/Star)"));

	menu.AppendMenu(MF_SEPARATOR);
	menu.AppendMenu(MF_STRING, kCmdTraySettings, _T("设置... (Settings...)"));
	menu.AppendMenu(MF_STRING, kCmdTrayExit, _T("退出"));

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
		auto sendEffect = [mouseFx](const char* category, const char* type) {
			std::string cmd = std::string("{\"cmd\":\"set_effect\",\"category\":\"") + category +
				"\",\"type\":\"" + type + "\"}";
			mouseFx->HandleCommand(cmd);
		};
		auto clearEffect = [mouseFx](const char* category) {
			std::string cmd = std::string("{\"cmd\":\"clear_effect\",\"category\":\"") + category + "\"}";
			mouseFx->HandleCommand(cmd);
		};
		switch (cmd) {
			// Click category
			case kCmdClickRipple:
				sendEffect("click", "ripple");
				break;
			case kCmdClickStar:
				sendEffect("click", "star");
				break;
			case kCmdClickText:
				sendEffect("click", "text");
				break;
			case kCmdClickNone:
				clearEffect("click");
				break;
			// Trail category
			case kCmdTrailLine:
				sendEffect("trail", "line");
				break;
			case kCmdTrailStreamer:
				sendEffect("trail", "streamer");
				break;
			case kCmdTrailElectric:
				sendEffect("trail", "electric");
				break;
			case kCmdTrailParticle:
				sendEffect("trail", "particle");
				break;
			case kCmdTrailNone:
				clearEffect("trail");
				break;
			// Scroll category
			case kCmdScrollArrow:
				sendEffect("scroll", "arrow");
				break;
			case kCmdScrollNone:
				clearEffect("scroll");
				break;
			// Hold category
			case kCmdHoldCharge:
				sendEffect("hold", "charge");
				break;
			case kCmdHoldLightning:
				sendEffect("hold", "lightning");
				break;
			case kCmdHoldHex:
				sendEffect("hold", "hex");
				break;
			case kCmdHoldTechRing:
				sendEffect("hold", "tech_ring");
				break;
			case kCmdHoldSciFi3D:
				// Note: mapped to 'hologram' in AppController if needed, or directly 'hologram'
				// But AppController::HandleCommand for "set_effect" calls CategoryFromString then SetEffect.
				// Wait, the client sends {cmd:"set_effect", type:"xx"}.
				// Our 'sendEffect' lambda sends "hologram". AppController should handle "hologram".
				// The kCmdHoldSciFi3D ID is mapped to "hologram" below?
				// Ah I need to update sendEffect call.
				sendEffect("hold", "hologram");
				break;
			case kCmdHoldNone:
				clearEffect("hold");
				break;
			// Hover category
			case kCmdHoverGlow:
				sendEffect("hover", "glow");
				break;
			case kCmdHoverNone:
				clearEffect("hover");
				break;
			// Theme
			case kCmdThemeSciFi:
				mouseFx->SetTheme("scifi");
				break;
			case kCmdThemeNeon:
				mouseFx->SetTheme("neon");
				break;
			case kCmdThemeMinimal:
				mouseFx->SetTheme("minimal");
				break;
			case kCmdThemeGame:
				mouseFx->SetTheme("game");
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

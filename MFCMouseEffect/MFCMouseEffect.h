
// MFCMouseEffect.h: MFCMouseEffect 应用程序的主头文件
//
#pragma once

#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含 'pch.h' 以生成 PCH"
#endif

#include "resource.h"       // 主符号

#include <memory>

namespace mousefx {
class AppController;
class IpcController;
}


// CMFCMouseEffectApp:
// 有关此类的实现，请参阅 MFCMouseEffect.cpp
//

class CMFCMouseEffectApp : public CWinAppEx
{
public:
	CMFCMouseEffectApp() noexcept;


// 重写
public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

// 实现
	UINT  m_nAppLook;
	BOOL  m_bHiColorIcons;

	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

public:
	// Public for TrayHostWnd access
	std::unique_ptr<mousefx::AppController> mouseFx_;
	std::unique_ptr<mousefx::IpcController> ipc_;

private:
	std::unique_ptr<class CTrayHostWnd> trayHost_;
};


extern CMFCMouseEffectApp theApp;

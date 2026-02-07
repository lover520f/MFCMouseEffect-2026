
// MFCMouseEffect.cpp: 定义应用程序的类行为。
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "MFCMouseEffect.h"
#include "UI/Frame/MainFrm.h"
#include "UI/Tray/TrayHostWnd.h"
#include "UI/Settings/SettingsWnd.h"
#include "Settings/SettingsBackend.h"

#include "UI/DocView/MFCMouseEffectDoc.h"
#include "UI/DocView/MFCMouseEffectView.h"

#include "MouseFx/Core/AppController.h"
#include "MouseFx/Core/IpcController.h"
#include "MouseFx/Server/WebSettingsServer.h"

#include <string>
#include <shellapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static void EnableDpiAwarenessForScreenCoords() {
	// Overlay host layers use screen coordinates from WH_MOUSE_LL. If the process is DPI-unaware,
	// the coordinate spaces can mismatch on scaled displays and effects may appear off-screen.
#ifndef DPI_AWARENESS_CONTEXT
	DECLARE_HANDLE(DPI_AWARENESS_CONTEXT);
#endif
#ifndef DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
#define DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((DPI_AWARENESS_CONTEXT)-4)
#endif

	HMODULE user32 = GetModuleHandleW(L"user32.dll");
	if (!user32) return;

	using SetProcessDpiAwarenessContextFn = BOOL(WINAPI*)(DPI_AWARENESS_CONTEXT);
	auto* setContext = reinterpret_cast<SetProcessDpiAwarenessContextFn>(
		GetProcAddress(user32, "SetProcessDpiAwarenessContext"));
	if (setContext) {
		setContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
		return;
	}

	using SetProcessDPIAwareFn = BOOL(WINAPI*)();
	auto* setAware = reinterpret_cast<SetProcessDPIAwareFn>(
		GetProcAddress(user32, "SetProcessDPIAware"));
	if (setAware) {
		setAware();
	}
}

static std::wstring FormatWin32ErrorMessage(DWORD error) {
	if (error == ERROR_SUCCESS) return L"(none)";

	wchar_t* msg = nullptr;
	const DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
	const DWORD n = FormatMessageW(flags, nullptr, error, 0, (LPWSTR)&msg, 0, nullptr);
	if (n == 0 || !msg) {
		wchar_t buf[64]{};
		wsprintfW(buf, L"Win32 error %lu", error);
		return buf;
	}

	std::wstring s(msg, msg + n);
	LocalFree(msg);
	while (!s.empty() && (s.back() == L'\r' || s.back() == L'\n' || s.back() == L' ' || s.back() == L'\t')) {
		s.pop_back();
	}
	return s;
}

static const wchar_t* StartStageToString(mousefx::AppController::StartStage stage) {
	using S = mousefx::AppController::StartStage;
	switch (stage) {
	case S::GdiPlusStartup: return L"GDI+ startup";
	case S::DispatchWindow: return L"dispatch window";
	case S::EffectInit: return L"effect initialization";
	case S::GlobalHook: return L"global mouse hook";
	default: return L"(unknown)";
	}
}

static std::string ExtractJsonValueA(const std::string& json, const std::string& key) {
	std::string search = "\"" + key + "\"";
	size_t keyPos = json.find(search);
	if (keyPos == std::string::npos) return "";

	size_t startQuote = json.find('"', keyPos + search.length());
	if (startQuote == std::string::npos) {
		startQuote = json.find('"', keyPos + search.length() + 1);
	}
	if (startQuote == std::string::npos) return "";

	size_t endQuote = json.find('"', startQuote + 1);
	if (endQuote == std::string::npos) return "";

	return json.substr(startQuote + 1, endQuote - startQuote - 1);
}

static bool IsExitCommand(const std::string& cmd) {
	if (cmd == "exit") return true;
	if (cmd.find("\"cmd\"") != std::string::npos) {
		return ExtractJsonValueA(cmd, "cmd") == "exit";
	}
	return false;
}


// CMFCMouseEffectApp

BEGIN_MESSAGE_MAP(CMFCMouseEffectApp, CWinAppEx)
	ON_COMMAND(ID_APP_ABOUT, &CMFCMouseEffectApp::OnAppAbout)
	// 基于文件的标准文档命令
	ON_COMMAND(ID_FILE_NEW, &CWinAppEx::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinAppEx::OnFileOpen)
	// 标准打印设置命令
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinAppEx::OnFilePrintSetup)
END_MESSAGE_MAP()


// CMFCMouseEffectApp 构造

CMFCMouseEffectApp::CMFCMouseEffectApp() noexcept
{
	m_bHiColorIcons = TRUE;


	m_nAppLook = 0;
	// 支持重新启动管理器
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_ALL_ASPECTS;
#ifdef _MANAGED
	// 如果应用程序是利用公共语言运行时支持(/clr)构建的，则: 
	//     1) 必须有此附加设置，“重新启动管理器”支持才能正常工作。
	//     2) 在您的项目中，您必须按照生成顺序向 System.Windows.Forms 添加引用。
	System::Windows::Forms::Application::SetUnhandledExceptionMode(System::Windows::Forms::UnhandledExceptionMode::ThrowException);
#endif

	// TODO: 将以下应用程序 ID 字符串替换为唯一的 ID 字符串；建议的字符串格式
	//为 CompanyName.ProductName.SubProduct.VersionInformation
	SetAppID(_T("MFCMouseEffect.AppID.NoVersion"));

	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
}

// 唯一的 CMFCMouseEffectApp 对象

CMFCMouseEffectApp theApp;


// CMFCMouseEffectApp 初始化

BOOL CMFCMouseEffectApp::InitInstance()
{
	// Ensure only one instance runs.
	hMutex_ = CreateMutexW(nullptr, TRUE, L"Global\\MFCMouseEffect_SingleInstance_Mutex");
	if (hMutex_ && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hMutex_);
		hMutex_ = nullptr;
		return FALSE; // Exit if another instance is already running
	}

	EnableDpiAwarenessForScreenCoords();
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();


	// 初始化 OLE 库
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}

	AfxEnableControlContainer();

	// 无需在任务栏显示主框架
	EnableTaskbarInteraction(FALSE);

	// 使用 RichEdit 控件需要 AfxInitRichEdit2()
	// Use RichEdit 5.0 (Msftedit.dll) for SettingsWnd
	m_hRichEditModule = LoadLibrary(L"Msftedit.dll");

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO: 应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));
	LoadStdProfileSettings(4);  // 加载标准 INI 文件选项(包括 MRU)


	InitContextMenuManager();

	InitKeyboardManager();

	InitTooltipManager();
	CMFCToolTipInfo ttParams;
	ttParams.m_bVislManagerTheme = TRUE;
	theApp.GetTooltipManager()->SetTooltipParams(AFX_TOOLTIP_TYPE_ALL,
		RUNTIME_CLASS(CMFCToolTipCtrl), &ttParams);

	// Parse command line arguments to determine mode
	// -mode tray (default)
	// -mode background
	bool showTrayIcon = true;
	int argc = 0;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	if (argv)
	{
		for (int i = 0; i < argc; ++i)
		{
			CString arg = argv[i];
			if (arg.CompareNoCase(L"-mode") == 0 && (i + 1 < argc))
			{
				CString val = argv[i + 1];
				if (val.CompareNoCase(L"background") == 0)
				{
					showTrayIcon = false;
				}
				// else if val == "tray", keep true.
			}
		}
		LocalFree(argv);
	}
	backgroundMode_ = !showTrayIcon;

#ifdef _DEBUG
	// Debug：创建 CTrayHostWnd (完整托盘菜单) + CMainFrame (MDI调试窗口)
	trayHost_ = std::make_unique<CTrayHostWnd>();
	trayHost_->CreateHost(showTrayIcon);

	CMainFrame* pMainFrame = new CMainFrame;
	if (!pMainFrame || !pMainFrame->LoadFrame(IDR_MAINFRAME))
	{
		delete pMainFrame;
		return FALSE;
	}
	m_pMainWnd = pMainFrame;
	pMainFrame->ShowWindow(SW_SHOW);
	pMainFrame->UpdateWindow();
#else
	// Release：仅创建隐藏宿主窗口用于托盘图标。
	trayHost_ = std::make_unique<CTrayHostWnd>();
	if (!trayHost_->CreateHost(showTrayIcon))
	{
		trayHost_.reset();
		return FALSE;
	}
	m_pMainWnd = trayHost_.get();
#endif

	// Start global mouse click effects (non-blocking, click-through).
	mouseFx_ = std::make_unique<mousefx::AppController>();
	if (!mouseFx_->Start())
	{
		// Keep the app running even if the effect subsystem fails (e.g. policy restrictions).
#ifdef _DEBUG
		TRACE0("MouseFx failed to start. Check Output window for details.\n");
		const auto diag = mouseFx_->Diagnostics();
		CString details;
		details.Format(L"Stage: %s\nError: %lu\nMessage: %s",
			StartStageToString(diag.stage),
			(unsigned long)diag.error,
			FormatWin32ErrorMessage(diag.error).c_str());
		AfxMessageBox(L"MouseFx failed to start.\n\n"
			L"Tips:\n"
			L"- Make sure you're running the correct exe (x64\\Debug\\MFCMouseEffect.exe).\n"
			L"- Try 'Run as administrator' if clicking admin windows.\n"
			L"- Check Visual Studio Output window for 'MouseFx:' logs.\n\n"
			+ details,
			MB_OK | MB_ICONWARNING);
#endif
		mouseFx_.reset();
	}

	// Start IPC listener (for parent process control)
	ipc_ = std::make_unique<mousefx::IpcController>();
	ipc_->Start([this](const std::string& cmd) {
		if (IsExitCommand(cmd)) {
			if (m_pMainWnd) {
				::PostMessageW(m_pMainWnd->GetSafeHwnd(), WM_CLOSE, 0, 0);
			} else {
				::PostQuitMessage(0);
			}
			return;
		}
		if (mouseFx_) {
			mouseFx_->HandleCommand(cmd);
		}
	}, [this]() {
		if (backgroundMode_) {
			if (m_pMainWnd) {
				::PostMessageW(m_pMainWnd->GetSafeHwnd(), WM_CLOSE, 0, 0);
			} else {
				::PostQuitMessage(0);
			}
		}
	});

	return TRUE;
}

void CMFCMouseEffectApp::ShowSettingsWindow()
{
	// Prefer the lightweight local web UI (opens in browser) to reduce MFC coupling.
	ShowWebSettings();
}

void CMFCMouseEffectApp::ShowWebSettings(const wchar_t* fragment)
{
	if (backgroundMode_) {
		return; // background mode is IPC-only
	}
	if (!mouseFx_) {
		return;
	}

	if (!webSettings_) {
		webSettings_ = std::make_unique<mousefx::WebSettingsServer>(mouseFx_.get());
	}
	webSettings_->RotateToken();
	if (!webSettings_->IsRunning()) {
		if (!webSettings_->Start()) {
			AfxMessageBox(L"Web settings server start failed.", MB_OK | MB_ICONWARNING);
			return;
		}
	}

	CString url(webSettings_->Url().c_str());
	if (fragment && *fragment) {
		url += fragment;
	}
	ShellExecuteW(nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
}

void CMFCMouseEffectApp::NotifySettingsWndDestroyed(CSettingsWnd* wnd)
{
	if (settingsWnd_ == wnd) {
		settingsWnd_ = nullptr;
	}
}

int CMFCMouseEffectApp::ExitInstance()
{
	if (hMutex_)
	{
		ReleaseMutex(hMutex_);
		CloseHandle(hMutex_);
		hMutex_ = nullptr;
	}
	//TODO: 处理可能已添加的附加资源
	if (ipc_)
	{
		ipc_->Stop();
		ipc_.reset();
	}
	if (webSettings_)
	{
		webSettings_->Stop();
		webSettings_.reset();
	}
	if (mouseFx_)
	{
		mouseFx_->Stop();
		mouseFx_.reset();
	}
	if (trayHost_)
	{
		trayHost_->DestroyWindow();
		trayHost_.reset();
	}
	if (m_hRichEditModule) {
		FreeLibrary(m_hRichEditModule);
		m_hRichEditModule = nullptr;
	}
	AfxOleTerm(FALSE);

	return CWinAppEx::ExitInstance();
}

// CMFCMouseEffectApp 消息处理程序


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg() noexcept;

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// 用于运行对话框的应用程序命令
void CMFCMouseEffectApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}

// CMFCMouseEffectApp 自定义加载/保存方法

void CMFCMouseEffectApp::PreLoadState()
{
	BOOL bNameValid;
	CString strName;
	bNameValid = strName.LoadString(IDS_EDIT_MENU);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EDIT);
	bNameValid = strName.LoadString(IDS_EXPLORER);
	ASSERT(bNameValid);
	GetContextMenuManager()->AddMenu(strName, IDR_POPUP_EXPLORER);
}

void CMFCMouseEffectApp::LoadCustomState()
{
}

void CMFCMouseEffectApp::SaveCustomState()
{
}

// CMFCMouseEffectApp 消息处理程序

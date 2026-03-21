; Reference: http://www.jrsoftware.org/ishelp/

#define MyAppName "MFCMouseEffect"
#define MyAppVersion "1.3.1"
#define MyAppPublisher "ksun22515@gmail.com"
#define MyAppURL "https://github.com/sqmw/MFCMouseEffect"
#define MyAppExeName "MFCMouseEffect.exe"
#define ReleaseDir "..\\x64\\Release\\"
#define ReleaseWebUiDir ReleaseDir + "webui\\"

; Optional Chinese language file detection (avoid build failure if not installed)
#define LangZh1 AddBackslash(GetEnv("ProgramFiles")) + "Inno Setup 6\\Languages\\ChineseSimplified.isl"
#define LangZh2 AddBackslash(GetEnv("ProgramFiles(x86)")) + "Inno Setup 6\\Languages\\ChineseSimplified.isl"
#if FileExists(LangZh1)
  #define LangZh LangZh1
#elif FileExists(LangZh2)
  #define LangZh LangZh2
#else
  #define LangZh ""
#endif

; Release packaging preflight (fail-fast on missing WebUI generated bundles)
#if !FileExists(ReleaseDir + MyAppExeName)
  #error "Missing release binary. Build x64 Release first."
#endif
#if !FileExists(ReleaseDir + "mfx_wasm_runtime.dll")
  #error "Missing mfx_wasm_runtime.dll in x64 Release output. Build WasmRuntimeBridge first."
#endif
#if !FileExists(ReleaseWebUiDir + "index.html")
  #error "Missing webui/index.html in x64 Release output."
#endif
#if !FileExists(ReleaseWebUiDir + "dialog.svelte.js")
  #error "Missing webui/dialog.svelte.js. Run WebUIWorkspace build before packaging."
#endif
#if !FileExists(ReleaseWebUiDir + "settings-shell.svelte.js")
  #error "Missing webui/settings-shell.svelte.js. Run WebUIWorkspace build before packaging."
#endif
#if !FileExists(ReleaseWebUiDir + "section-workspace.svelte.js")
  #error "Missing webui/section-workspace.svelte.js. Run WebUIWorkspace build before packaging."
#endif
#if !FileExists(ReleaseWebUiDir + "general-settings.svelte.js")
  #error "Missing webui/general-settings.svelte.js. Run WebUIWorkspace build before packaging."
#endif
#if !FileExists(ReleaseWebUiDir + "effects-settings.svelte.js")
  #error "Missing webui/effects-settings.svelte.js. Run WebUIWorkspace build before packaging."
#endif
#if !FileExists(ReleaseWebUiDir + "text-settings.svelte.js")
  #error "Missing webui/text-settings.svelte.js. Run WebUIWorkspace build before packaging."
#endif
#if !FileExists(ReleaseWebUiDir + "trail-settings.svelte.js")
  #error "Missing webui/trail-settings.svelte.js. Run WebUIWorkspace build before packaging."
#endif
#if !FileExists(ReleaseWebUiDir + "input-indicator-settings.svelte.js")
  #error "Missing webui/input-indicator-settings.svelte.js. Run WebUIWorkspace build before packaging."
#endif
#if !FileExists(ReleaseWebUiDir + "automation-ui.svelte.js")
  #error "Missing webui/automation-ui.svelte.js. Run WebUIWorkspace build before packaging."
#endif
#if !FileExists(ReleaseWebUiDir + "wasm-settings.svelte.js")
  #error "Missing webui/wasm-settings.svelte.js. Run WebUIWorkspace build before packaging."
#endif

[Setup]
; Unique ID for the application
AppId={{D3F7B7B1-4A2E-4F8A-8C8E-9B2E1D2E3F4G}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
UsePreviousAppDir=yes
DisableProgramGroupPage=yes
; Require administrative privileges for installation
PrivilegesRequired=admin
PrivilegesRequiredOverridesAllowed=dialog

; Version Info
VersionInfoVersion={#MyAppVersion}
VersionInfoCompany={#MyAppPublisher}
VersionInfoDescription={#MyAppName} Setup
VersionInfoProductVersion={#MyAppVersion}
VersionInfoCopyright=Copyright (C) 2026 {#MyAppPublisher}
VersionInfoProductName={#MyAppName}

OutputDir=Output
OutputBaseFilename=MFCMouseEffect_{#MyAppVersion}_Setup_x64
Compression=lzma
SolidCompression=yes
WizardStyle=modern
SetupIconFile=..\MFCMouseEffect\res\MFCMouseEffect.ico
UsePreviousLanguage=yes
ShowLanguageDialog=auto
; --- 64-bit Architecture Configuration ---
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible

; --- Close running instances automatically ---
; Do NOT use AppMutex here, otherwise Inno will prompt the user to close the app.
; We kill the process in [Code] before installing to avoid file-in-use prompts.
CloseApplications=yes
CloseApplicationsFilter={#MyAppExeName}
RestartApplications=no

[Languages]
#if LangZh != ""
Name: "chinesesimplified"; MessagesFile: "{#LangZh}"
#endif
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"
Name: "startup"; Description: "Run at Windows startup"; GroupDescription: "Additional options:"

[Files]
; Binaries + config
Source: "..\x64\Release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\x64\Release\config.json"; DestDir: "{app}"; Flags: ignoreversion onlyifdoesntexist skipifsourcedoesntexist
; Web UI (local server assets)
Source: "..\x64\Release\webui\*"; DestDir: "{app}\webui"; Flags: ignoreversion recursesubdirs createallsubdirs
; Runtime DLLs
Source: "..\x64\Release\webgpu_dawn.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "..\x64\Release\mfx_wasm_runtime.dll"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
; Docs + license (optional)
Source: "..\LICENSE"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "..\README.md"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist
Source: "..\README.en.md"; DestDir: "{app}"; Flags: ignoreversion skipifsourcedoesntexist

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Registry]
; Optional: Add startup entry if selected
Root: HKA; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "{#MyAppName}"; ValueData: """{app}\{#MyAppExeName}"" -mode tray"; Tasks: startup; Flags: uninsdeletevalue

[Code]
function KillAppIfRunning(): Boolean;
var
  ResultCode: Integer;
begin
  // Best-effort: ask Windows to terminate the running tray/background process.
  // This avoids "please close the app" prompts and prevents file-in-use issues.
  Result := True;

  // First try graceful tree kill (no /F).
  if Exec('taskkill', '/IM ' + '{#MyAppExeName}' + ' /T', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
  begin
    if (ResultCode = 0) or (ResultCode = 128) then
      Exit;
  end;

  // Fallback: force kill if still running.
  if Exec('taskkill', '/F /IM ' + '{#MyAppExeName}' + ' /T', '', SW_HIDE, ewWaitUntilTerminated, ResultCode) then
  begin
    // 0 = killed, 128 = not found
    Result := (ResultCode = 0) or (ResultCode = 128);
  end;
end;

function InitializeSetup(): Boolean;
begin
  Result := KillAppIfRunning();
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  // Also stop it on uninstall, otherwise uninstall may fail due to file locks.
  if CurUninstallStep = usUninstall then
    KillAppIfRunning();
end;

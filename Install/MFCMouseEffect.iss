; Reference: http://www.jrsoftware.org/ishelp/

#define MyAppName "MFCMouseEffect"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "YourName"
#define MyAppURL "https://example.com/mfcmouseeffect"
#define MyAppExeName "MFCMouseEffect.exe"

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
DisableProgramGroupPage=yes
; Require administrative privileges for installation
PrivilegesRequired=admin
OutputDir=Output
OutputBaseFilename=MFCMouseEffect_Setup
Compression=lzma
SolidCompression=yes
WizardStyle=modern

; --- CRITICAL Singleton Logic ---
; Using the mutex we implemented in the app to detect if it's currently running.
AppMutex=Global\MFCMouseEffect_SingleInstance_Mutex
; Ask the user to close the application if it is detected.
CloseApplications=yes

[Languages]
Name: "chinesesimplified"; MessagesFile: "compiler:Languages\ChineseSimplified.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked
Name: "startup"; Description: "Run at Windows startup"; GroupDescription: "Additional options:"; Flags: unchecked

[Files]
; Source path is relative to where the .iss file is located.
; Assuming .iss is in 'Install' folder, exe is in '..\x64\Release\'
Source: "..\x64\Release\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\x64\Release\config.json"; DestDir: "{app}"; Flags: ignoreversion onlyifdoesntexist
; Add any other assets here
; Source: "..\Config\*"; DestDir: "{app}\Config"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent

[Registry]
; Optional: Add startup entry if selected
Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Run"; ValueType: string; ValueName: "{#MyAppName}"; ValueData: """{app}\{#MyAppExeName}"" -mode tray"; Tasks: startup; Flags: uninsdeletevalue

[Code]
// Requirement 2: Delete existing installation files
// Inno Setup handles overwriting files in the same {app} directory automatically.
// If you want to force a clean uninstall before reinstalling, you can add custom logic here, 
// but usually the standard behavior is sufficient for MFC apps.

function InitializeSetup(): Boolean;
begin
  Result := True;
  // Additional pre-install checks can be placed here if needed.
end;

; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

#define MyAppName "TreeScan"
#define MyAppVersion "2.0 Beta 5 Build 1"
#define MyAppPublisher "Information Management Services, Ins."
#define MyAppURL "https://www.treescan.org/"
#define MyAppExeName "TreeScan.exe"

[Setup]
; NOTE: The value of AppId uniquely identifies this application. Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
AppId={{AD0046EA-ADC2-4AD7-B623-E53C00CDAEC9}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
;AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\{#MyAppName}
DisableProgramGroupPage=yes
LicenseFile=U:\eula\LicenseAgreement.rtf
InfoBeforeFile=U:\build\treescan\installers\inno-setup\before-install.rtf
; Uncomment the following line to run in non administrative install mode (install for current user only.)
;PrivilegesRequired=lowest
OutputDir=U:\installers\v.2.0.x
OutputBaseFilename=install-2_0_windows
;SetupIconFile=U:\build\treescan\installers\resources\TreeScan.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern
; "ArchitecturesInstallIn64BitMode=x64" requests that the install be
; done in "64-bit mode" on x64, meaning it should use the native
; 64-bit Program Files directory and the 64-bit view of the registry.
; On all other architectures it will install in "32-bit mode".
ArchitecturesInstallIn64BitMode=x64
; Note: We don't set ProcessorsAllowed because we want this
; installation to run on all architectures (including Itanium,
; since it's capable of running 32-bit code too).

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "U:\build\treescan\java_application\jni_application\dist\TreeScan.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "U:\build\treescan\java_application\jni_application\dist\lib\*"; DestDir: "{app}\lib"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "U:\build\treescan\installers\examples\*"; DestDir: "{app}\examples"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "U:\build\treescan\installers\java\jre_x86\*"; DestDir: "{app}\jre"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: not Is64BitInstallMode
Source: "U:\build\treescan\installers\java\jre_x64\*"; DestDir: "{app}\jre"; Flags: ignoreversion recursesubdirs createallsubdirs; Check: Is64BitInstallMode
Source: "U:\build\treescan\installers\documents\eula.html"; DestDir: "{app}"; Flags: ignoreversion
Source: "U:\build\treescan\installers\documents\userguide.pdf"; DestDir: "{app}"; Flags: ignoreversion
Source: "U:\build\treescan\batch_application\Win32\Release\treescan32.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "U:\build\treescan\batch_application\x64\Release\treescan64.exe"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "U:\build\treescan\library\Win32\Release\treescan32.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: not Is64BitInstallMode
Source: "U:\build\treescan\library\x64\Release\treescan64.dll"; DestDir: "{app}"; Flags: ignoreversion; Check: Is64BitInstallMode
Source: "U:\build\treescan\java_application\jni_application\dist\TreeScan.jar"; DestDir: "{app}"; Flags: ignoreversion
; NOTE: Don't use "Flags: ignoreversion" on any shared system files

[Icons]
Name: "{autoprograms}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent


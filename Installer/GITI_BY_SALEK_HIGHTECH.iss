; GITI BY SALEK HIGHTECH — Windows installer
#define MyAppName "GITI BY SALEK HIGHTECH"
#define MyAppVersion "1.0.0"
#define MyPublisher "ALI AGHAKOOCHAK AKA SALEK"
#define MyURL "https://github.com/alidante90-wq/SALEK-HIGHTECH-VST"

[Setup]
AppId={{7C1F6E2D-7B2A-4E4B-A5F4-5B9A4C7D1F10}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyPublisher}
AppPublisherURL={#MyURL}
DefaultDirName={autopf}\GITI BY SALEK HIGHTECH
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
OutputDir=installer-output
OutputBaseFilename=GITI_BY_SALEK_HIGHTECH_SETUP
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
UninstallDisplayName={#MyAppName}
VersionInfoDescription={#MyAppName} Installer
VersionInfoProductName={#MyAppName}
VersionInfoCompany={#MyPublisher}
VersionInfoCopyright=© 2026 Ali Aghakoochak / SALEK
SetupLogging=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut for GITI BY SALEK HIGHTECH"; Flags: unchecked

[Files]
Source: "package\GITI BY SALEK HIGHTECH.vst3"; DestDir: "{commoncf64}\VST3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "package\GITI BY SALEK HIGHTECH.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "package\Assets\*"; DestDir: "{app}\Assets"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}\GITI BY SALEK HIGHTECH"; Filename: "{app}\GITI BY SALEK HIGHTECH.exe"; WorkingDir: "{app}"
Name: "{autodesktop}\GITI BY SALEK HIGHTECH"; Filename: "{app}\GITI BY SALEK HIGHTECH.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\GITI BY SALEK HIGHTECH.exe"; Description: "Launch GITI BY SALEK HIGHTECH"; Flags: nowait postinstall skipifsilent

[Code]
procedure SetStatus(const S: String);
begin
  WizardForm.StatusLabel.Caption := S;
  WizardForm.StatusLabel.Update;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpWelcome then
    SetStatus('GITI ONLINE • Welcome to the SALEK sound universe');
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  case CurStep of
    ssInstall:
      SetStatus('GITI CORE DEPLOYMENT • Installing VST3 + Standalone + Assets...');
    ssPostInstall:
      SetStatus('GITI READY • 3-DAY TRIAL ENABLED • Have fun making noise :D');
  end;
end;

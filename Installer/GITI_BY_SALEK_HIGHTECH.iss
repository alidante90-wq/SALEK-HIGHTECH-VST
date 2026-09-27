; GITI BY SALEK HIGHTECH — professional Windows installer
#ifndef MyAppVersion
  #define MyAppVersion "1.0.0"
#endif
#ifndef MyAppEdition
  #define MyAppEdition "001"
#endif
#ifndef MyAppEditionName
  #define MyAppEditionName "ORIGIN"
#endif

#define MyAppName "GITI BY SALEK HIGHTECH"
#define MyPublisher "ALI AGHAKOOCHAK AKA SALEK"
#define MyURL "https://github.com/alidante90-wq/SALEK-HIGHTECH-VST"
#define MyDisplayVersion MyAppVersion + " • GENESIS #" + MyAppEdition
#define MyDisplayEdition "GENESIS #" + MyAppEdition + " • " + MyAppEditionName

[Setup]
AppId={{7C1F6E2D-7B2A-4E4B-A5F4-5B9A4C7D1F10}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyDisplayVersion}
AppPublisher={#MyPublisher}
AppPublisherURL={#MyURL}
AppSupportURL={#MyURL}
AppUpdatesURL={#MyURL}
DefaultDirName={autopf}\GITI BY SALEK HIGHTECH
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=no
OutputDir=..\installer-output
OutputBaseFilename=GITI BY SALEK Setup
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
UninstallDisplayName={#MyAppName} • {#MyDisplayEdition}
VersionInfoDescription={#MyAppName} Installer • {#MyDisplayEdition}
VersionInfoProductName={#MyAppName}
VersionInfoCompany={#MyPublisher}
VersionInfoCopyright=© 2026 Ali Aghakoochak / SALEK
SetupLogging=yes
Uninstallable=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut for GITI BY SALEK HIGHTECH"; Flags: unchecked

[Files]
Source: "package\GITI BY SALEK HIGHTECH.vst3"; DestDir: "{commoncf64}\VST3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "package\GITI BY SALEK HIGHTECH.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "package\Assets\*"; DestDir: "{app}\Assets"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}\GITI BY SALEK HIGHTECH • {#MyDisplayEdition}"; Filename: "{app}\GITI BY SALEK HIGHTECH.exe"; WorkingDir: "{app}"
Name: "{autodesktop}\GITI BY SALEK HIGHTECH • {#MyDisplayEdition}"; Filename: "{app}\GITI BY SALEK HIGHTECH.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\GITI BY SALEK HIGHTECH.exe"; Description: "Launch GITI BY SALEK HIGHTECH • {#MyDisplayEdition}"; Flags: nowait postinstall skipifsilent

[Code]
procedure SetStatus(const S: String);
begin
  WizardForm.StatusLabel.Caption := S;
  WizardForm.StatusLabel.Update;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpWelcome then
  begin
    WizardForm.WelcomeLabel1.Caption := 'GITI BY SALEK HIGHTECH';
    WizardForm.WelcomeLabel2.Caption :=
      'Genesis Edition #{#MyAppEdition} • {#MyAppEditionName}' + #13#10#13#10 +
      'Software Version {#MyAppVersion}' + #13#10 +
      'ALI AGHAKOOCHAK AKA SALEK' + #13#10#13#10 +
      'High-Tech Psytrance • Sound Design • Digital Instrument Universe';
    SetStatus('GITI ONLINE • {#MyDisplayEdition} • Version {#MyAppVersion}');
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  case CurStep of
    ssInstall:
      SetStatus('GITI CORE DEPLOYMENT • Installing VST3 + Standalone + Assets...');
    ssPostInstall:
      SetStatus('GITI READY • {#MyDisplayEdition} • 3-DAY TRIAL ENABLED');
  end;
end;

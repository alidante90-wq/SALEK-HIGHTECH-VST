; GITI BY SALEK HIGHTECH — Genesis Edition installer
; Every installer is bound to a real compiled GITI edition.
#ifndef MyAppVersion
  #define MyAppVersion "1.0.0"
#endif
#ifndef MyAppEdition
  #define MyAppEdition "001"
#endif
#ifndef MyAppEditionName
  #define MyAppEditionName "ORIGIN"
#endif
#ifndef MyAppId
  #define MyAppId "00000000-0000-0000-0000-000000000001"
#endif
#ifndef MyPluginFile
  #define MyPluginFile "GITI BY SALEK HIGHTECH GENESIS 001.vst3"
#endif
#ifndef MyStandaloneFile
  #define MyStandaloneFile "GITI BY SALEK HIGHTECH GENESIS 001.exe"
#endif
#ifndef MyInstallDir
  #define MyInstallDir "GITI BY SALEK HIGHTECH Genesis 001"
#endif
#ifndef MyOutputBaseFilename
  #define MyOutputBaseFilename "GITI BY SALEK HIGHTECH - GENESIS 001 - ORIGIN Setup"
#endif

#define MyAppName "GITI BY SALEK HIGHTECH"
#define MyPublisher "ALI AGHAKOOCHAK AKA SALEK"
#define MyURL "https://github.com/alidante90-wq/SALEK-HIGHTECH-VST"
#define MyDisplayVersion MyAppVersion + " • GENESIS #" + MyAppEdition
#define MyDisplayEdition "GENESIS #" + MyAppEdition + " • " + MyAppEditionName

[Setup]
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyDisplayVersion} • {#MyAppEditionName}
AppPublisher={#MyPublisher}
AppPublisherURL={#MyURL}
AppSupportURL={#MyURL}
AppUpdatesURL={#MyURL}
DefaultDirName={autopf}\{#MyInstallDir}
DefaultGroupName={#MyAppName}\Genesis #{#MyAppEdition}
DisableProgramGroupPage=no
OutputDir=..\installer-output
OutputBaseFilename={#MyOutputBaseFilename}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
PrivilegesRequired=admin
UninstallDisplayName={#MyAppName} • {#MyDisplayEdition}
VersionInfoDescription={#MyAppName} Installer • {#MyDisplayEdition}
VersionInfoProductName={#MyAppName} • Genesis #{#MyAppEdition}
VersionInfoCompany={#MyPublisher}
VersionInfoCopyright=© 2026 Ali Aghakoochak / SALEK
SetupLogging=yes
Uninstallable=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut for this GITI Genesis edition"; Flags: unchecked

[Files]
Source: "package\{#MyPluginFile}"; DestDir: "{commoncf64}\VST3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "package\{#MyStandaloneFile}"; DestDir: "{app}"; Flags: ignoreversion
Source: "package\Assets\*"; DestDir: "{app}\Assets"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}\Genesis #{#MyAppEdition} • {#MyAppEditionName}"; Filename: "{app}\{#MyStandaloneFile}"; WorkingDir: "{app}"
Name: "{autodesktop}\GITI • #{#MyAppEdition} • {#MyAppEditionName}"; Filename: "{app}\{#MyStandaloneFile}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyStandaloneFile}"; Description: "Launch GITI • {#MyDisplayEdition}"; Flags: nowait postinstall skipifsilent

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
      'FIRST GENERATION • SALEK DIGITAL INSTRUMENT UNIVERSE';
    SetStatus('GITI ONLINE • {#MyDisplayEdition} • Version {#MyAppVersion}');
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  case CurStep of
    ssInstall:
      SetStatus('GITI CORE DEPLOYMENT • Installing Genesis VST3 + Standalone + Assets...');
    ssPostInstall:
      SetStatus('GITI READY • {#MyDisplayEdition} • 3-DAY TRIAL ENABLED');
  end;
end;

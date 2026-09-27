; GITI BY SALEK HIGHTECH — FINAL E01 COMMERCIAL INSTALLER
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
#define MyAppId "8E0D9E7A-5C9A-4E1E-9A3A-4D1D9E7A0001"
#endif
#ifndef MyPluginFile
#define MyPluginFile "GITI BY SALEK HIGHTECH.vst3"
#endif
#ifndef MyStandaloneFile
#define MyStandaloneFile "GITI BY SALEK HIGHTECH.exe"
#endif
#ifndef MyInstallDir
#define MyInstallDir "GITI BY SALEK HIGHTECH"
#endif
#ifndef MyOutputBaseFilename
#define MyOutputBaseFilename "GITI BY SALEK Setup"
#endif

#define MyAppName "GITI BY SALEK HIGHTECH"
#define MyPublisher "ALI AGHAKOOCHAK AKA SALEK"
#define MyURL "https://github.com/alidante90-wq/SALEK-HIGHTECH-VST"

[Setup]
AppId={#MyAppId}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
VersionInfoVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion} - GENESIS #{#MyAppEdition} - {#MyAppEditionName}
AppPublisher={#MyPublisher}
AppComments=GITI Genesis sonic instrument from the SALEK Universe
AppPublisherURL={#MyURL}
AppSupportURL={#MyURL}
AppUpdatesURL={#MyURL}
DefaultDirName={autopf}\{#MyInstallDir}
DefaultGroupName={#MyAppName}
OutputDir=..\installer-output
OutputBaseFilename={#MyOutputBaseFilename}
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=admin
Uninstallable=yes
UninstallDisplayName={#MyAppName}
VersionInfoDescription={#MyAppName} Installer
VersionInfoProductName={#MyAppName}
VersionInfoCompany={#MyPublisher}
VersionInfoCopyright=Copyright 2026 Ali Aghakoochak / SALEK
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
SetupLogging=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Create a desktop shortcut"; Flags: unchecked

[Files]
Source: "..\package\{#MyPluginFile}"; DestDir: "{commoncf64}\VST3"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\package\{#MyStandaloneFile}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\package\Assets\*"; DestDir: "{app}\Assets"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autoprograms}\{#MyAppName}\GITI Genesis #{#MyAppEdition} - {#MyAppEditionName}"; Filename: "{app}\{#MyStandaloneFile}"; WorkingDir: "{app}"
Name: "{autodesktop}\GITI Genesis #{#MyAppEdition}"; Filename: "{app}\{#MyStandaloneFile}"; WorkingDir: "{app}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyStandaloneFile}"; Description: "Launch GITI BY SALEK HIGHTECH"; Flags: nowait postinstall skipifsilent

[Code]

var
  AboutButton: TNewButton;

procedure ShowGitiAbout;
begin
  MsgBox(
    'GITI BY SALEK HIGHTECH' + #13#10 +
    'GENESIS #{#MyAppEdition} — {#MyAppEditionName}' + #13#10#13#10 +
    'Created by ALI AGHAKOOCHAK AKA SALEK' + #13#10 +
    'High-Tech / Hitech Producer • Sound Designer • Songwriter' + #13#10#13#10 +
    'GITI is the first generation of sonic machines in the SALEK UNIVERSE.' + #13#10 +
    'It is designed as a precision sound instrument: small changes in the right' + #13#10 +
    'frequencies can create large musical consequences — a sonic butterfly effect.' + #13#10#13#10 +
    'Copyright 2026 Ali Aghakoochak / SALEK' + #13#10 +
    'GITI BY SALEK HIGHTECH',
    mbInformation, MB_OK);
end;

procedure InitializeWizard;
begin
  AboutButton := WizardForm.CreateButton(WizardForm.NextButton.Parent);
  AboutButton.Caption := 'ABOUT GITI';
  AboutButton.Left := WizardForm.BackButton.Left;
  AboutButton.Top := WizardForm.BackButton.Top - ScaleY(34);
  AboutButton.Width := ScaleX(110);
  AboutButton.Height := ScaleY(25);
  AboutButton.OnClick := @ShowGitiAbout;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  if CurPageID = wpWelcome then
  begin
    WizardForm.WelcomeLabel1.Caption := 'GITI BY SALEK HIGHTECH';
    WizardForm.WelcomeLabel2.Caption :=
      'SALEK UNIVERSE - FIRST GENERATION' + #13#10#13#10 +
      'GENESIS # {#MyAppEdition} - {#MyAppEditionName}' + #13#10 +
      'Software Version {#MyAppVersion}' + #13#10#13#10 +
      'ALI AGHAKOOCHAK AKA SALEK' + #13#10#13#10 +
      'A professional sonic instrument from the SALEK Universe.';
  end;
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then
    WizardForm.StatusLabel.Caption := 'Installing GITI core, VST3, Standalone and Universe assets...';
end;

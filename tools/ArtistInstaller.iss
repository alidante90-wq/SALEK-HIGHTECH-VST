#ifndef ArtistName
  #define ArtistName "GITI Artist"
#endif
#ifndef StageDir
  #define StageDir "dist"
#endif
#ifndef OutputDir
  #define OutputDir "dist\setups"
#endif
#define SafeName StringChange(ArtistName, " ", "-")
[Setup]
AppId={{B2A70A55-71A4-4C1D-AE50-2F2B8A00D1C1}
AppName=GITI BY SALEK HIGHTECH - {#ArtistName}
AppVersion=0.9.0
AppPublisher=SALEK / ISATIS ART CENTER
DefaultDirName={autopf}\GITI BY SALEK HIGHTECH\{#ArtistName}
DefaultGroupName=GITI BY SALEK HIGHTECH\{#ArtistName}
DisableProgramGroupPage=yes
OutputDir={#OutputDir}
OutputBaseFilename={#SafeName}-GITI-Setup
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
ArchitecturesInstallIn64BitMode=x64
PrivilegesRequired=admin
Uninstallable=yes

[Files]
Source: "{#StageDir}\*"; DestDir: "{app}"; Flags: recursesubdirs createallsubdirs ignoreversion

[Icons]
Name: "{group}\Open install folder"; Filename: "{app}"

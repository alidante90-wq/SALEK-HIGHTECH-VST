Unicode true
RequestExecutionLevel admin
Name "GITI BY SALEK HIGHTECH"
Caption "GITI BY SALEK HIGHTECH - Genesis #001"
OutFile "..\installer-output\GITI BY SALEK Setup.exe"
InstallDir "$PROGRAMFILES64\GITI BY SALEK HIGHTECH"
InstallDirRegKey HKLM "Software\GITI BY SALEK HIGHTECH" "Install_Dir"
VIProductVersion "1.0.0.0"
VIAddVersionKey "ProductName" "GITI BY SALEK HIGHTECH"
VIAddVersionKey "CompanyName" "ALI AGHAKOOCHAK AKA SALEK"
VIAddVersionKey "FileDescription" "GITI BY SALEK HIGHTECH - Genesis #001"
VIAddVersionKey "LegalCopyright" "Copyright 2026 Ali Aghakoochak / SALEK"
SetCompressor /SOLID lzma

PageEx license
  LicenseText "GITI BY SALEK HIGHTECH - SALEK UNIVERSE"
  LicenseData "..\Installer\GITI_LICENSE.txt"
PageExEnd
Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

Section "GITI BY SALEK HIGHTECH"
  SetOutPath "$INSTDIR"
  File "..\package\GITI BY SALEK HIGHTECH.exe"
  SetOutPath "$INSTDIR\Assets"
  File /r "..\package\Assets\*"
  SetOutPath "$COMMONFILES64\VST3\GITI BY SALEK HIGHTECH.vst3"
  File /r "..\package\GITI BY SALEK HIGHTECH.vst3\*"

  WriteRegStr HKLM "Software\GITI BY SALEK HIGHTECH" "Install_Dir" "$INSTDIR"
  WriteUninstaller "$INSTDIR\Uninstall GITI BY SALEK HIGHTECH.exe"
  CreateDirectory "$SMPROGRAMS\GITI BY SALEK HIGHTECH"
  CreateShortCut "$SMPROGRAMS\GITI BY SALEK HIGHTECH\GITI BY SALEK HIGHTECH.lnk" "$INSTDIR\GITI BY SALEK HIGHTECH.exe"
  CreateShortCut "$DESKTOP\GITI BY SALEK HIGHTECH.lnk" "$INSTDIR\GITI BY SALEK HIGHTECH.exe"
SectionEnd

Section "Uninstall"
  Delete "$DESKTOP\GITI BY SALEK HIGHTECH.lnk"
  Delete "$SMPROGRAMS\GITI BY SALEK HIGHTECH\GITI BY SALEK HIGHTECH.lnk"
  RMDir "$SMPROGRAMS\GITI BY SALEK HIGHTECH"
  Delete "$INSTDIR\Uninstall GITI BY SALEK HIGHTECH.exe"
  RMDir /r "$INSTDIR\Assets"
  Delete "$INSTDIR\GITI BY SALEK HIGHTECH.exe"
  RMDir "$INSTDIR"
  RMDir /r "$COMMONFILES64\VST3\GITI BY SALEK HIGHTECH.vst3"
  DeleteRegKey HKLM "Software\GITI BY SALEK HIGHTECH"
SectionEnd

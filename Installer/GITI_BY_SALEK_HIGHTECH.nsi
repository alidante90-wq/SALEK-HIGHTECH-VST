Unicode true
RequestExecutionLevel admin
!include "MUI2.nsh"
!define MUI_ABORTWARNING
!define MUI_WELCOMEPAGE_TITLE "Welcome to GITI BY SALEK HIGHTECH"
!define MUI_WELCOMEPAGE_TEXT "SALEK UNIVERSE - FIRST GENERATION.$\r$\n$\r$\nGENESIS #001 - ORIGIN.$\r$\n$\r$\nALI AGHAKOOCHAK AKA SALEK$\r$\n$\r$\nA professional sonic instrument from the SALEK Universe."
!define MUI_FINISHPAGE_RUN "$INSTDIR\GITI BY SALEK HIGHTECH.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch GITI BY SALEK HIGHTECH"
!define MUI_FINISHPAGE_LINK "SALEK UNIVERSE"
!define MUI_FINISHPAGE_LINK_LOCATION "https://github.com/alidante90-wq/SALEK-HIGHTECH-VST"
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
BrandingText "SALEK UNIVERSE • GITI GENESIS #001"
ShowInstDetails show
ShowUninstDetails show
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_LANGUAGE "English"

Section "GITI Core" SecCore
  SectionIn RO
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

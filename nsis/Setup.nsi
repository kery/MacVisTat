##################################################################
# Include Modern UI

!include "MUI2.nsh"

##################################################################
# General

# Name and file
Name "VisualStatistics"
OutFile "..\build\VisualStatisticsSetup_win64.exe"

# Default installation folder
InstallDir "$PROGRAMFILES64\VisualStatistics"

# Get installation folder from registry if available
InstallDirRegKey HKLM "Software\VisualStatistics" ""

# Request application privileges for Windows Vista
RequestExecutionLevel admin

ShowInstDetails show
ShowUninstDetails show

##################################################################
#Interface Settings
!define MUI_ABORTWARNING

##################################################################
# Pages

!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\VisualStatistics.exe"
# Use show readme for shortcut creation
!define MUI_FINISHPAGE_SHOWREADME
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Create Shortcut"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION createDesktopShortcut
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

##################################################################
# Languages

!insertmacro MUI_LANGUAGE "English"

##################################################################
# Functions

Function createDesktopShortcut
    CreateShortCut "$DESKTOP\VisualStatistics.lnk" "$INSTDIR\VisualStatistics.exe"
FunctionEnd

##################################################################
# Installer Sections

Section "VisualStatistics (required)" secVisualStatistics
    SectionIn RO
    SetShellVarContext all
    SetOutPath "$INSTDIR"

    File /r "D:\Software\Tools\VisualStatistics\*.*"

    # Store installation folder
    WriteRegStr HKLM "Software\VisualStatistics" "" $INSTDIR

    # Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Start Menu Shortcuts" secStartMenuShortcuts
    CreateDirectory "$SMPROGRAMS\VisualStatistics"
    CreateShortCut "$SMPROGRAMS\VisualStatistics\VisualStatistics.lnk" "$INSTDIR\VisualStatistics.exe"
    CreateShortCut "$SMPROGRAMS\VisualStatistics\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
SectionEnd

##################################################################
# Section descriptions

LangString descSecVisualstatistics ${LANG_ENGLISH} "Install the required files of VisualStatistics"
LangString descSecStartMenuShortcuts ${LANG_ENGLISH} "Create shortcuts in start menu"
;Assign language strings to sections
!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${secVisualStatistics} $(descSecVisualstatistics)
    !insertmacro MUI_DESCRIPTION_TEXT ${secStartMenuShortcuts} $(descSecStartMenuShortcuts)
!insertmacro MUI_FUNCTION_DESCRIPTION_END

##################################################################
# Uninstaller Section

Section "Uninstall"
    SetShellVarContext all
    # Delete shortcuts
    Delete "$DESKTOP\VisualStatistics.lnk"
    Delete "$SMPROGRAMS\VisualStatistics\VisualStatistics.lnk"
    Delete "$SMPROGRAMS\VisualStatistics\Uninstall.lnk"
    RMDir "$SMPROGRAMS\VisualStatistics"
    
    # Using this is not safe (refer to user manual for more information)
    RMDir /r "$INSTDIR"

    DeleteRegKey /ifempty HKLM "Software\VisualStatistics"
SectionEnd
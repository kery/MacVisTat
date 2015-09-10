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
# Variables
Var startMenuFolder

##################################################################
#Interface Settings
!define MUI_ABORTWARNING

##################################################################
# Pages

!insertmacro MUI_PAGE_DIRECTORY
# Start Menu Folder Page Configuration
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\VisualStatistics" 
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
!insertmacro MUI_PAGE_STARTMENU "Application" $startMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\VisualStatistics.exe"
# Use show readme for shortcut creation
!define MUI_FINISHPAGE_SHOWREADME
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Create Shortcut"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION createShortcut
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

##################################################################
# Languages

!insertmacro MUI_LANGUAGE "English"

##################################################################
# Functions

Function createShortcut
    CreateShortCut "$DESKTOP\VisualStatistics.lnk" "$INSTDIR\VisualStatistics.exe"
FunctionEnd

##################################################################
# Installer Sections

Section
    SetOutPath "$INSTDIR"

    File /r "D:\Software\Tools\VisualStatistics\*.*"

    # Store installation folder
    WriteRegStr HKLM "Software\VisualStatistics" "" $INSTDIR

    # Create uninstaller
    WriteUninstaller "$INSTDIR\Uninstall.exe"
    
    !insertmacro MUI_STARTMENU_WRITE_BEGIN "Application"
    # Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\VisualStatistics.lnk" "$INSTDIR\VisualStatistics.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

##################################################################
# Uninstaller Section

Section "Uninstall"
    # Delete shortcuts
    Delete "$DESKTOP\VisualStatistics.lnk"
    !insertmacro MUI_STARTMENU_GETFOLDER "Application" $startMenuFolder
    Delete "$SMPROGRAMS\$startMenuFolder\VisualStatistics.lnk"
    Delete "$SMPROGRAMS\$startMenuFolder\Uninstall.lnk"
    RMDir "$SMPROGRAMS\$startMenuFolder"
    
    # Using this is not safe (refer to user manual for more information)
    RMDir /r "$INSTDIR"

    DeleteRegKey /ifempty HKLM "Software\VisualStatistics"
SectionEnd
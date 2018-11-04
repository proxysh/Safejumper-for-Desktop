!include "MUI2.nsh"
icon "application.ico"
Name Shieldtra
OutFile Shieldtra_install.exe ; NsiDecompiler: generated value!
InstallColors 00FF00 000000
InstallDir 'C:\Program Files\Shieldtra'
Page directory "" "" ""
Page instfiles "" "" ""
Page custom ""  ""

!define VERSION         "1.0"
!define BUILD             "1"

Section main

SectionIn RO
       CreateDirectory $INSTDIR
       SetOutPath $INSTDIR
       # Stop and uninstall service in case it's running
       nsExec::Exec '$INSTDIR\shieldtraservice.exe -t'
       Pop $0
       nsExec::Exec '$INSTDIR\shieldtraservice.exe -u'
       Pop $0
       
       File  libeay32.dll
       File  vcredist_x86.exe
       File  ssleay32.dll
       File  application.ico
       File  shieldtra.exe
       File  shieldtraservice.exe
       File  Qt5Core.dll
       File  Qt5Gui.dll
       File  Qt5Network.dll
       File  Qt5Svg.dll
       File  Qt5Widgets.dll
       File  Qt5Xml.dll
	   File  Qt5Qml.dll
	   File  Qt5Quick.dll
	   File  Qt5QuickControls2.dll
	   File  Qt5QuickTemplates2.dll
	   File  ..\src\languages\gui_zh.qm
	   File  ..\src\languages\gui_en.qm
       SetOutPath $INSTDIR\platforms
       File  platforms\qwindows.dll
	   SetOutPath $INSTDIR\QtGraphicalEffects
	   File  /r QtGraphicalEffects\*
	   SetOutPath $INSTDIR\QtQuick
	   File  /r QtQuick\*
	   SetOutPath $INSTDIR\QtQuick.2
	   File  /r QtQuick.2\*
       SetOutPath $TEMP
       File  openvpn-proxysh.exe
       Push $0
       ExecWait '$OUTDIR\openvpn-proxysh.exe /S /SELECT_PATH=0 /SELECT_OPENVPNGUI=0 /SELECT_SHORTCUTS=0 /D=$INSTDIR\OpenVPN' $0
       IfErrors Label_0x19 Label_0x1A

  Label_0x19:
       MessageBox  MB_OK 'OpenVPN network card driver installation fails. Reinstall, please.' /SD IDOK

  Label_0x1A:
       Delete  $OUTDIR\openvpn-proxysh.exe

        ExecWait '$INSTDIR\vcredist_x86.exe /install /quiet /norestart'

       ; Stop and unnistall in case a previous build is installed
       nsExec::Exec '$INSTDIR\shieldtraservice.exe -t'
       Pop $0
       nsExec::Exec '$INSTDIR\shieldtraservice.exe -u'
       Pop $0
       nsExec::Exec '$INSTDIR\shieldtraservice.exe -i'
       Pop $0
       nsExec::Exec '$INSTDIR\shieldtraservice.exe -s'
       Pop $0

    # set DACL for todylserver
    nsExec::Exec 'sc sdset "Shieldtra" D:(A;;CCLCSWRPWPDTLOCRRC;;;SY)(A;;CCDCLCSWRPWPDTLOCRSDRCWDWO;;;BA)(A;;CCLCSWLOCRRCRPWP;;;IU)(A;;CCLCSWLOCRRC;;;SU)'

    # create the uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"

    # create a shortcut named "new shortcut" in the start menu programs directory
    # point the new shortcut at the program uninstaller

	SetOutPath $INSTDIR
    CreateShortCut  "$DESKTOP\Shieldtra for Windows.lnk" "$INSTDIR\shieldtra.exe"
    CreateDirectory "$SMPROGRAMS\Shieldtra"
    CreateShortCut  "$SMPROGRAMS\Shieldtra\Shieldtra.lnk" "$INSTDIR\shieldtra.exe"
    CreateShortCut  "$SMPROGRAMS\Shieldtra\Uninstall.lnk" "$INSTDIR\uninstall.exe"

    # Add uninstaller to registry for easy uninstallation
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Shieldtra" \
            "DisplayName" "Shieldtra"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Shieldtra" \
            "DisplayIcon" "$INSTDIR\application.ico"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Shieldtra" \
            "Publisher" "shieldtra.com"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Shieldtra" \
            "DisplayVersion" "${VERSION} build ${BUILD}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Shieldtra" \
            "UninstallString" "$\"$INSTDIR\uninstall.exe$\""
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Shieldtra" \
            "QuietUninstallString" "$\"$INSTDIR\uninstall.exe$\" /S"
SectionEnd

# uninstaller section start
Section "uninstall"

    Delete  $DESKTOP\Shieldtra.lnk
    Delete  $SMPROGRAMS\Shieldtra\Shieldtra.lnk
    Delete  $SMPROGRAMS\Shieldtra\Uninstall.lnk
    ExecWait '$INSTDIR\OpenVPN\Uninstall.exe /S'  $0
    RMDir /r $INSTDIR\*.*
    RMDir $INSTDIR
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Shieldtra"

# uninstaller section end
SectionEnd

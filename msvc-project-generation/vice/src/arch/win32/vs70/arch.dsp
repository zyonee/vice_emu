# Microsoft Developer Studio Project File - Name="arch" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=arch - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "arch.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "arch.mak" CFG="arch - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "arch - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "arch - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "arch - Win32 DX Release" (based on "Win32 (x86) Static Library")
!MESSAGE "arch - Win32 DX Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "arch - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "libs\arch\Release"
# PROP Intermediate_Dir "libs\arch\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "NODIRECTX" /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\msvc" /I "..\\" /I "..\..\..\\" /I "..\..\..\arch\win32\msvc\wpcap" /I "..\..\..\c128" /I "..\..\..\c64" /I "..\..\..\c64\cart" /I "..\..\..\c64dtv" /I "..\..\..\cbm2" /I "..\..\..\drive" /I "..\..\..\imagecontents" /I "..\..\..\lib\libffmpeg" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\pet" /I "..\..\..\platform" /I "..\..\..\plus4" /I "..\..\..\raster" /I "..\..\..\rs232drv" /I "..\..\..\sid" /I "..\..\..\userport" /I "..\..\..\vdrive" /I "..\..\..\vic20"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "NODIRECTX" /D "NDEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE" /d "NODIRECTX"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE" /d "NODIRECTX"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "arch - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "libs\arch\Debug"
# PROP Intermediate_Dir "libs\arch\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "NODIRECTX" /D "_DEBUG" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "..\msvc" /I "..\\" /I "..\..\..\\" /I "..\..\..\arch\win32\msvc\wpcap" /I "..\..\..\c128" /I "..\..\..\c64" /I "..\..\..\c64\cart" /I "..\..\..\c64dtv" /I "..\..\..\cbm2" /I "..\..\..\drive" /I "..\..\..\imagecontents" /I "..\..\..\lib\libffmpeg" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\pet" /I "..\..\..\platform" /I "..\..\..\plus4" /I "..\..\..\raster" /I "..\..\..\rs232drv" /I "..\..\..\sid" /I "..\..\..\userport" /I "..\..\..\vdrive" /I "..\..\..\vic20"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "NODIRECTX" /D "_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE" /d "NODIRECTX"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE" /d "NODIRECTX"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "arch - Win32 DX Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DXRelease"
# PROP BASE Intermediate_Dir "DXRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "libs\arch\DXRelease"
# PROP Intermediate_Dir "libs\arch\DXRelease"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\msvc" /I "..\\" /I "..\..\..\\" /I "..\..\..\arch\win32\msvc\wpcap" /I "..\..\..\c128" /I "..\..\..\c64" /I "..\..\..\c64\cart" /I "..\..\..\c64dtv" /I "..\..\..\cbm2" /I "..\..\..\drive" /I "..\..\..\imagecontents" /I "..\..\..\lib\libffmpeg" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\pet" /I "..\..\..\platform" /I "..\..\..\plus4" /I "..\..\..\raster" /I "..\..\..\rs232drv" /I "..\..\..\sid" /I "..\..\..\userport" /I "..\..\..\vdrive" /I "..\..\..\vic20"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "arch - Win32 DX Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DXDebug"
# PROP BASE Intermediate_Dir "DXDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "libs\arch\DXDebug"
# PROP Intermediate_Dir "libs\arch\DXDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "..\msvc" /I "..\\" /I "..\..\..\\" /I "..\..\..\arch\win32\msvc\wpcap" /I "..\..\..\c128" /I "..\..\..\c64" /I "..\..\..\c64\cart" /I "..\..\..\c64dtv" /I "..\..\..\cbm2" /I "..\..\..\drive" /I "..\..\..\imagecontents" /I "..\..\..\lib\libffmpeg" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\pet" /I "..\..\..\platform" /I "..\..\..\plus4" /I "..\..\..\raster" /I "..\..\..\rs232drv" /I "..\..\..\sid" /I "..\..\..\userport" /I "..\..\..\vdrive" /I "..\..\..\vic20"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF

# Begin Target

# Name "arch - Win32 Release"
# Name "arch - Win32 Debug"
# Name "arch - Win32 DX Release"
# Name "arch - Win32 DX Debug"
# Begin Source File

SOURCE="..\..\..\arch\win32\archdep.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\blockdev.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\c128ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\c64dtvui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\c64ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\catweaselmkiii.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\cbm2ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\cbm5x0ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\console.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\dinput_handle.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\dirent.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\dynlib.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\fullscrn.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\gifdrv.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\hardsid.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\intl.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\joy.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\kbd.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\lightpendrv.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\mididrv.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\mousedrv.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\parsid.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\petui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\plus4ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\rawnetarch.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\rs232.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\rs232dev.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\rs232net.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\scpu64ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\signals.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\statusbar.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\system.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\ui-resources.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiacia.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiattach.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiautostart.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uic128.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uic128model.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uic64_256k.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uic64burstmod.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uic64cart.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uic64dtv.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uic64dtvmodel.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uic64model.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uicart.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uicbm2model.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uicbm5x0model.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uicbm2set.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uicbm5x0set.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uicia.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uicmdline.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiconsole.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uidatasette.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uidigimax.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uidqbb.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uidrivec128.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uidrivec64.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uidrivepetcbm2.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uidriveplus4.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uidrivevic20.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uids12c887rtc.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uieasyflash.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uievent.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiexpert.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uifliplist.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uigeoram.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uihelp.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiide64.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiisepic.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uijoystick.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uikeyboard.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uilib.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uilightpen.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uimagicvoice.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uimediafile.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uimidi.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uimmc64.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uimmcreplay.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uimon.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uimouse.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uinetwork.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiperipheral.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uipetcolour.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uipetdww.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uipetmodel.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uipetreu.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uipetset.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiplus256k.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiplus4cart.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiplus4mem.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiplus4model.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiplus60k.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiquicksnapshot.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiram.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiramcart.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiretroreplay.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uireu.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uirom.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uirs232.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uirs232user.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiscpu64.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uisid.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uisidcart.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uisiddtv.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uisnapshot.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uisound.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uisoundexpander.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uisoundsampler.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uispeed.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uited.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uitfe.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uiv364speech.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uivic.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uivic20model.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uivicii.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uivicset.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\uivideo.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\vic20ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\video-ddraw.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\video-dx9.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\video.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\vsidui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\vsyncarch.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\win32\winmain.c"
# End Source File
# Begin Source File

SOURCE="..\res.txt"

!IF  "$(CFG)" == "arch - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__CUSTOM="..\res.txt"	"..\resdialogs.txt"	"..\resmenu.txt"	"..\resstrings.txt"	
# Begin Custom Build - Generating res.h
InputDir=.
InputPath="..\res.txt"

"$(InputDir)\..\res.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\..\..\data\genwinres ..\res.h ..\menuid.h ..\stringid.h ..\res.txt ..\resdialogs.txt ..\resmenu.txt ..\resstrings.txt

# End Custom Build

!ELSEIF  "$(CFG)" == "arch - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__CUSTOM="..\res.txt"	"..\resdialogs.txt"	"..\resmenu.txt"	"..\resstrings.txt"	
# Begin Custom Build - Generating res.h
InputDir=.
InputPath="..\res.txt"

"$(InputDir)\..\res.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\..\..\data\genwinres ..\res.h ..\menuid.h ..\stringid.h ..\res.txt ..\resdialogs.txt ..\resmenu.txt ..\resstrings.txt

# End Custom Build

!ELSEIF  "$(CFG)" == "arch - Win32 DX Release"

# PROP Ignore_Default_Tool 1
USERDEP__CUSTOM="..\res.txt"	"..\resdialogs.txt"	"..\resmenu.txt"	"..\resstrings.txt"	
# Begin Custom Build - Generating res.h
InputDir=.
InputPath="..\res.txt"

"$(InputDir)\..\res.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\..\..\data\genwinres ..\res.h ..\menuid.h ..\stringid.h ..\res.txt ..\resdialogs.txt ..\resmenu.txt ..\resstrings.txt

# End Custom Build

!ELSEIF  "$(CFG)" == "arch - Win32 DX Debug"

# PROP Ignore_Default_Tool 1
USERDEP__CUSTOM="..\res.txt"	"..\resdialogs.txt"	"..\resmenu.txt"	"..\resstrings.txt"	
# Begin Custom Build - Generating res.h
InputDir=.
InputPath="..\res.txt"

"$(InputDir)\..\res.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\..\..\data\genwinres ..\res.h ..\menuid.h ..\stringid.h ..\res.txt ..\resdialogs.txt ..\resmenu.txt ..\resstrings.txt

# End Custom Build

!ENDIF

# End Source File
# End Target
# End Project

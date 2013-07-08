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
# ADD BASE CPP /nologo /MD /W3 /GX /O2  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\" /I "..\\" /I "..\..\..\\" /I "..\..\..\arch\win32\msvc\wpcap" /I "..\..\..\c64" /I "..\..\..\c64\cart" /I "..\..\..\c64dtv" /I "..\..\..\c128" /I "..\..\..\cbm2" /I "..\..\..\drive" /I "..\..\..\imagecontents" /I "..\..\..\lib\libffmpeg" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\pet" /I "..\..\..\platform" /I "..\..\..\plus4" /I "..\..\..\raster" /I "..\..\..\rs232drv" /I "..\..\..\sid" /I "..\..\..\userport" /I "..\..\..\vdrive" /I "..\..\..\vic20"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE"
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
# ADD BASE CPP /nologo /MDd /W3 /GX /Z7 /Od  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I ".\" /I "..\\" /I "..\..\..\\" /I "..\..\..\arch\win32\msvc\wpcap" /I "..\..\..\c64" /I "..\..\..\c64\cart" /I "..\..\..\c64dtv" /I "..\..\..\c128" /I "..\..\..\cbm2" /I "..\..\..\drive" /I "..\..\..\imagecontents" /I "..\..\..\lib\libffmpeg" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\pet" /I "..\..\..\platform" /I "..\..\..\plus4" /I "..\..\..\raster" /I "..\..\..\rs232drv" /I "..\..\..\sid" /I "..\..\..\userport" /I "..\..\..\vdrive" /I "..\..\..\vic20"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /YX /FD /c
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
# Begin Source File

SOURCE="..\..\..\arch\sdl\archdep.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\blockdev.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\catweaselmkiii.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\console.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\dynlib.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\fullscreen.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\hardsid.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\joy.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\kbd.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\lightpendrv.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_c128hw.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_c64_common_expansions.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_c64_expansions.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_c64cart.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_c64dtvhw.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_c64hw.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_c64model.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_cbm2cart.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_cbm2hw.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_common.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_debug.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_drive.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_drive_rom.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_ffmpeg.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_help.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_joystick.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_lightpen.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_midi.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_mouse.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_network.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_petcart.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_pethw.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_plus4cart.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_plus4hw.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_printer.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_ram.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_reset.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_rom.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_rs232.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_scpu64hw.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_screenshot.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_settings.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_sid.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_snapshot.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_sound.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_speed.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_tape.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_tfe.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_vic20cart.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_vic20hw.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\menu_video.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\mousedrv.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\rawnetarch.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\rs232.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\rs232dev.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\rs232net.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\sdlmain.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\signals.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\uicmdline.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\uifilereq.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\uihotkey.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\uimenu.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\uimon.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\uimsgbox.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\uipause.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\uipoll.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\uistatusbar.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\video.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\vkbd.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\vsidui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\vsyncarch.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\x128_ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\x64_ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\x64dtv_ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\x64sc_ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\xcbm2_ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\xpet_ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\xplus4_ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\xscpu64_ui.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\arch\sdl\xvic_ui.c"
# End Source File
# End Target
# End Project

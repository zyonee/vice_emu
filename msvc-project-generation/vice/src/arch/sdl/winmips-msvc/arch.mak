# Microsoft Developer Studio Generated NMAKE File, Based on arch.dsp
!IF "$(CFG)" == ""
CFG=arch - Win32 Release
!MESSAGE No configuration specified. Defaulting to arch - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "arch - Win32 Release" && "$(CFG)" != "arch - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
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
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "arch - Win32 Release"

OUTDIR=.\libs\arch\Release
INTDIR=.\libs\arch\Release
# Begin Custom Macros
OutDir=.\libs\arch\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\arch.lib" 

!ELSE 

ALL : "base - Win32 Release" "geninfocontrib - Win32 Release" "gentranslate - Win32 Release" "$(OUTDIR)\arch.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"gentranslate - Win32 ReleaseCLEAN" "geninfocontrib - Win32 ReleaseCLEAN" "base - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\arch\sdl\archdep.obj"
	-@erase "$(INTDIR)\arch\sdl\blockdev.obj"
	-@erase "$(INTDIR)\arch\sdl\catweaselmkiii.obj"
	-@erase "$(INTDIR)\arch\sdl\console.obj"
	-@erase "$(INTDIR)\arch\sdl\dynlib.obj"
	-@erase "$(INTDIR)\arch\sdl\fullscreen.obj"
	-@erase "$(INTDIR)\arch\sdl\hardsid.obj"
	-@erase "$(INTDIR)\arch\sdl\joy.obj"
	-@erase "$(INTDIR)\arch\sdl\kbd.obj"
	-@erase "$(INTDIR)\arch\sdl\lightpendrv.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_c128hw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_c64_common_expansions.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_c64_expansions.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_c64cart.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_c64dtvhw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_c64hw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_c64model.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_cbm2cart.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_cbm2hw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_common.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_debug.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_drive.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_drive_rom.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_ffmpeg.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_help.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_joystick.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_lightpen.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_midi.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_mouse.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_network.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_petcart.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_pethw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_plus4cart.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_plus4hw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_printer.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_ram.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_reset.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_rom.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_rs232.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_scpu64hw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_screenshot.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_settings.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_sid.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_snapshot.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_sound.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_speed.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_tape.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_tfe.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_vic20cart.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_vic20hw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_video.obj"
	-@erase "$(INTDIR)\arch\sdl\mousedrv.obj"
	-@erase "$(INTDIR)\arch\sdl\rawnetarch.obj"
	-@erase "$(INTDIR)\arch\sdl\rs232.obj"
	-@erase "$(INTDIR)\arch\sdl\rs232dev.obj"
	-@erase "$(INTDIR)\arch\sdl\rs232net.obj"
	-@erase "$(INTDIR)\arch\sdl\sdlmain.obj"
	-@erase "$(INTDIR)\arch\sdl\signals.obj"
	-@erase "$(INTDIR)\arch\sdl\ui.obj"
	-@erase "$(INTDIR)\arch\sdl\uicmdline.obj"
	-@erase "$(INTDIR)\arch\sdl\uifilereq.obj"
	-@erase "$(INTDIR)\arch\sdl\uihotkey.obj"
	-@erase "$(INTDIR)\arch\sdl\uimenu.obj"
	-@erase "$(INTDIR)\arch\sdl\uimon.obj"
	-@erase "$(INTDIR)\arch\sdl\uimsgbox.obj"
	-@erase "$(INTDIR)\arch\sdl\uipause.obj"
	-@erase "$(INTDIR)\arch\sdl\uipoll.obj"
	-@erase "$(INTDIR)\arch\sdl\uistatusbar.obj"
	-@erase "$(INTDIR)\arch\sdl\video.obj"
	-@erase "$(INTDIR)\arch\sdl\vkbd.obj"
	-@erase "$(INTDIR)\arch\sdl\vsidui.obj"
	-@erase "$(INTDIR)\arch\sdl\vsyncarch.obj"
	-@erase "$(INTDIR)\arch\sdl\x128_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\x64_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\x64dtv_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\x64sc_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\xcbm2_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\xpet_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\xplus4_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\xscpu64_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\xvic_ui.obj"
	-@erase "$(OUTDIR)\arch.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\arch\win32\msvc\wpcap "/I "..\..\..\c64 "/I "..\..\..\c64\cart "/I "..\..\..\c64dtv "/I "..\..\..\c128 "/I "..\..\..\cbm2 "/I "..\..\..\drive "/I "..\..\..\imagecontents "/I "..\..\..\lib\libffmpeg "/I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\pet "/I "..\..\..\platform "/I "..\..\..\plus4 "/I "..\..\..\raster "/I "..\..\..\rs232drv "/I "..\..\..\sid "/I "..\..\..\userport "/I "..\..\..\vdrive "/I "..\..\..\vic20 "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\arch.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

.c{$(INTDIR)}.obj :
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj :
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj :
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr :
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr :
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr :
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\arch.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\arch.lib" 
LIB32_OBJS= \
	"$(INTDIR)\arch\sdl\archdep.obj" \
	"$(INTDIR)\arch\sdl\blockdev.obj" \
	"$(INTDIR)\arch\sdl\catweaselmkiii.obj" \
	"$(INTDIR)\arch\sdl\console.obj" \
	"$(INTDIR)\arch\sdl\dynlib.obj" \
	"$(INTDIR)\arch\sdl\fullscreen.obj" \
	"$(INTDIR)\arch\sdl\hardsid.obj" \
	"$(INTDIR)\arch\sdl\joy.obj" \
	"$(INTDIR)\arch\sdl\kbd.obj" \
	"$(INTDIR)\arch\sdl\lightpendrv.obj" \
	"$(INTDIR)\arch\sdl\menu_c128hw.obj" \
	"$(INTDIR)\arch\sdl\menu_c64_common_expansions.obj" \
	"$(INTDIR)\arch\sdl\menu_c64_expansions.obj" \
	"$(INTDIR)\arch\sdl\menu_c64cart.obj" \
	"$(INTDIR)\arch\sdl\menu_c64dtvhw.obj" \
	"$(INTDIR)\arch\sdl\menu_c64hw.obj" \
	"$(INTDIR)\arch\sdl\menu_c64model.obj" \
	"$(INTDIR)\arch\sdl\menu_cbm2cart.obj" \
	"$(INTDIR)\arch\sdl\menu_cbm2hw.obj" \
	"$(INTDIR)\arch\sdl\menu_common.obj" \
	"$(INTDIR)\arch\sdl\menu_debug.obj" \
	"$(INTDIR)\arch\sdl\menu_drive.obj" \
	"$(INTDIR)\arch\sdl\menu_drive_rom.obj" \
	"$(INTDIR)\arch\sdl\menu_ffmpeg.obj" \
	"$(INTDIR)\arch\sdl\menu_help.obj" \
	"$(INTDIR)\arch\sdl\menu_joystick.obj" \
	"$(INTDIR)\arch\sdl\menu_lightpen.obj" \
	"$(INTDIR)\arch\sdl\menu_midi.obj" \
	"$(INTDIR)\arch\sdl\menu_mouse.obj" \
	"$(INTDIR)\arch\sdl\menu_network.obj" \
	"$(INTDIR)\arch\sdl\menu_petcart.obj" \
	"$(INTDIR)\arch\sdl\menu_pethw.obj" \
	"$(INTDIR)\arch\sdl\menu_plus4cart.obj" \
	"$(INTDIR)\arch\sdl\menu_plus4hw.obj" \
	"$(INTDIR)\arch\sdl\menu_printer.obj" \
	"$(INTDIR)\arch\sdl\menu_ram.obj" \
	"$(INTDIR)\arch\sdl\menu_reset.obj" \
	"$(INTDIR)\arch\sdl\menu_rom.obj" \
	"$(INTDIR)\arch\sdl\menu_rs232.obj" \
	"$(INTDIR)\arch\sdl\menu_scpu64hw.obj" \
	"$(INTDIR)\arch\sdl\menu_screenshot.obj" \
	"$(INTDIR)\arch\sdl\menu_settings.obj" \
	"$(INTDIR)\arch\sdl\menu_sid.obj" \
	"$(INTDIR)\arch\sdl\menu_snapshot.obj" \
	"$(INTDIR)\arch\sdl\menu_sound.obj" \
	"$(INTDIR)\arch\sdl\menu_speed.obj" \
	"$(INTDIR)\arch\sdl\menu_tape.obj" \
	"$(INTDIR)\arch\sdl\menu_tfe.obj" \
	"$(INTDIR)\arch\sdl\menu_vic20cart.obj" \
	"$(INTDIR)\arch\sdl\menu_vic20hw.obj" \
	"$(INTDIR)\arch\sdl\menu_video.obj" \
	"$(INTDIR)\arch\sdl\mousedrv.obj" \
	"$(INTDIR)\arch\sdl\rawnetarch.obj" \
	"$(INTDIR)\arch\sdl\rs232.obj" \
	"$(INTDIR)\arch\sdl\rs232dev.obj" \
	"$(INTDIR)\arch\sdl\rs232net.obj" \
	"$(INTDIR)\arch\sdl\sdlmain.obj" \
	"$(INTDIR)\arch\sdl\signals.obj" \
	"$(INTDIR)\arch\sdl\ui.obj" \
	"$(INTDIR)\arch\sdl\uicmdline.obj" \
	"$(INTDIR)\arch\sdl\uifilereq.obj" \
	"$(INTDIR)\arch\sdl\uihotkey.obj" \
	"$(INTDIR)\arch\sdl\uimenu.obj" \
	"$(INTDIR)\arch\sdl\uimon.obj" \
	"$(INTDIR)\arch\sdl\uimsgbox.obj" \
	"$(INTDIR)\arch\sdl\uipause.obj" \
	"$(INTDIR)\arch\sdl\uipoll.obj" \
	"$(INTDIR)\arch\sdl\uistatusbar.obj" \
	"$(INTDIR)\arch\sdl\video.obj" \
	"$(INTDIR)\arch\sdl\vkbd.obj" \
	"$(INTDIR)\arch\sdl\vsidui.obj" \
	"$(INTDIR)\arch\sdl\vsyncarch.obj" \
	"$(INTDIR)\arch\sdl\x128_ui.obj" \
	"$(INTDIR)\arch\sdl\x64_ui.obj" \
	"$(INTDIR)\arch\sdl\x64dtv_ui.obj" \
	"$(INTDIR)\arch\sdl\x64sc_ui.obj" \
	"$(INTDIR)\arch\sdl\xcbm2_ui.obj" \
	"$(INTDIR)\arch\sdl\xpet_ui.obj" \
	"$(INTDIR)\arch\sdl\xplus4_ui.obj" \
	"$(INTDIR)\arch\sdl\xscpu64_ui.obj" \
	"$(INTDIR)\arch\sdl\xvic_ui.obj" \
	".\libsbase\Release\base.lib" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\arch\Debug
INTDIR=.\libs\arch\Debug
# Begin Custom Macros
OutDir=.\libs\arch\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\arch.lib" 

!ELSE 

ALL : "base - Win32 Debug" "geninfocontrib - Win32 Debug" "gentranslate - Win32 Debug" "$(OUTDIR)\arch.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"gentranslate - Win32 DebugCLEAN" "geninfocontrib - Win32 DebugCLEAN" "base - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\arch\sdl\archdep.obj"
	-@erase "$(INTDIR)\arch\sdl\blockdev.obj"
	-@erase "$(INTDIR)\arch\sdl\catweaselmkiii.obj"
	-@erase "$(INTDIR)\arch\sdl\console.obj"
	-@erase "$(INTDIR)\arch\sdl\dynlib.obj"
	-@erase "$(INTDIR)\arch\sdl\fullscreen.obj"
	-@erase "$(INTDIR)\arch\sdl\hardsid.obj"
	-@erase "$(INTDIR)\arch\sdl\joy.obj"
	-@erase "$(INTDIR)\arch\sdl\kbd.obj"
	-@erase "$(INTDIR)\arch\sdl\lightpendrv.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_c128hw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_c64_common_expansions.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_c64_expansions.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_c64cart.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_c64dtvhw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_c64hw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_c64model.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_cbm2cart.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_cbm2hw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_common.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_debug.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_drive.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_drive_rom.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_ffmpeg.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_help.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_joystick.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_lightpen.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_midi.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_mouse.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_network.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_petcart.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_pethw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_plus4cart.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_plus4hw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_printer.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_ram.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_reset.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_rom.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_rs232.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_scpu64hw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_screenshot.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_settings.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_sid.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_snapshot.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_sound.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_speed.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_tape.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_tfe.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_vic20cart.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_vic20hw.obj"
	-@erase "$(INTDIR)\arch\sdl\menu_video.obj"
	-@erase "$(INTDIR)\arch\sdl\mousedrv.obj"
	-@erase "$(INTDIR)\arch\sdl\rawnetarch.obj"
	-@erase "$(INTDIR)\arch\sdl\rs232.obj"
	-@erase "$(INTDIR)\arch\sdl\rs232dev.obj"
	-@erase "$(INTDIR)\arch\sdl\rs232net.obj"
	-@erase "$(INTDIR)\arch\sdl\sdlmain.obj"
	-@erase "$(INTDIR)\arch\sdl\signals.obj"
	-@erase "$(INTDIR)\arch\sdl\ui.obj"
	-@erase "$(INTDIR)\arch\sdl\uicmdline.obj"
	-@erase "$(INTDIR)\arch\sdl\uifilereq.obj"
	-@erase "$(INTDIR)\arch\sdl\uihotkey.obj"
	-@erase "$(INTDIR)\arch\sdl\uimenu.obj"
	-@erase "$(INTDIR)\arch\sdl\uimon.obj"
	-@erase "$(INTDIR)\arch\sdl\uimsgbox.obj"
	-@erase "$(INTDIR)\arch\sdl\uipause.obj"
	-@erase "$(INTDIR)\arch\sdl\uipoll.obj"
	-@erase "$(INTDIR)\arch\sdl\uistatusbar.obj"
	-@erase "$(INTDIR)\arch\sdl\video.obj"
	-@erase "$(INTDIR)\arch\sdl\vkbd.obj"
	-@erase "$(INTDIR)\arch\sdl\vsidui.obj"
	-@erase "$(INTDIR)\arch\sdl\vsyncarch.obj"
	-@erase "$(INTDIR)\arch\sdl\x128_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\x64_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\x64dtv_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\x64sc_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\xcbm2_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\xpet_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\xplus4_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\xscpu64_ui.obj"
	-@erase "$(INTDIR)\arch\sdl\xvic_ui.obj"
	-@erase "$(OUTDIR)\arch.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\arch\win32\msvc\wpcap "/I "..\..\..\c64 "/I "..\..\..\c64\cart "/I "..\..\..\c64dtv "/I "..\..\..\c128 "/I "..\..\..\cbm2 "/I "..\..\..\drive "/I "..\..\..\imagecontents "/I "..\..\..\lib\libffmpeg "/I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\pet "/I "..\..\..\platform "/I "..\..\..\plus4 "/I "..\..\..\raster "/I "..\..\..\rs232drv "/I "..\..\..\sid "/I "..\..\..\userport "/I "..\..\..\vdrive "/I "..\..\..\vic20 "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\arch.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

.c{$(INTDIR)}.obj :
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj :
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj :
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr :
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr :
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr :
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\arch.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\arch.lib" 
LIB32_OBJS= \
	"$(INTDIR)\arch\sdl\archdep.obj" \
	"$(INTDIR)\arch\sdl\blockdev.obj" \
	"$(INTDIR)\arch\sdl\catweaselmkiii.obj" \
	"$(INTDIR)\arch\sdl\console.obj" \
	"$(INTDIR)\arch\sdl\dynlib.obj" \
	"$(INTDIR)\arch\sdl\fullscreen.obj" \
	"$(INTDIR)\arch\sdl\hardsid.obj" \
	"$(INTDIR)\arch\sdl\joy.obj" \
	"$(INTDIR)\arch\sdl\kbd.obj" \
	"$(INTDIR)\arch\sdl\lightpendrv.obj" \
	"$(INTDIR)\arch\sdl\menu_c128hw.obj" \
	"$(INTDIR)\arch\sdl\menu_c64_common_expansions.obj" \
	"$(INTDIR)\arch\sdl\menu_c64_expansions.obj" \
	"$(INTDIR)\arch\sdl\menu_c64cart.obj" \
	"$(INTDIR)\arch\sdl\menu_c64dtvhw.obj" \
	"$(INTDIR)\arch\sdl\menu_c64hw.obj" \
	"$(INTDIR)\arch\sdl\menu_c64model.obj" \
	"$(INTDIR)\arch\sdl\menu_cbm2cart.obj" \
	"$(INTDIR)\arch\sdl\menu_cbm2hw.obj" \
	"$(INTDIR)\arch\sdl\menu_common.obj" \
	"$(INTDIR)\arch\sdl\menu_debug.obj" \
	"$(INTDIR)\arch\sdl\menu_drive.obj" \
	"$(INTDIR)\arch\sdl\menu_drive_rom.obj" \
	"$(INTDIR)\arch\sdl\menu_ffmpeg.obj" \
	"$(INTDIR)\arch\sdl\menu_help.obj" \
	"$(INTDIR)\arch\sdl\menu_joystick.obj" \
	"$(INTDIR)\arch\sdl\menu_lightpen.obj" \
	"$(INTDIR)\arch\sdl\menu_midi.obj" \
	"$(INTDIR)\arch\sdl\menu_mouse.obj" \
	"$(INTDIR)\arch\sdl\menu_network.obj" \
	"$(INTDIR)\arch\sdl\menu_petcart.obj" \
	"$(INTDIR)\arch\sdl\menu_pethw.obj" \
	"$(INTDIR)\arch\sdl\menu_plus4cart.obj" \
	"$(INTDIR)\arch\sdl\menu_plus4hw.obj" \
	"$(INTDIR)\arch\sdl\menu_printer.obj" \
	"$(INTDIR)\arch\sdl\menu_ram.obj" \
	"$(INTDIR)\arch\sdl\menu_reset.obj" \
	"$(INTDIR)\arch\sdl\menu_rom.obj" \
	"$(INTDIR)\arch\sdl\menu_rs232.obj" \
	"$(INTDIR)\arch\sdl\menu_scpu64hw.obj" \
	"$(INTDIR)\arch\sdl\menu_screenshot.obj" \
	"$(INTDIR)\arch\sdl\menu_settings.obj" \
	"$(INTDIR)\arch\sdl\menu_sid.obj" \
	"$(INTDIR)\arch\sdl\menu_snapshot.obj" \
	"$(INTDIR)\arch\sdl\menu_sound.obj" \
	"$(INTDIR)\arch\sdl\menu_speed.obj" \
	"$(INTDIR)\arch\sdl\menu_tape.obj" \
	"$(INTDIR)\arch\sdl\menu_tfe.obj" \
	"$(INTDIR)\arch\sdl\menu_vic20cart.obj" \
	"$(INTDIR)\arch\sdl\menu_vic20hw.obj" \
	"$(INTDIR)\arch\sdl\menu_video.obj" \
	"$(INTDIR)\arch\sdl\mousedrv.obj" \
	"$(INTDIR)\arch\sdl\rawnetarch.obj" \
	"$(INTDIR)\arch\sdl\rs232.obj" \
	"$(INTDIR)\arch\sdl\rs232dev.obj" \
	"$(INTDIR)\arch\sdl\rs232net.obj" \
	"$(INTDIR)\arch\sdl\sdlmain.obj" \
	"$(INTDIR)\arch\sdl\signals.obj" \
	"$(INTDIR)\arch\sdl\ui.obj" \
	"$(INTDIR)\arch\sdl\uicmdline.obj" \
	"$(INTDIR)\arch\sdl\uifilereq.obj" \
	"$(INTDIR)\arch\sdl\uihotkey.obj" \
	"$(INTDIR)\arch\sdl\uimenu.obj" \
	"$(INTDIR)\arch\sdl\uimon.obj" \
	"$(INTDIR)\arch\sdl\uimsgbox.obj" \
	"$(INTDIR)\arch\sdl\uipause.obj" \
	"$(INTDIR)\arch\sdl\uipoll.obj" \
	"$(INTDIR)\arch\sdl\uistatusbar.obj" \
	"$(INTDIR)\arch\sdl\video.obj" \
	"$(INTDIR)\arch\sdl\vkbd.obj" \
	"$(INTDIR)\arch\sdl\vsidui.obj" \
	"$(INTDIR)\arch\sdl\vsyncarch.obj" \
	"$(INTDIR)\arch\sdl\x128_ui.obj" \
	"$(INTDIR)\arch\sdl\x64_ui.obj" \
	"$(INTDIR)\arch\sdl\x64dtv_ui.obj" \
	"$(INTDIR)\arch\sdl\x64sc_ui.obj" \
	"$(INTDIR)\arch\sdl\xcbm2_ui.obj" \
	"$(INTDIR)\arch\sdl\xpet_ui.obj" \
	"$(INTDIR)\arch\sdl\xplus4_ui.obj" \
	"$(INTDIR)\arch\sdl\xscpu64_ui.obj" \
	"$(INTDIR)\arch\sdl\xvic_ui.obj" \
	".\libsbase\Debug\base.lib" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "arch - Win32 Release" || "$(CFG)" == "arch - Win32 Debug"

!IF  "$(CFG)" == "arch - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "arch - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "arch - Win32 Release"

"geninfocontrib - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\geninfocontrib.mak" CFG="geninfocontrib - Win32 Release" 
   cd "."

"geninfocontrib - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\geninfocontrib.mak" CFG="geninfocontrib - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "arch - Win32 Debug"

"geninfocontrib - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\geninfocontrib.mak" CFG="geninfocontrib - Win32 Debug" 
   cd "."

"geninfocontrib - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\geninfocontrib.mak" CFG="geninfocontrib - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "arch - Win32 Release"

"gentranslate - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\gentranslate.mak" CFG="gentranslate - Win32 Release" 
   cd "."

"gentranslate - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\gentranslate.mak" CFG="gentranslate - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "arch - Win32 Debug"

"gentranslate - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\gentranslate.mak" CFG="gentranslate - Win32 Debug" 
   cd "."

"gentranslate - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\gentranslate.mak" CFG="gentranslate - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\arch\sdl\archdep.c

"$(INTDIR)\arch\sdl\archdep.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\blockdev.c

"$(INTDIR)\arch\sdl\blockdev.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\catweaselmkiii.c

"$(INTDIR)\arch\sdl\catweaselmkiii.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\console.c

"$(INTDIR)\arch\sdl\console.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\dynlib.c

"$(INTDIR)\arch\sdl\dynlib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\fullscreen.c

"$(INTDIR)\arch\sdl\fullscreen.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\hardsid.c

"$(INTDIR)\arch\sdl\hardsid.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\joy.c

"$(INTDIR)\arch\sdl\joy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\kbd.c

"$(INTDIR)\arch\sdl\kbd.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\lightpendrv.c

"$(INTDIR)\arch\sdl\lightpendrv.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_c128hw.c

"$(INTDIR)\arch\sdl\menu_c128hw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_c64_common_expansions.c

"$(INTDIR)\arch\sdl\menu_c64_common_expansions.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_c64_expansions.c

"$(INTDIR)\arch\sdl\menu_c64_expansions.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_c64cart.c

"$(INTDIR)\arch\sdl\menu_c64cart.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_c64dtvhw.c

"$(INTDIR)\arch\sdl\menu_c64dtvhw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_c64hw.c

"$(INTDIR)\arch\sdl\menu_c64hw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_c64model.c

"$(INTDIR)\arch\sdl\menu_c64model.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_cbm2cart.c

"$(INTDIR)\arch\sdl\menu_cbm2cart.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_cbm2hw.c

"$(INTDIR)\arch\sdl\menu_cbm2hw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_common.c

"$(INTDIR)\arch\sdl\menu_common.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_debug.c

"$(INTDIR)\arch\sdl\menu_debug.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_drive.c

"$(INTDIR)\arch\sdl\menu_drive.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_drive_rom.c

"$(INTDIR)\arch\sdl\menu_drive_rom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_ffmpeg.c

"$(INTDIR)\arch\sdl\menu_ffmpeg.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_help.c

"$(INTDIR)\arch\sdl\menu_help.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_joystick.c

"$(INTDIR)\arch\sdl\menu_joystick.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_lightpen.c

"$(INTDIR)\arch\sdl\menu_lightpen.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_midi.c

"$(INTDIR)\arch\sdl\menu_midi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_mouse.c

"$(INTDIR)\arch\sdl\menu_mouse.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_network.c

"$(INTDIR)\arch\sdl\menu_network.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_petcart.c

"$(INTDIR)\arch\sdl\menu_petcart.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_pethw.c

"$(INTDIR)\arch\sdl\menu_pethw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_plus4cart.c

"$(INTDIR)\arch\sdl\menu_plus4cart.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_plus4hw.c

"$(INTDIR)\arch\sdl\menu_plus4hw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_printer.c

"$(INTDIR)\arch\sdl\menu_printer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_ram.c

"$(INTDIR)\arch\sdl\menu_ram.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_reset.c

"$(INTDIR)\arch\sdl\menu_reset.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_rom.c

"$(INTDIR)\arch\sdl\menu_rom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_rs232.c

"$(INTDIR)\arch\sdl\menu_rs232.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_scpu64hw.c

"$(INTDIR)\arch\sdl\menu_scpu64hw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_screenshot.c

"$(INTDIR)\arch\sdl\menu_screenshot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_settings.c

"$(INTDIR)\arch\sdl\menu_settings.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_sid.c

"$(INTDIR)\arch\sdl\menu_sid.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_snapshot.c

"$(INTDIR)\arch\sdl\menu_snapshot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_sound.c

"$(INTDIR)\arch\sdl\menu_sound.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_speed.c

"$(INTDIR)\arch\sdl\menu_speed.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_tape.c

"$(INTDIR)\arch\sdl\menu_tape.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_tfe.c

"$(INTDIR)\arch\sdl\menu_tfe.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_vic20cart.c

"$(INTDIR)\arch\sdl\menu_vic20cart.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_vic20hw.c

"$(INTDIR)\arch\sdl\menu_vic20hw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\menu_video.c

"$(INTDIR)\arch\sdl\menu_video.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\mousedrv.c

"$(INTDIR)\arch\sdl\mousedrv.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\rawnetarch.c

"$(INTDIR)\arch\sdl\rawnetarch.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\rs232.c

"$(INTDIR)\arch\sdl\rs232.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\rs232dev.c

"$(INTDIR)\arch\sdl\rs232dev.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\rs232net.c

"$(INTDIR)\arch\sdl\rs232net.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\sdlmain.c

"$(INTDIR)\arch\sdl\sdlmain.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\signals.c

"$(INTDIR)\arch\sdl\signals.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\ui.c

"$(INTDIR)\arch\sdl\ui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\uicmdline.c

"$(INTDIR)\arch\sdl\uicmdline.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\uifilereq.c

"$(INTDIR)\arch\sdl\uifilereq.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\uihotkey.c

"$(INTDIR)\arch\sdl\uihotkey.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\uimenu.c

"$(INTDIR)\arch\sdl\uimenu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\uimon.c

"$(INTDIR)\arch\sdl\uimon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\uimsgbox.c

"$(INTDIR)\arch\sdl\uimsgbox.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\uipause.c

"$(INTDIR)\arch\sdl\uipause.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\uipoll.c

"$(INTDIR)\arch\sdl\uipoll.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\uistatusbar.c

"$(INTDIR)\arch\sdl\uistatusbar.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\video.c

"$(INTDIR)\arch\sdl\video.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\vkbd.c

"$(INTDIR)\arch\sdl\vkbd.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\vsidui.c

"$(INTDIR)\arch\sdl\vsidui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\vsyncarch.c

"$(INTDIR)\arch\sdl\vsyncarch.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\x128_ui.c

"$(INTDIR)\arch\sdl\x128_ui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\x64_ui.c

"$(INTDIR)\arch\sdl\x64_ui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\x64dtv_ui.c

"$(INTDIR)\arch\sdl\x64dtv_ui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\x64sc_ui.c

"$(INTDIR)\arch\sdl\x64sc_ui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\xcbm2_ui.c

"$(INTDIR)\arch\sdl\xcbm2_ui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\xpet_ui.c

"$(INTDIR)\arch\sdl\xpet_ui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\xplus4_ui.c

"$(INTDIR)\arch\sdl\xplus4_ui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\xscpu64_ui.c

"$(INTDIR)\arch\sdl\xscpu64_ui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\arch\sdl\xvic_ui.c

"$(INTDIR)\arch\sdl\xvic_ui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


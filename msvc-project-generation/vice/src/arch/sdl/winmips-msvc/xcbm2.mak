# Microsoft Developer Studio Generated NMAKE File, Based on xcbm2.dsp
!IF "$(CFG)" == ""
CFG=xcbm2 - Win32 Release
!MESSAGE No configuration specified. Defaulting to xcbm2 - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "xcbm2 - Win32 Release" && "$(CFG)" != "xcbm2 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "xcbm2.mak" CFG="xcbm2 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "xcbm2 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "xcbm2 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

OUTDIR=.\..\..\..\..\data
INTDIR=.\libs\xcbm2\Release
# Begin Custom Macros
OutDir=.\..\..\..\..\data
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\xcbm2.exe" 

!ELSE 

ALL : "arch - Win32 Release" "base - Win32 Release" "cbm2 - Win32 Release" "cbm2common - Win32 Release" "core - Win32 Release" "crtc - Win32 Release" "diskimage - Win32 Release" "drive - Win32 Release" "fileio - Win32 Release" "fsdevice - Win32 Release" "gfxoutputdrv - Win32 Release" "iecieee - Win32 Release" "ieee - Win32 Release" "imagecontents - Win32 Release" "monitor - Win32 Release" "p64 - Win32 Release" "parallel - Win32 Release" "platform - Win32 Release" "printerdrv - Win32 Release" "raster - Win32 Release" "rs232drv - Win32 Release" "rtc - Win32 Release" "serial - Win32 Release" "sid - Win32 Release" "sounddrv - Win32 Release" "tape - Win32 Release" "userport - Win32 Release" "vdrive - Win32 Release" "video - Win32 Release" "$(OUTDIR)\xcbm2.exe" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"video - Win32 ReleaseCLEAN" "vdrive - Win32 ReleaseCLEAN" "userport - Win32 ReleaseCLEAN" "tape - Win32 ReleaseCLEAN" "sounddrv - Win32 ReleaseCLEAN" "sid - Win32 ReleaseCLEAN" "serial - Win32 ReleaseCLEAN" "rtc - Win32 ReleaseCLEAN" "rs232drv - Win32 ReleaseCLEAN" "raster - Win32 ReleaseCLEAN" "printerdrv - Win32 ReleaseCLEAN" "platform - Win32 ReleaseCLEAN" "parallel - Win32 ReleaseCLEAN" "p64 - Win32 ReleaseCLEAN" "monitor - Win32 ReleaseCLEAN" "imagecontents - Win32 ReleaseCLEAN" "ieee - Win32 ReleaseCLEAN" "iecieee - Win32 ReleaseCLEAN" "gfxoutputdrv - Win32 ReleaseCLEAN" "fsdevice - Win32 ReleaseCLEAN" "fileio - Win32 ReleaseCLEAN" "drive - Win32 ReleaseCLEAN" "diskimage - Win32 ReleaseCLEAN" "crtc - Win32 ReleaseCLEAN" "core - Win32 ReleaseCLEAN" "cbm2common - Win32 ReleaseCLEAN" "cbm2 - Win32 ReleaseCLEAN" "base - Win32 ReleaseCLEAN" "arch - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(OUTDIR)\xcbm2.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\platform "/I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\xcbm2.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xcbm2.bsc" 
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib winmm.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib version.lib SDLmain.lib SDL.lib opengl32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\xcbm2.pdb" /machine:MIPS /out:"$(OUTDIR)\xcbm2.exe" 
LINK32_OBJS= \
	"$(INTDIR)\main.obj" \
	".\libsarch\Release\arch.lib" \
	".\libsbase\Release\base.lib" \
	".\libscbm2\Release\cbm2.lib" \
	".\libscbm2common\Release\cbm2common.lib" \
	".\libscore\Release\core.lib" \
	".\libscrtc\Release\crtc.lib" \
	".\libsdiskimage\Release\diskimage.lib" \
	".\libsdrive\Release\drive.lib" \
	".\libsfileio\Release\fileio.lib" \
	".\libsfsdevice\Release\fsdevice.lib" \
	".\libsgfxoutputdrv\Release\gfxoutputdrv.lib" \
	".\libsiecieee\Release\iecieee.lib" \
	".\libsieee\Release\ieee.lib" \
	".\libsimagecontents\Release\imagecontents.lib" \
	".\libsmonitor\Release\monitor.lib" \
	".\libsp64\Release\p64.lib" \
	".\libsparallel\Release\parallel.lib" \
	".\libsplatform\Release\platform.lib" \
	".\libsprinterdrv\Release\printerdrv.lib" \
	".\libsraster\Release\raster.lib" \
	".\libsrs232drv\Release\rs232drv.lib" \
	".\libsrtc\Release\rtc.lib" \
	".\libsserial\Release\serial.lib" \
	".\libssid\Release\sid.lib" \
	".\libssounddrv\Release\sounddrv.lib" \
	".\libstape\Release\tape.lib" \
	".\libsuserport\Release\userport.lib" \
	".\libsvdrive\Release\vdrive.lib" \
	".\libsvideo\Release\video.lib" \


"$(OUTDIR)\xcbm2.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

OUTDIR=.\..\..\..\..\data
INTDIR=.\libs\xcbm2\Debug
# Begin Custom Macros
OutDir=.\..\..\..\..\data
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\xcbm2.exe" 

!ELSE 

ALL : "arch - Win32 Debug" "base - Win32 Debug" "cbm2 - Win32 Debug" "cbm2common - Win32 Debug" "core - Win32 Debug" "crtc - Win32 Debug" "diskimage - Win32 Debug" "drive - Win32 Debug" "fileio - Win32 Debug" "fsdevice - Win32 Debug" "gfxoutputdrv - Win32 Debug" "iecieee - Win32 Debug" "ieee - Win32 Debug" "imagecontents - Win32 Debug" "monitor - Win32 Debug" "p64 - Win32 Debug" "parallel - Win32 Debug" "platform - Win32 Debug" "printerdrv - Win32 Debug" "raster - Win32 Debug" "rs232drv - Win32 Debug" "rtc - Win32 Debug" "serial - Win32 Debug" "sid - Win32 Debug" "sounddrv - Win32 Debug" "tape - Win32 Debug" "userport - Win32 Debug" "vdrive - Win32 Debug" "video - Win32 Debug" "$(OUTDIR)\xcbm2.exe" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"video - Win32 DebugCLEAN" "vdrive - Win32 DebugCLEAN" "userport - Win32 DebugCLEAN" "tape - Win32 DebugCLEAN" "sounddrv - Win32 DebugCLEAN" "sid - Win32 DebugCLEAN" "serial - Win32 DebugCLEAN" "rtc - Win32 DebugCLEAN" "rs232drv - Win32 DebugCLEAN" "raster - Win32 DebugCLEAN" "printerdrv - Win32 DebugCLEAN" "platform - Win32 DebugCLEAN" "parallel - Win32 DebugCLEAN" "p64 - Win32 DebugCLEAN" "monitor - Win32 DebugCLEAN" "imagecontents - Win32 DebugCLEAN" "ieee - Win32 DebugCLEAN" "iecieee - Win32 DebugCLEAN" "gfxoutputdrv - Win32 DebugCLEAN" "fsdevice - Win32 DebugCLEAN" "fileio - Win32 DebugCLEAN" "drive - Win32 DebugCLEAN" "diskimage - Win32 DebugCLEAN" "crtc - Win32 DebugCLEAN" "core - Win32 DebugCLEAN" "cbm2common - Win32 DebugCLEAN" "cbm2 - Win32 DebugCLEAN" "base - Win32 DebugCLEAN" "arch - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(OUTDIR)\xcbm2.exe"
	-@erase "$(OUTDIR)\xcbm2.ilk"
	-@erase "$(OUTDIR)\xcbm2.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\platform "/I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\xcbm2.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32 
RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\xcbm2.bsc" 
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib winmm.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib version.lib SDLmain.lib SDL.lib opengl32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\xcbm2.pdb" /debug /machine:MIPS /nodefaultlib:"msvcrt.lib" /out:"$(OUTDIR)\xcbm2.exe" /pdbtype:sept
LINK32_OBJS= \
	"$(INTDIR)\main.obj" \
	".\libsarch\Debug\arch.lib" \
	".\libsbase\Debug\base.lib" \
	".\libscbm2\Debug\cbm2.lib" \
	".\libscbm2common\Debug\cbm2common.lib" \
	".\libscore\Debug\core.lib" \
	".\libscrtc\Debug\crtc.lib" \
	".\libsdiskimage\Debug\diskimage.lib" \
	".\libsdrive\Debug\drive.lib" \
	".\libsfileio\Debug\fileio.lib" \
	".\libsfsdevice\Debug\fsdevice.lib" \
	".\libsgfxoutputdrv\Debug\gfxoutputdrv.lib" \
	".\libsiecieee\Debug\iecieee.lib" \
	".\libsieee\Debug\ieee.lib" \
	".\libsimagecontents\Debug\imagecontents.lib" \
	".\libsmonitor\Debug\monitor.lib" \
	".\libsp64\Debug\p64.lib" \
	".\libsparallel\Debug\parallel.lib" \
	".\libsplatform\Debug\platform.lib" \
	".\libsprinterdrv\Debug\printerdrv.lib" \
	".\libsraster\Debug\raster.lib" \
	".\libsrs232drv\Debug\rs232drv.lib" \
	".\libsrtc\Debug\rtc.lib" \
	".\libsserial\Debug\serial.lib" \
	".\libssid\Debug\sid.lib" \
	".\libssounddrv\Debug\sounddrv.lib" \
	".\libstape\Debug\tape.lib" \
	".\libsuserport\Debug\userport.lib" \
	".\libsvdrive\Debug\vdrive.lib" \
	".\libsvideo\Debug\video.lib" \


"$(OUTDIR)\xcbm2.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "xcbm2 - Win32 Release" || "$(CFG)" == "xcbm2 - Win32 Debug"

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"arch - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\arch.mak" CFG="arch - Win32 Release" 
   cd "."

"arch - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\arch.mak" CFG="arch - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"arch - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\arch.mak" CFG="arch - Win32 Debug" 
   cd "."

"arch - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\arch.mak" CFG="arch - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"cbm2 - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\cbm2.mak" CFG="cbm2 - Win32 Release" 
   cd "."

"cbm2 - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\cbm2.mak" CFG="cbm2 - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"cbm2 - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\cbm2.mak" CFG="cbm2 - Win32 Debug" 
   cd "."

"cbm2 - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\cbm2.mak" CFG="cbm2 - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"cbm2common - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\cbm2common.mak" CFG="cbm2common - Win32 Release" 
   cd "."

"cbm2common - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\cbm2common.mak" CFG="cbm2common - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"cbm2common - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\cbm2common.mak" CFG="cbm2common - Win32 Debug" 
   cd "."

"cbm2common - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\cbm2common.mak" CFG="cbm2common - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"core - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\core.mak" CFG="core - Win32 Release" 
   cd "."

"core - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\core.mak" CFG="core - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"core - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\core.mak" CFG="core - Win32 Debug" 
   cd "."

"core - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\core.mak" CFG="core - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"crtc - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\crtc.mak" CFG="crtc - Win32 Release" 
   cd "."

"crtc - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\crtc.mak" CFG="crtc - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"crtc - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\crtc.mak" CFG="crtc - Win32 Debug" 
   cd "."

"crtc - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\crtc.mak" CFG="crtc - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"diskimage - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\diskimage.mak" CFG="diskimage - Win32 Release" 
   cd "."

"diskimage - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\diskimage.mak" CFG="diskimage - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"diskimage - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\diskimage.mak" CFG="diskimage - Win32 Debug" 
   cd "."

"diskimage - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\diskimage.mak" CFG="diskimage - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"drive - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\drive.mak" CFG="drive - Win32 Release" 
   cd "."

"drive - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\drive.mak" CFG="drive - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"drive - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\drive.mak" CFG="drive - Win32 Debug" 
   cd "."

"drive - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\drive.mak" CFG="drive - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"fileio - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\fileio.mak" CFG="fileio - Win32 Release" 
   cd "."

"fileio - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\fileio.mak" CFG="fileio - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"fileio - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\fileio.mak" CFG="fileio - Win32 Debug" 
   cd "."

"fileio - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\fileio.mak" CFG="fileio - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"fsdevice - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\fsdevice.mak" CFG="fsdevice - Win32 Release" 
   cd "."

"fsdevice - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\fsdevice.mak" CFG="fsdevice - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"fsdevice - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\fsdevice.mak" CFG="fsdevice - Win32 Debug" 
   cd "."

"fsdevice - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\fsdevice.mak" CFG="fsdevice - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"gfxoutputdrv - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\gfxoutputdrv.mak" CFG="gfxoutputdrv - Win32 Release" 
   cd "."

"gfxoutputdrv - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\gfxoutputdrv.mak" CFG="gfxoutputdrv - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"gfxoutputdrv - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\gfxoutputdrv.mak" CFG="gfxoutputdrv - Win32 Debug" 
   cd "."

"gfxoutputdrv - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\gfxoutputdrv.mak" CFG="gfxoutputdrv - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"iecieee - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\iecieee.mak" CFG="iecieee - Win32 Release" 
   cd "."

"iecieee - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\iecieee.mak" CFG="iecieee - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"iecieee - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\iecieee.mak" CFG="iecieee - Win32 Debug" 
   cd "."

"iecieee - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\iecieee.mak" CFG="iecieee - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"ieee - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\ieee.mak" CFG="ieee - Win32 Release" 
   cd "."

"ieee - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\ieee.mak" CFG="ieee - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"ieee - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\ieee.mak" CFG="ieee - Win32 Debug" 
   cd "."

"ieee - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\ieee.mak" CFG="ieee - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"imagecontents - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\imagecontents.mak" CFG="imagecontents - Win32 Release" 
   cd "."

"imagecontents - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\imagecontents.mak" CFG="imagecontents - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"imagecontents - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\imagecontents.mak" CFG="imagecontents - Win32 Debug" 
   cd "."

"imagecontents - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\imagecontents.mak" CFG="imagecontents - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"monitor - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\monitor.mak" CFG="monitor - Win32 Release" 
   cd "."

"monitor - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\monitor.mak" CFG="monitor - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"monitor - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\monitor.mak" CFG="monitor - Win32 Debug" 
   cd "."

"monitor - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\monitor.mak" CFG="monitor - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"p64 - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\p64.mak" CFG="p64 - Win32 Release" 
   cd "."

"p64 - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\p64.mak" CFG="p64 - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"p64 - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\p64.mak" CFG="p64 - Win32 Debug" 
   cd "."

"p64 - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\p64.mak" CFG="p64 - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"parallel - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\parallel.mak" CFG="parallel - Win32 Release" 
   cd "."

"parallel - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\parallel.mak" CFG="parallel - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"parallel - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\parallel.mak" CFG="parallel - Win32 Debug" 
   cd "."

"parallel - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\parallel.mak" CFG="parallel - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"platform - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\platform.mak" CFG="platform - Win32 Release" 
   cd "."

"platform - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\platform.mak" CFG="platform - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"platform - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\platform.mak" CFG="platform - Win32 Debug" 
   cd "."

"platform - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\platform.mak" CFG="platform - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"printerdrv - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\printerdrv.mak" CFG="printerdrv - Win32 Release" 
   cd "."

"printerdrv - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\printerdrv.mak" CFG="printerdrv - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"printerdrv - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\printerdrv.mak" CFG="printerdrv - Win32 Debug" 
   cd "."

"printerdrv - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\printerdrv.mak" CFG="printerdrv - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"raster - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\raster.mak" CFG="raster - Win32 Release" 
   cd "."

"raster - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\raster.mak" CFG="raster - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"raster - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\raster.mak" CFG="raster - Win32 Debug" 
   cd "."

"raster - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\raster.mak" CFG="raster - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"rs232drv - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\rs232drv.mak" CFG="rs232drv - Win32 Release" 
   cd "."

"rs232drv - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\rs232drv.mak" CFG="rs232drv - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"rs232drv - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\rs232drv.mak" CFG="rs232drv - Win32 Debug" 
   cd "."

"rs232drv - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\rs232drv.mak" CFG="rs232drv - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"rtc - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\rtc.mak" CFG="rtc - Win32 Release" 
   cd "."

"rtc - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\rtc.mak" CFG="rtc - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"rtc - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\rtc.mak" CFG="rtc - Win32 Debug" 
   cd "."

"rtc - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\rtc.mak" CFG="rtc - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"serial - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\serial.mak" CFG="serial - Win32 Release" 
   cd "."

"serial - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\serial.mak" CFG="serial - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"serial - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\serial.mak" CFG="serial - Win32 Debug" 
   cd "."

"serial - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\serial.mak" CFG="serial - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"sid - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\sid.mak" CFG="sid - Win32 Release" 
   cd "."

"sid - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\sid.mak" CFG="sid - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"sid - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\sid.mak" CFG="sid - Win32 Debug" 
   cd "."

"sid - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\sid.mak" CFG="sid - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"sounddrv - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\sounddrv.mak" CFG="sounddrv - Win32 Release" 
   cd "."

"sounddrv - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\sounddrv.mak" CFG="sounddrv - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"sounddrv - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\sounddrv.mak" CFG="sounddrv - Win32 Debug" 
   cd "."

"sounddrv - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\sounddrv.mak" CFG="sounddrv - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"tape - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\tape.mak" CFG="tape - Win32 Release" 
   cd "."

"tape - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\tape.mak" CFG="tape - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"tape - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\tape.mak" CFG="tape - Win32 Debug" 
   cd "."

"tape - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\tape.mak" CFG="tape - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"userport - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\userport.mak" CFG="userport - Win32 Release" 
   cd "."

"userport - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\userport.mak" CFG="userport - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"userport - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\userport.mak" CFG="userport - Win32 Debug" 
   cd "."

"userport - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\userport.mak" CFG="userport - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"vdrive - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\vdrive.mak" CFG="vdrive - Win32 Release" 
   cd "."

"vdrive - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\vdrive.mak" CFG="vdrive - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"vdrive - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\vdrive.mak" CFG="vdrive - Win32 Debug" 
   cd "."

"vdrive - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\vdrive.mak" CFG="vdrive - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "xcbm2 - Win32 Release"

"video - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\video.mak" CFG="video - Win32 Release" 
   cd "."

"video - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\video.mak" CFG="video - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "xcbm2 - Win32 Debug"

"video - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\video.mak" CFG="video - Win32 Debug" 
   cd "."

"video - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\video.mak" CFG="video - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\main.c

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


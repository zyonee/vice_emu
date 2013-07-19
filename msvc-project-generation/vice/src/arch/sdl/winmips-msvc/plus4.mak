# Microsoft Developer Studio Generated NMAKE File, Based on plus4.dsp
!IF "$(CFG)" == ""
CFG=plus4 - Win32 Release
!MESSAGE No configuration specified. Defaulting to plus4 - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "plus4 - Win32 Release" && "$(CFG)" != "plus4 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "plus4.mak" CFG="plus4 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "plus4 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "plus4 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "plus4 - Win32 Release"

OUTDIR=.\libs\plus4\Release
INTDIR=.\libs\plus4\Release
# Begin Custom Macros
OutDir=.\libs\plus4\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\plus4.lib" 

!ELSE 

ALL : "base - Win32 Release" "$(OUTDIR)\plus4.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\plus4\digiblaster.obj"
	-@erase "$(INTDIR)\plus4\plus4-cmdline-options.obj"
	-@erase "$(INTDIR)\plus4\plus4-resources.obj"
	-@erase "$(INTDIR)\plus4\plus4-sidcart.obj"
	-@erase "$(INTDIR)\plus4\plus4-snapshot.obj"
	-@erase "$(INTDIR)\plus4\plus4.obj"
	-@erase "$(INTDIR)\plus4\plus4acia.obj"
	-@erase "$(INTDIR)\plus4\plus4bus.obj"
	-@erase "$(INTDIR)\plus4\plus4cart.obj"
	-@erase "$(INTDIR)\plus4\plus4datasette.obj"
	-@erase "$(INTDIR)\plus4\plus4drive.obj"
	-@erase "$(INTDIR)\plus4\plus4iec.obj"
	-@erase "$(INTDIR)\plus4\plus4mem.obj"
	-@erase "$(INTDIR)\plus4\plus4memcsory256k.obj"
	-@erase "$(INTDIR)\plus4\plus4memhannes256k.obj"
	-@erase "$(INTDIR)\plus4\plus4memlimit.obj"
	-@erase "$(INTDIR)\plus4\plus4memrom.obj"
	-@erase "$(INTDIR)\plus4\plus4memsnapshot.obj"
	-@erase "$(INTDIR)\plus4\plus4model.obj"
	-@erase "$(INTDIR)\plus4\plus4parallel.obj"
	-@erase "$(INTDIR)\plus4\plus4pio1.obj"
	-@erase "$(INTDIR)\plus4\plus4pio2.obj"
	-@erase "$(INTDIR)\plus4\plus4printer.obj"
	-@erase "$(INTDIR)\plus4\plus4rom.obj"
	-@erase "$(INTDIR)\plus4\plus4romset.obj"
	-@erase "$(INTDIR)\plus4\plus4speech.obj"
	-@erase "$(INTDIR)\plus4\plus4tcbm.obj"
	-@erase "$(INTDIR)\plus4\plus4video.obj"
	-@erase "$(INTDIR)\plus4\ted-badline.obj"
	-@erase "$(INTDIR)\plus4\ted-cmdline-options.obj"
	-@erase "$(INTDIR)\plus4\ted-color.obj"
	-@erase "$(INTDIR)\plus4\ted-draw.obj"
	-@erase "$(INTDIR)\plus4\ted-fetch.obj"
	-@erase "$(INTDIR)\plus4\ted-irq.obj"
	-@erase "$(INTDIR)\plus4\ted-mem.obj"
	-@erase "$(INTDIR)\plus4\ted-resources.obj"
	-@erase "$(INTDIR)\plus4\ted-snapshot.obj"
	-@erase "$(INTDIR)\plus4\ted-sound.obj"
	-@erase "$(INTDIR)\plus4\ted-timer.obj"
	-@erase "$(INTDIR)\plus4\ted-timing.obj"
	-@erase "$(INTDIR)\plus4\ted.obj"
	-@erase "$(INTDIR)\plus4\plus4cpu.obj"
	-@erase "$(OUTDIR)\plus4.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\core "/I "..\..\..\drive "/I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\raster "/I "..\..\..\sid "/I "..\..\..\tape "/I "..\..\..\video "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\plus4.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\plus4.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\plus4.lib" 
LIB32_OBJS= \
	"$(INTDIR)\plus4\digiblaster.obj" \
	"$(INTDIR)\plus4\plus4-cmdline-options.obj" \
	"$(INTDIR)\plus4\plus4-resources.obj" \
	"$(INTDIR)\plus4\plus4-sidcart.obj" \
	"$(INTDIR)\plus4\plus4-snapshot.obj" \
	"$(INTDIR)\plus4\plus4.obj" \
	"$(INTDIR)\plus4\plus4acia.obj" \
	"$(INTDIR)\plus4\plus4bus.obj" \
	"$(INTDIR)\plus4\plus4cart.obj" \
	"$(INTDIR)\plus4\plus4datasette.obj" \
	"$(INTDIR)\plus4\plus4drive.obj" \
	"$(INTDIR)\plus4\plus4iec.obj" \
	"$(INTDIR)\plus4\plus4mem.obj" \
	"$(INTDIR)\plus4\plus4memcsory256k.obj" \
	"$(INTDIR)\plus4\plus4memhannes256k.obj" \
	"$(INTDIR)\plus4\plus4memlimit.obj" \
	"$(INTDIR)\plus4\plus4memrom.obj" \
	"$(INTDIR)\plus4\plus4memsnapshot.obj" \
	"$(INTDIR)\plus4\plus4model.obj" \
	"$(INTDIR)\plus4\plus4parallel.obj" \
	"$(INTDIR)\plus4\plus4pio1.obj" \
	"$(INTDIR)\plus4\plus4pio2.obj" \
	"$(INTDIR)\plus4\plus4printer.obj" \
	"$(INTDIR)\plus4\plus4rom.obj" \
	"$(INTDIR)\plus4\plus4romset.obj" \
	"$(INTDIR)\plus4\plus4speech.obj" \
	"$(INTDIR)\plus4\plus4tcbm.obj" \
	"$(INTDIR)\plus4\plus4video.obj" \
	"$(INTDIR)\plus4\ted-badline.obj" \
	"$(INTDIR)\plus4\ted-cmdline-options.obj" \
	"$(INTDIR)\plus4\ted-color.obj" \
	"$(INTDIR)\plus4\ted-draw.obj" \
	"$(INTDIR)\plus4\ted-fetch.obj" \
	"$(INTDIR)\plus4\ted-irq.obj" \
	"$(INTDIR)\plus4\ted-mem.obj" \
	"$(INTDIR)\plus4\ted-resources.obj" \
	"$(INTDIR)\plus4\ted-snapshot.obj" \
	"$(INTDIR)\plus4\ted-sound.obj" \
	"$(INTDIR)\plus4\ted-timer.obj" \
	"$(INTDIR)\plus4\ted-timing.obj" \
	"$(INTDIR)\plus4\ted.obj" \
	"$(INTDIR)\plus4\plus4cpu.obj" \
	".\libsbase\Release\base.lib" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\plus4\Debug
INTDIR=.\libs\plus4\Debug
# Begin Custom Macros
OutDir=.\libs\plus4\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\plus4.lib" 

!ELSE 

ALL : "base - Win32 Debug" "$(OUTDIR)\plus4.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\plus4\digiblaster.obj"
	-@erase "$(INTDIR)\plus4\plus4-cmdline-options.obj"
	-@erase "$(INTDIR)\plus4\plus4-resources.obj"
	-@erase "$(INTDIR)\plus4\plus4-sidcart.obj"
	-@erase "$(INTDIR)\plus4\plus4-snapshot.obj"
	-@erase "$(INTDIR)\plus4\plus4.obj"
	-@erase "$(INTDIR)\plus4\plus4acia.obj"
	-@erase "$(INTDIR)\plus4\plus4bus.obj"
	-@erase "$(INTDIR)\plus4\plus4cart.obj"
	-@erase "$(INTDIR)\plus4\plus4datasette.obj"
	-@erase "$(INTDIR)\plus4\plus4drive.obj"
	-@erase "$(INTDIR)\plus4\plus4iec.obj"
	-@erase "$(INTDIR)\plus4\plus4mem.obj"
	-@erase "$(INTDIR)\plus4\plus4memcsory256k.obj"
	-@erase "$(INTDIR)\plus4\plus4memhannes256k.obj"
	-@erase "$(INTDIR)\plus4\plus4memlimit.obj"
	-@erase "$(INTDIR)\plus4\plus4memrom.obj"
	-@erase "$(INTDIR)\plus4\plus4memsnapshot.obj"
	-@erase "$(INTDIR)\plus4\plus4model.obj"
	-@erase "$(INTDIR)\plus4\plus4parallel.obj"
	-@erase "$(INTDIR)\plus4\plus4pio1.obj"
	-@erase "$(INTDIR)\plus4\plus4pio2.obj"
	-@erase "$(INTDIR)\plus4\plus4printer.obj"
	-@erase "$(INTDIR)\plus4\plus4rom.obj"
	-@erase "$(INTDIR)\plus4\plus4romset.obj"
	-@erase "$(INTDIR)\plus4\plus4speech.obj"
	-@erase "$(INTDIR)\plus4\plus4tcbm.obj"
	-@erase "$(INTDIR)\plus4\plus4video.obj"
	-@erase "$(INTDIR)\plus4\ted-badline.obj"
	-@erase "$(INTDIR)\plus4\ted-cmdline-options.obj"
	-@erase "$(INTDIR)\plus4\ted-color.obj"
	-@erase "$(INTDIR)\plus4\ted-draw.obj"
	-@erase "$(INTDIR)\plus4\ted-fetch.obj"
	-@erase "$(INTDIR)\plus4\ted-irq.obj"
	-@erase "$(INTDIR)\plus4\ted-mem.obj"
	-@erase "$(INTDIR)\plus4\ted-resources.obj"
	-@erase "$(INTDIR)\plus4\ted-snapshot.obj"
	-@erase "$(INTDIR)\plus4\ted-sound.obj"
	-@erase "$(INTDIR)\plus4\ted-timer.obj"
	-@erase "$(INTDIR)\plus4\ted-timing.obj"
	-@erase "$(INTDIR)\plus4\ted.obj"
	-@erase "$(INTDIR)\plus4\plus4cpu.obj"
	-@erase "$(OUTDIR)\plus4.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\core "/I "..\..\..\drive "/I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\raster "/I "..\..\..\sid "/I "..\..\..\tape "/I "..\..\..\video "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\plus4.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\plus4.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\plus4.lib" 
LIB32_OBJS= \
	"$(INTDIR)\plus4\digiblaster.obj" \
	"$(INTDIR)\plus4\plus4-cmdline-options.obj" \
	"$(INTDIR)\plus4\plus4-resources.obj" \
	"$(INTDIR)\plus4\plus4-sidcart.obj" \
	"$(INTDIR)\plus4\plus4-snapshot.obj" \
	"$(INTDIR)\plus4\plus4.obj" \
	"$(INTDIR)\plus4\plus4acia.obj" \
	"$(INTDIR)\plus4\plus4bus.obj" \
	"$(INTDIR)\plus4\plus4cart.obj" \
	"$(INTDIR)\plus4\plus4datasette.obj" \
	"$(INTDIR)\plus4\plus4drive.obj" \
	"$(INTDIR)\plus4\plus4iec.obj" \
	"$(INTDIR)\plus4\plus4mem.obj" \
	"$(INTDIR)\plus4\plus4memcsory256k.obj" \
	"$(INTDIR)\plus4\plus4memhannes256k.obj" \
	"$(INTDIR)\plus4\plus4memlimit.obj" \
	"$(INTDIR)\plus4\plus4memrom.obj" \
	"$(INTDIR)\plus4\plus4memsnapshot.obj" \
	"$(INTDIR)\plus4\plus4model.obj" \
	"$(INTDIR)\plus4\plus4parallel.obj" \
	"$(INTDIR)\plus4\plus4pio1.obj" \
	"$(INTDIR)\plus4\plus4pio2.obj" \
	"$(INTDIR)\plus4\plus4printer.obj" \
	"$(INTDIR)\plus4\plus4rom.obj" \
	"$(INTDIR)\plus4\plus4romset.obj" \
	"$(INTDIR)\plus4\plus4speech.obj" \
	"$(INTDIR)\plus4\plus4tcbm.obj" \
	"$(INTDIR)\plus4\plus4video.obj" \
	"$(INTDIR)\plus4\ted-badline.obj" \
	"$(INTDIR)\plus4\ted-cmdline-options.obj" \
	"$(INTDIR)\plus4\ted-color.obj" \
	"$(INTDIR)\plus4\ted-draw.obj" \
	"$(INTDIR)\plus4\ted-fetch.obj" \
	"$(INTDIR)\plus4\ted-irq.obj" \
	"$(INTDIR)\plus4\ted-mem.obj" \
	"$(INTDIR)\plus4\ted-resources.obj" \
	"$(INTDIR)\plus4\ted-snapshot.obj" \
	"$(INTDIR)\plus4\ted-sound.obj" \
	"$(INTDIR)\plus4\ted-timer.obj" \
	"$(INTDIR)\plus4\ted-timing.obj" \
	"$(INTDIR)\plus4\ted.obj" \
	"$(INTDIR)\plus4\plus4cpu.obj" \
	".\libsbase\Debug\base.lib" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "plus4 - Win32 Release" || "$(CFG)" == "plus4 - Win32 Debug"

!IF  "$(CFG)" == "plus4 - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "plus4 - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\plus4\digiblaster.c

"$(INTDIR)\plus4\digiblaster.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4-cmdline-options.c

"$(INTDIR)\plus4\plus4-cmdline-options.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4-resources.c

"$(INTDIR)\plus4\plus4-resources.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4-sidcart.c

"$(INTDIR)\plus4\plus4-sidcart.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4-snapshot.c

"$(INTDIR)\plus4\plus4-snapshot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4.c

"$(INTDIR)\plus4\plus4.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4acia.c

"$(INTDIR)\plus4\plus4acia.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4bus.c

"$(INTDIR)\plus4\plus4bus.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4cart.c

"$(INTDIR)\plus4\plus4cart.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4datasette.c

"$(INTDIR)\plus4\plus4datasette.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4drive.c

"$(INTDIR)\plus4\plus4drive.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4iec.c

"$(INTDIR)\plus4\plus4iec.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4mem.c

"$(INTDIR)\plus4\plus4mem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4memcsory256k.c

"$(INTDIR)\plus4\plus4memcsory256k.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4memhannes256k.c

"$(INTDIR)\plus4\plus4memhannes256k.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4memlimit.c

"$(INTDIR)\plus4\plus4memlimit.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4memrom.c

"$(INTDIR)\plus4\plus4memrom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4memsnapshot.c

"$(INTDIR)\plus4\plus4memsnapshot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4model.c

"$(INTDIR)\plus4\plus4model.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4parallel.c

"$(INTDIR)\plus4\plus4parallel.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4pio1.c

"$(INTDIR)\plus4\plus4pio1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4pio2.c

"$(INTDIR)\plus4\plus4pio2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4printer.c

"$(INTDIR)\plus4\plus4printer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4rom.c

"$(INTDIR)\plus4\plus4rom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4romset.c

"$(INTDIR)\plus4\plus4romset.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4speech.c

"$(INTDIR)\plus4\plus4speech.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4tcbm.c

"$(INTDIR)\plus4\plus4tcbm.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4video.c

"$(INTDIR)\plus4\plus4video.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\ted-badline.c

"$(INTDIR)\plus4\ted-badline.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\ted-cmdline-options.c

"$(INTDIR)\plus4\ted-cmdline-options.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\ted-color.c

"$(INTDIR)\plus4\ted-color.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\ted-draw.c

"$(INTDIR)\plus4\ted-draw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\ted-fetch.c

"$(INTDIR)\plus4\ted-fetch.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\ted-irq.c

"$(INTDIR)\plus4\ted-irq.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\ted-mem.c

"$(INTDIR)\plus4\ted-mem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\ted-resources.c

"$(INTDIR)\plus4\ted-resources.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\ted-snapshot.c

"$(INTDIR)\plus4\ted-snapshot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\ted-sound.c

"$(INTDIR)\plus4\ted-sound.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\ted-timer.c

"$(INTDIR)\plus4\ted-timer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\ted-timing.c

"$(INTDIR)\plus4\ted-timing.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\ted.c

"$(INTDIR)\plus4\ted.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\plus4\plus4cpu.c

!IF  "$(CFG)" == "plus4 - Win32 Release"


CPP_SWITCHES=/nologo /MD /W3 /GX /Ot /Oa /Ow /Oi /Op /Oy /Ob2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\core" /I "..\..\..\drive" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\raster" /I "..\..\..\sid" /I "..\..\..\tape" /I "..\..\..\video" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\plus4.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

"$(INTDIR)\plus4\plus4cpu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<

!ELSEIF  "$(CFG)" == "plus4 - Win32 Debug"


CPP_SWITCHES=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\core" /I "..\..\..\drive" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\raster" /I "..\..\..\sid" /I "..\..\..\tape" /I "..\..\..\video" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\plus4.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

"$(INTDIR)\plus4\plus4cpu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<

!ENDIF 



!ENDIF 


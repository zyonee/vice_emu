# Microsoft Developer Studio Generated NMAKE File, Based on c128.dsp
!IF "$(CFG)" == ""
CFG=c128 - Win32 Release
!MESSAGE No configuration specified. Defaulting to c128 - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "c128 - Win32 Release" && "$(CFG)" != "c128 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "c128.mak" CFG="c128 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "c128 - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "c128 - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "c128 - Win32 Release"

OUTDIR=.\libs\c128\Release
INTDIR=.\libs\c128\Release
# Begin Custom Macros
OutDir=.\libs\c128\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\c128.lib" 

!ELSE 

ALL : "base - Win32 Release" "$(OUTDIR)\c128.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\c128\c128-cmdline-options.obj"
	-@erase "$(INTDIR)\c128\c128-resources.obj"
	-@erase "$(INTDIR)\c128\c128-snapshot.obj"
	-@erase "$(INTDIR)\c128\c128.obj"
	-@erase "$(INTDIR)\c128\c128cia1.obj"
	-@erase "$(INTDIR)\c128\c128drive.obj"
	-@erase "$(INTDIR)\c128\c128fastiec.obj"
	-@erase "$(INTDIR)\c128\c128mem.obj"
	-@erase "$(INTDIR)\c128\c128meminit.obj"
	-@erase "$(INTDIR)\c128\c128memlimit.obj"
	-@erase "$(INTDIR)\c128\c128memrom.obj"
	-@erase "$(INTDIR)\c128\c128memsnapshot.obj"
	-@erase "$(INTDIR)\c128\c128mmu.obj"
	-@erase "$(INTDIR)\c128\c128model.obj"
	-@erase "$(INTDIR)\c128\c128rom.obj"
	-@erase "$(INTDIR)\c128\c128romset.obj"
	-@erase "$(INTDIR)\c128\c128video.obj"
	-@erase "$(INTDIR)\c128\daa.obj"
	-@erase "$(INTDIR)\c128\functionrom.obj"
	-@erase "$(INTDIR)\c128\z80.obj"
	-@erase "$(INTDIR)\c128\z80mem.obj"
	-@erase "$(INTDIR)\c128\c128cpu.obj"
	-@erase "$(OUTDIR)\c128.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\c64 "/I "..\..\..\c64\cart "/I "..\..\..\drive "/I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\raster "/I "..\..\..\rtc "/I "..\..\..\sid "/I "..\..\..\tape "/I "..\..\..\userport "/I "..\..\..\vdc "/I "..\..\..\vicii "/I "..\..\..\video "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\c128.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\c128.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\c128.lib" 
LIB32_OBJS= \
	"$(INTDIR)\c128\c128-cmdline-options.obj" \
	"$(INTDIR)\c128\c128-resources.obj" \
	"$(INTDIR)\c128\c128-snapshot.obj" \
	"$(INTDIR)\c128\c128.obj" \
	"$(INTDIR)\c128\c128cia1.obj" \
	"$(INTDIR)\c128\c128drive.obj" \
	"$(INTDIR)\c128\c128fastiec.obj" \
	"$(INTDIR)\c128\c128mem.obj" \
	"$(INTDIR)\c128\c128meminit.obj" \
	"$(INTDIR)\c128\c128memlimit.obj" \
	"$(INTDIR)\c128\c128memrom.obj" \
	"$(INTDIR)\c128\c128memsnapshot.obj" \
	"$(INTDIR)\c128\c128mmu.obj" \
	"$(INTDIR)\c128\c128model.obj" \
	"$(INTDIR)\c128\c128rom.obj" \
	"$(INTDIR)\c128\c128romset.obj" \
	"$(INTDIR)\c128\c128video.obj" \
	"$(INTDIR)\c128\daa.obj" \
	"$(INTDIR)\c128\functionrom.obj" \
	"$(INTDIR)\c128\z80.obj" \
	"$(INTDIR)\c128\z80mem.obj" \
	"$(INTDIR)\c128\c128cpu.obj" \
	".\libsbase\Release\base.lib" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\c128\Debug
INTDIR=.\libs\c128\Debug
# Begin Custom Macros
OutDir=.\libs\c128\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\c128.lib" 

!ELSE 

ALL : "base - Win32 Debug" "$(OUTDIR)\c128.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\c128\c128-cmdline-options.obj"
	-@erase "$(INTDIR)\c128\c128-resources.obj"
	-@erase "$(INTDIR)\c128\c128-snapshot.obj"
	-@erase "$(INTDIR)\c128\c128.obj"
	-@erase "$(INTDIR)\c128\c128cia1.obj"
	-@erase "$(INTDIR)\c128\c128drive.obj"
	-@erase "$(INTDIR)\c128\c128fastiec.obj"
	-@erase "$(INTDIR)\c128\c128mem.obj"
	-@erase "$(INTDIR)\c128\c128meminit.obj"
	-@erase "$(INTDIR)\c128\c128memlimit.obj"
	-@erase "$(INTDIR)\c128\c128memrom.obj"
	-@erase "$(INTDIR)\c128\c128memsnapshot.obj"
	-@erase "$(INTDIR)\c128\c128mmu.obj"
	-@erase "$(INTDIR)\c128\c128model.obj"
	-@erase "$(INTDIR)\c128\c128rom.obj"
	-@erase "$(INTDIR)\c128\c128romset.obj"
	-@erase "$(INTDIR)\c128\c128video.obj"
	-@erase "$(INTDIR)\c128\daa.obj"
	-@erase "$(INTDIR)\c128\functionrom.obj"
	-@erase "$(INTDIR)\c128\z80.obj"
	-@erase "$(INTDIR)\c128\z80mem.obj"
	-@erase "$(INTDIR)\c128\c128cpu.obj"
	-@erase "$(OUTDIR)\c128.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\c64 "/I "..\..\..\c64\cart "/I "..\..\..\drive "/I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\raster "/I "..\..\..\rtc "/I "..\..\..\sid "/I "..\..\..\tape "/I "..\..\..\userport "/I "..\..\..\vdc "/I "..\..\..\vicii "/I "..\..\..\video "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\c128.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\c128.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\c128.lib" 
LIB32_OBJS= \
	"$(INTDIR)\c128\c128-cmdline-options.obj" \
	"$(INTDIR)\c128\c128-resources.obj" \
	"$(INTDIR)\c128\c128-snapshot.obj" \
	"$(INTDIR)\c128\c128.obj" \
	"$(INTDIR)\c128\c128cia1.obj" \
	"$(INTDIR)\c128\c128drive.obj" \
	"$(INTDIR)\c128\c128fastiec.obj" \
	"$(INTDIR)\c128\c128mem.obj" \
	"$(INTDIR)\c128\c128meminit.obj" \
	"$(INTDIR)\c128\c128memlimit.obj" \
	"$(INTDIR)\c128\c128memrom.obj" \
	"$(INTDIR)\c128\c128memsnapshot.obj" \
	"$(INTDIR)\c128\c128mmu.obj" \
	"$(INTDIR)\c128\c128model.obj" \
	"$(INTDIR)\c128\c128rom.obj" \
	"$(INTDIR)\c128\c128romset.obj" \
	"$(INTDIR)\c128\c128video.obj" \
	"$(INTDIR)\c128\daa.obj" \
	"$(INTDIR)\c128\functionrom.obj" \
	"$(INTDIR)\c128\z80.obj" \
	"$(INTDIR)\c128\z80mem.obj" \
	"$(INTDIR)\c128\c128cpu.obj" \
	".\libsbase\Debug\base.lib" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "c128 - Win32 Release" || "$(CFG)" == "c128 - Win32 Debug"

!IF  "$(CFG)" == "c128 - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "c128 - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\c128\c128-cmdline-options.c

"$(INTDIR)\c128\c128-cmdline-options.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128-resources.c

"$(INTDIR)\c128\c128-resources.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128-snapshot.c

"$(INTDIR)\c128\c128-snapshot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128.c

"$(INTDIR)\c128\c128.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128cia1.c

"$(INTDIR)\c128\c128cia1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128drive.c

"$(INTDIR)\c128\c128drive.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128fastiec.c

"$(INTDIR)\c128\c128fastiec.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128mem.c

"$(INTDIR)\c128\c128mem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128meminit.c

"$(INTDIR)\c128\c128meminit.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128memlimit.c

"$(INTDIR)\c128\c128memlimit.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128memrom.c

"$(INTDIR)\c128\c128memrom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128memsnapshot.c

"$(INTDIR)\c128\c128memsnapshot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128mmu.c

"$(INTDIR)\c128\c128mmu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128model.c

"$(INTDIR)\c128\c128model.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128rom.c

"$(INTDIR)\c128\c128rom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128romset.c

"$(INTDIR)\c128\c128romset.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128video.c

"$(INTDIR)\c128\c128video.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\daa.c

"$(INTDIR)\c128\daa.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\functionrom.c

"$(INTDIR)\c128\functionrom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\z80.c

"$(INTDIR)\c128\z80.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\z80mem.c

"$(INTDIR)\c128\z80mem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c128\c128cpu.c

!IF  "$(CFG)" == "c128 - Win32 Release"


CPP_SWITCHES=/nologo /MD /W3 /GX /Ot /Oa /Ow /Oi /Op /Oy /Ob2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\c64" /I "..\..\..\c64\cart" /I "..\..\..\drive" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\raster" /I "..\..\..\rtc" /I "..\..\..\sid" /I "..\..\..\tape" /I "..\..\..\userport" /I "..\..\..\vdc" /I "..\..\..\vicii" /I "..\..\..\video" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\c128.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

"$(INTDIR)\c128\c128cpu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<

!ELSEIF  "$(CFG)" == "c128 - Win32 Debug"


CPP_SWITCHES=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\c64" /I "..\..\..\c64\cart" /I "..\..\..\drive" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\raster" /I "..\..\..\rtc" /I "..\..\..\sid" /I "..\..\..\tape" /I "..\..\..\userport" /I "..\..\..\vdc" /I "..\..\..\vicii" /I "..\..\..\video" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\c128.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

"$(INTDIR)\c128\c128cpu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<

!ENDIF 



!ENDIF 


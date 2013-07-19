# Microsoft Developer Studio Generated NMAKE File, Based on viciisc.dsp
!IF "$(CFG)" == ""
CFG=viciisc - Win32 Release
!MESSAGE No configuration specified. Defaulting to viciisc - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "viciisc - Win32 Release" && "$(CFG)" != "viciisc - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "viciisc.mak" CFG="viciisc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "viciisc - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "viciisc - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "viciisc - Win32 Release"

OUTDIR=.\libs\viciisc\Release
INTDIR=.\libs\viciisc\Release
# Begin Custom Macros
OutDir=.\libs\viciisc\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\viciisc.lib" 

!ELSE 

ALL : "base - Win32 Release" "$(OUTDIR)\viciisc.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\viciisc\vicii-chip-model.obj"
	-@erase "$(INTDIR)\viciisc\vicii-cmdline-options.obj"
	-@erase "$(INTDIR)\viciisc\vicii-color.obj"
	-@erase "$(INTDIR)\viciisc\vicii-cycle.obj"
	-@erase "$(INTDIR)\viciisc\vicii-draw-cycle.obj"
	-@erase "$(INTDIR)\viciisc\vicii-draw.obj"
	-@erase "$(INTDIR)\viciisc\vicii-fetch.obj"
	-@erase "$(INTDIR)\viciisc\vicii-irq.obj"
	-@erase "$(INTDIR)\viciisc\vicii-lightpen.obj"
	-@erase "$(INTDIR)\viciisc\vicii-mem.obj"
	-@erase "$(INTDIR)\viciisc\vicii-phi1.obj"
	-@erase "$(INTDIR)\viciisc\vicii-resources.obj"
	-@erase "$(INTDIR)\viciisc\vicii-snapshot.obj"
	-@erase "$(INTDIR)\viciisc\vicii-timing.obj"
	-@erase "$(INTDIR)\viciisc\vicii.obj"
	-@erase "$(OUTDIR)\viciisc.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\c64 "/I "..\..\..\c64\cart "/I "..\..\..\c64dtv "/I "..\..\..\monitor "/I "..\..\..\raster "/I "..\..\..\video "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\viciisc.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\viciisc.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\viciisc.lib" 
LIB32_OBJS= \
	"$(INTDIR)\viciisc\vicii-chip-model.obj" \
	"$(INTDIR)\viciisc\vicii-cmdline-options.obj" \
	"$(INTDIR)\viciisc\vicii-color.obj" \
	"$(INTDIR)\viciisc\vicii-cycle.obj" \
	"$(INTDIR)\viciisc\vicii-draw-cycle.obj" \
	"$(INTDIR)\viciisc\vicii-draw.obj" \
	"$(INTDIR)\viciisc\vicii-fetch.obj" \
	"$(INTDIR)\viciisc\vicii-irq.obj" \
	"$(INTDIR)\viciisc\vicii-lightpen.obj" \
	"$(INTDIR)\viciisc\vicii-mem.obj" \
	"$(INTDIR)\viciisc\vicii-phi1.obj" \
	"$(INTDIR)\viciisc\vicii-resources.obj" \
	"$(INTDIR)\viciisc\vicii-snapshot.obj" \
	"$(INTDIR)\viciisc\vicii-timing.obj" \
	"$(INTDIR)\viciisc\vicii.obj" \
	".\libsbase\Release\base.lib" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\viciisc\Debug
INTDIR=.\libs\viciisc\Debug
# Begin Custom Macros
OutDir=.\libs\viciisc\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\viciisc.lib" 

!ELSE 

ALL : "base - Win32 Debug" "$(OUTDIR)\viciisc.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\viciisc\vicii-chip-model.obj"
	-@erase "$(INTDIR)\viciisc\vicii-cmdline-options.obj"
	-@erase "$(INTDIR)\viciisc\vicii-color.obj"
	-@erase "$(INTDIR)\viciisc\vicii-cycle.obj"
	-@erase "$(INTDIR)\viciisc\vicii-draw-cycle.obj"
	-@erase "$(INTDIR)\viciisc\vicii-draw.obj"
	-@erase "$(INTDIR)\viciisc\vicii-fetch.obj"
	-@erase "$(INTDIR)\viciisc\vicii-irq.obj"
	-@erase "$(INTDIR)\viciisc\vicii-lightpen.obj"
	-@erase "$(INTDIR)\viciisc\vicii-mem.obj"
	-@erase "$(INTDIR)\viciisc\vicii-phi1.obj"
	-@erase "$(INTDIR)\viciisc\vicii-resources.obj"
	-@erase "$(INTDIR)\viciisc\vicii-snapshot.obj"
	-@erase "$(INTDIR)\viciisc\vicii-timing.obj"
	-@erase "$(INTDIR)\viciisc\vicii.obj"
	-@erase "$(OUTDIR)\viciisc.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\c64 "/I "..\..\..\c64\cart "/I "..\..\..\c64dtv "/I "..\..\..\monitor "/I "..\..\..\raster "/I "..\..\..\video "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\viciisc.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\viciisc.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\viciisc.lib" 
LIB32_OBJS= \
	"$(INTDIR)\viciisc\vicii-chip-model.obj" \
	"$(INTDIR)\viciisc\vicii-cmdline-options.obj" \
	"$(INTDIR)\viciisc\vicii-color.obj" \
	"$(INTDIR)\viciisc\vicii-cycle.obj" \
	"$(INTDIR)\viciisc\vicii-draw-cycle.obj" \
	"$(INTDIR)\viciisc\vicii-draw.obj" \
	"$(INTDIR)\viciisc\vicii-fetch.obj" \
	"$(INTDIR)\viciisc\vicii-irq.obj" \
	"$(INTDIR)\viciisc\vicii-lightpen.obj" \
	"$(INTDIR)\viciisc\vicii-mem.obj" \
	"$(INTDIR)\viciisc\vicii-phi1.obj" \
	"$(INTDIR)\viciisc\vicii-resources.obj" \
	"$(INTDIR)\viciisc\vicii-snapshot.obj" \
	"$(INTDIR)\viciisc\vicii-timing.obj" \
	"$(INTDIR)\viciisc\vicii.obj" \
	".\libsbase\Debug\base.lib" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "viciisc - Win32 Release" || "$(CFG)" == "viciisc - Win32 Debug"

!IF  "$(CFG)" == "viciisc - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "viciisc - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\viciisc\vicii-chip-model.c

"$(INTDIR)\viciisc\vicii-chip-model.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\viciisc\vicii-cmdline-options.c

"$(INTDIR)\viciisc\vicii-cmdline-options.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\viciisc\vicii-color.c

"$(INTDIR)\viciisc\vicii-color.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\viciisc\vicii-cycle.c

"$(INTDIR)\viciisc\vicii-cycle.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\viciisc\vicii-draw-cycle.c

"$(INTDIR)\viciisc\vicii-draw-cycle.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\viciisc\vicii-draw.c

"$(INTDIR)\viciisc\vicii-draw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\viciisc\vicii-fetch.c

"$(INTDIR)\viciisc\vicii-fetch.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\viciisc\vicii-irq.c

"$(INTDIR)\viciisc\vicii-irq.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\viciisc\vicii-lightpen.c

"$(INTDIR)\viciisc\vicii-lightpen.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\viciisc\vicii-mem.c

"$(INTDIR)\viciisc\vicii-mem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\viciisc\vicii-phi1.c

"$(INTDIR)\viciisc\vicii-phi1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\viciisc\vicii-resources.c

"$(INTDIR)\viciisc\vicii-resources.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\viciisc\vicii-snapshot.c

"$(INTDIR)\viciisc\vicii-snapshot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\viciisc\vicii-timing.c

"$(INTDIR)\viciisc\vicii-timing.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\viciisc\vicii.c

"$(INTDIR)\viciisc\vicii.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


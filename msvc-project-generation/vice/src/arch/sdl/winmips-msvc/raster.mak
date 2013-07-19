# Microsoft Developer Studio Generated NMAKE File, Based on raster.dsp
!IF "$(CFG)" == ""
CFG=raster - Win32 Release
!MESSAGE No configuration specified. Defaulting to raster - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "raster - Win32 Release" && "$(CFG)" != "raster - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "raster.mak" CFG="raster - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "raster - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "raster - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "raster - Win32 Release"

OUTDIR=.\libs\raster\Release
INTDIR=.\libs\raster\Release
# Begin Custom Macros
OutDir=.\libs\raster\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\raster.lib" 

!ELSE 

ALL : "base - Win32 Release" "$(OUTDIR)\raster.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\raster\raster-cache.obj"
	-@erase "$(INTDIR)\raster\raster-canvas.obj"
	-@erase "$(INTDIR)\raster\raster-changes.obj"
	-@erase "$(INTDIR)\raster\raster-cmdline-options.obj"
	-@erase "$(INTDIR)\raster\raster-line-changes-sprite.obj"
	-@erase "$(INTDIR)\raster\raster-line-changes.obj"
	-@erase "$(INTDIR)\raster\raster-line.obj"
	-@erase "$(INTDIR)\raster\raster-modes.obj"
	-@erase "$(INTDIR)\raster\raster-resources.obj"
	-@erase "$(INTDIR)\raster\raster-sprite-cache.obj"
	-@erase "$(INTDIR)\raster\raster-sprite-status.obj"
	-@erase "$(INTDIR)\raster\raster-sprite.obj"
	-@erase "$(INTDIR)\raster\raster.obj"
	-@erase "$(OUTDIR)\raster.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\raster.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\raster.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\raster.lib" 
LIB32_OBJS= \
	"$(INTDIR)\raster\raster-cache.obj" \
	"$(INTDIR)\raster\raster-canvas.obj" \
	"$(INTDIR)\raster\raster-changes.obj" \
	"$(INTDIR)\raster\raster-cmdline-options.obj" \
	"$(INTDIR)\raster\raster-line-changes-sprite.obj" \
	"$(INTDIR)\raster\raster-line-changes.obj" \
	"$(INTDIR)\raster\raster-line.obj" \
	"$(INTDIR)\raster\raster-modes.obj" \
	"$(INTDIR)\raster\raster-resources.obj" \
	"$(INTDIR)\raster\raster-sprite-cache.obj" \
	"$(INTDIR)\raster\raster-sprite-status.obj" \
	"$(INTDIR)\raster\raster-sprite.obj" \
	"$(INTDIR)\raster\raster.obj" \
	".\libsbase\Release\base.lib" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\raster\Debug
INTDIR=.\libs\raster\Debug
# Begin Custom Macros
OutDir=.\libs\raster\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\raster.lib" 

!ELSE 

ALL : "base - Win32 Debug" "$(OUTDIR)\raster.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\raster\raster-cache.obj"
	-@erase "$(INTDIR)\raster\raster-canvas.obj"
	-@erase "$(INTDIR)\raster\raster-changes.obj"
	-@erase "$(INTDIR)\raster\raster-cmdline-options.obj"
	-@erase "$(INTDIR)\raster\raster-line-changes-sprite.obj"
	-@erase "$(INTDIR)\raster\raster-line-changes.obj"
	-@erase "$(INTDIR)\raster\raster-line.obj"
	-@erase "$(INTDIR)\raster\raster-modes.obj"
	-@erase "$(INTDIR)\raster\raster-resources.obj"
	-@erase "$(INTDIR)\raster\raster-sprite-cache.obj"
	-@erase "$(INTDIR)\raster\raster-sprite-status.obj"
	-@erase "$(INTDIR)\raster\raster-sprite.obj"
	-@erase "$(INTDIR)\raster\raster.obj"
	-@erase "$(OUTDIR)\raster.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\raster.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\raster.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\raster.lib" 
LIB32_OBJS= \
	"$(INTDIR)\raster\raster-cache.obj" \
	"$(INTDIR)\raster\raster-canvas.obj" \
	"$(INTDIR)\raster\raster-changes.obj" \
	"$(INTDIR)\raster\raster-cmdline-options.obj" \
	"$(INTDIR)\raster\raster-line-changes-sprite.obj" \
	"$(INTDIR)\raster\raster-line-changes.obj" \
	"$(INTDIR)\raster\raster-line.obj" \
	"$(INTDIR)\raster\raster-modes.obj" \
	"$(INTDIR)\raster\raster-resources.obj" \
	"$(INTDIR)\raster\raster-sprite-cache.obj" \
	"$(INTDIR)\raster\raster-sprite-status.obj" \
	"$(INTDIR)\raster\raster-sprite.obj" \
	"$(INTDIR)\raster\raster.obj" \
	".\libsbase\Debug\base.lib" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "raster - Win32 Release" || "$(CFG)" == "raster - Win32 Debug"

!IF  "$(CFG)" == "raster - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "raster - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\raster\raster-cache.c

"$(INTDIR)\raster\raster-cache.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\raster\raster-canvas.c

"$(INTDIR)\raster\raster-canvas.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\raster\raster-changes.c

"$(INTDIR)\raster\raster-changes.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\raster\raster-cmdline-options.c

"$(INTDIR)\raster\raster-cmdline-options.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\raster\raster-line-changes-sprite.c

"$(INTDIR)\raster\raster-line-changes-sprite.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\raster\raster-line-changes.c

"$(INTDIR)\raster\raster-line-changes.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\raster\raster-line.c

"$(INTDIR)\raster\raster-line.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\raster\raster-modes.c

"$(INTDIR)\raster\raster-modes.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\raster\raster-resources.c

"$(INTDIR)\raster\raster-resources.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\raster\raster-sprite-cache.c

"$(INTDIR)\raster\raster-sprite-cache.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\raster\raster-sprite-status.c

"$(INTDIR)\raster\raster-sprite-status.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\raster\raster-sprite.c

"$(INTDIR)\raster\raster-sprite.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\raster\raster.c

"$(INTDIR)\raster\raster.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


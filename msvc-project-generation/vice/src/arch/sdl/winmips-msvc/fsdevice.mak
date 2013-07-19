# Microsoft Developer Studio Generated NMAKE File, Based on fsdevice.dsp
!IF "$(CFG)" == ""
CFG=fsdevice - Win32 Release
!MESSAGE No configuration specified. Defaulting to fsdevice - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "fsdevice - Win32 Release" && "$(CFG)" != "fsdevice - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "fsdevice.mak" CFG="fsdevice - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "fsdevice - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "fsdevice - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "fsdevice - Win32 Release"

OUTDIR=.\libs\fsdevice\Release
INTDIR=.\libs\fsdevice\Release
# Begin Custom Macros
OutDir=.\libs\fsdevice\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\fsdevice.lib" 

!ELSE 

ALL : "base - Win32 Release" "$(OUTDIR)\fsdevice.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\fsdevice\fsdevice-close.obj"
	-@erase "$(INTDIR)\fsdevice\fsdevice-cmdline-options.obj"
	-@erase "$(INTDIR)\fsdevice\fsdevice-flush.obj"
	-@erase "$(INTDIR)\fsdevice\fsdevice-open.obj"
	-@erase "$(INTDIR)\fsdevice\fsdevice-read.obj"
	-@erase "$(INTDIR)\fsdevice\fsdevice-resources.obj"
	-@erase "$(INTDIR)\fsdevice\fsdevice-write.obj"
	-@erase "$(INTDIR)\fsdevice\fsdevice.obj"
	-@erase "$(OUTDIR)\fsdevice.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\fsdevice "/I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\fsdevice.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\fsdevice.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\fsdevice.lib" 
LIB32_OBJS= \
	"$(INTDIR)\fsdevice\fsdevice-close.obj" \
	"$(INTDIR)\fsdevice\fsdevice-cmdline-options.obj" \
	"$(INTDIR)\fsdevice\fsdevice-flush.obj" \
	"$(INTDIR)\fsdevice\fsdevice-open.obj" \
	"$(INTDIR)\fsdevice\fsdevice-read.obj" \
	"$(INTDIR)\fsdevice\fsdevice-resources.obj" \
	"$(INTDIR)\fsdevice\fsdevice-write.obj" \
	"$(INTDIR)\fsdevice\fsdevice.obj" \
	".\libsbase\Release\base.lib" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\fsdevice\Debug
INTDIR=.\libs\fsdevice\Debug
# Begin Custom Macros
OutDir=.\libs\fsdevice\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\fsdevice.lib" 

!ELSE 

ALL : "base - Win32 Debug" "$(OUTDIR)\fsdevice.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\fsdevice\fsdevice-close.obj"
	-@erase "$(INTDIR)\fsdevice\fsdevice-cmdline-options.obj"
	-@erase "$(INTDIR)\fsdevice\fsdevice-flush.obj"
	-@erase "$(INTDIR)\fsdevice\fsdevice-open.obj"
	-@erase "$(INTDIR)\fsdevice\fsdevice-read.obj"
	-@erase "$(INTDIR)\fsdevice\fsdevice-resources.obj"
	-@erase "$(INTDIR)\fsdevice\fsdevice-write.obj"
	-@erase "$(INTDIR)\fsdevice\fsdevice.obj"
	-@erase "$(OUTDIR)\fsdevice.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\fsdevice "/I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\fsdevice.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\fsdevice.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\fsdevice.lib" 
LIB32_OBJS= \
	"$(INTDIR)\fsdevice\fsdevice-close.obj" \
	"$(INTDIR)\fsdevice\fsdevice-cmdline-options.obj" \
	"$(INTDIR)\fsdevice\fsdevice-flush.obj" \
	"$(INTDIR)\fsdevice\fsdevice-open.obj" \
	"$(INTDIR)\fsdevice\fsdevice-read.obj" \
	"$(INTDIR)\fsdevice\fsdevice-resources.obj" \
	"$(INTDIR)\fsdevice\fsdevice-write.obj" \
	"$(INTDIR)\fsdevice\fsdevice.obj" \
	".\libsbase\Debug\base.lib" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "fsdevice - Win32 Release" || "$(CFG)" == "fsdevice - Win32 Debug"

!IF  "$(CFG)" == "fsdevice - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "fsdevice - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\fsdevice\fsdevice-close.c

"$(INTDIR)\fsdevice\fsdevice-close.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\fsdevice\fsdevice-cmdline-options.c

"$(INTDIR)\fsdevice\fsdevice-cmdline-options.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\fsdevice\fsdevice-flush.c

"$(INTDIR)\fsdevice\fsdevice-flush.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\fsdevice\fsdevice-open.c

"$(INTDIR)\fsdevice\fsdevice-open.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\fsdevice\fsdevice-read.c

"$(INTDIR)\fsdevice\fsdevice-read.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\fsdevice\fsdevice-resources.c

"$(INTDIR)\fsdevice\fsdevice-resources.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\fsdevice\fsdevice-write.c

"$(INTDIR)\fsdevice\fsdevice-write.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\fsdevice\fsdevice.c

"$(INTDIR)\fsdevice\fsdevice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


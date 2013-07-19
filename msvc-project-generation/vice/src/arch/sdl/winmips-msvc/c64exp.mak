# Microsoft Developer Studio Generated NMAKE File, Based on c64exp.dsp
!IF "$(CFG)" == ""
CFG=c64exp - Win32 Release
!MESSAGE No configuration specified. Defaulting to c64exp - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "c64exp - Win32 Release" && "$(CFG)" != "c64exp - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "c64exp.mak" CFG="c64exp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "c64exp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "c64exp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "c64exp - Win32 Release"

OUTDIR=.\libs\c64exp\Release
INTDIR=.\libs\c64exp\Release
# Begin Custom Macros
OutDir=.\libs\c64exp\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\c64exp.lib" 

!ELSE 

ALL : "base - Win32 Release" "$(OUTDIR)\c64exp.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\drive\iec\c64exp\c64exp-cmdline-options.obj"
	-@erase "$(INTDIR)\drive\iec\c64exp\c64exp-resources.obj"
	-@erase "$(INTDIR)\drive\iec\c64exp\dolphindos3.obj"
	-@erase "$(INTDIR)\drive\iec\c64exp\iec-c64exp.obj"
	-@erase "$(INTDIR)\drive\iec\c64exp\profdos.obj"
	-@erase "$(INTDIR)\drive\iec\c64exp\supercard.obj"
	-@erase "$(OUTDIR)\c64exp.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\lib\p64 "/I "..\..\..\core "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\c64exp.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\c64exp.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\c64exp.lib" 
LIB32_OBJS= \
	"$(INTDIR)\drive\iec\c64exp\c64exp-cmdline-options.obj" \
	"$(INTDIR)\drive\iec\c64exp\c64exp-resources.obj" \
	"$(INTDIR)\drive\iec\c64exp\dolphindos3.obj" \
	"$(INTDIR)\drive\iec\c64exp\iec-c64exp.obj" \
	"$(INTDIR)\drive\iec\c64exp\profdos.obj" \
	"$(INTDIR)\drive\iec\c64exp\supercard.obj" \
	".\libsbase\Release\base.lib" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\c64exp\Debug
INTDIR=.\libs\c64exp\Debug
# Begin Custom Macros
OutDir=.\libs\c64exp\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\c64exp.lib" 

!ELSE 

ALL : "base - Win32 Debug" "$(OUTDIR)\c64exp.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\drive\iec\c64exp\c64exp-cmdline-options.obj"
	-@erase "$(INTDIR)\drive\iec\c64exp\c64exp-resources.obj"
	-@erase "$(INTDIR)\drive\iec\c64exp\dolphindos3.obj"
	-@erase "$(INTDIR)\drive\iec\c64exp\iec-c64exp.obj"
	-@erase "$(INTDIR)\drive\iec\c64exp\profdos.obj"
	-@erase "$(INTDIR)\drive\iec\c64exp\supercard.obj"
	-@erase "$(OUTDIR)\c64exp.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\lib\p64 "/I "..\..\..\core "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\c64exp.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\c64exp.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\c64exp.lib" 
LIB32_OBJS= \
	"$(INTDIR)\drive\iec\c64exp\c64exp-cmdline-options.obj" \
	"$(INTDIR)\drive\iec\c64exp\c64exp-resources.obj" \
	"$(INTDIR)\drive\iec\c64exp\dolphindos3.obj" \
	"$(INTDIR)\drive\iec\c64exp\iec-c64exp.obj" \
	"$(INTDIR)\drive\iec\c64exp\profdos.obj" \
	"$(INTDIR)\drive\iec\c64exp\supercard.obj" \
	".\libsbase\Debug\base.lib" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "c64exp - Win32 Release" || "$(CFG)" == "c64exp - Win32 Debug"

!IF  "$(CFG)" == "c64exp - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "c64exp - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\drive\iec\c64exp\c64exp-cmdline-options.c

"$(INTDIR)\drive\iec\c64exp\c64exp-cmdline-options.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\iec\c64exp\c64exp-resources.c

"$(INTDIR)\drive\iec\c64exp\c64exp-resources.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\iec\c64exp\dolphindos3.c

"$(INTDIR)\drive\iec\c64exp\dolphindos3.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\iec\c64exp\iec-c64exp.c

"$(INTDIR)\drive\iec\c64exp\iec-c64exp.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\iec\c64exp\profdos.c

"$(INTDIR)\drive\iec\c64exp\profdos.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\iec\c64exp\supercard.c

"$(INTDIR)\drive\iec\c64exp\supercard.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


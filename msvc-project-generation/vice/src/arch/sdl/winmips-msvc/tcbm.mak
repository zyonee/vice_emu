# Microsoft Developer Studio Generated NMAKE File, Based on tcbm.dsp
!IF "$(CFG)" == ""
CFG=tcbm - Win32 Release
!MESSAGE No configuration specified. Defaulting to tcbm - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "tcbm - Win32 Release" && "$(CFG)" != "tcbm - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "tcbm.mak" CFG="tcbm - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "tcbm - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "tcbm - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "tcbm - Win32 Release"

OUTDIR=.\libs\tcbm\Release
INTDIR=.\libs\tcbm\Release
# Begin Custom Macros
OutDir=.\libs\tcbm\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\tcbm.lib" 

!ELSE 

ALL : "base - Win32 Release" "$(OUTDIR)\tcbm.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\drive\tcbm\glue1551.obj"
	-@erase "$(INTDIR)\drive\tcbm\mem1551.obj"
	-@erase "$(INTDIR)\drive\tcbm\tcbm-cmdline-options.obj"
	-@erase "$(INTDIR)\drive\tcbm\tcbm-resources.obj"
	-@erase "$(INTDIR)\drive\tcbm\tcbm.obj"
	-@erase "$(INTDIR)\drive\tcbm\tcbmrom.obj"
	-@erase "$(INTDIR)\drive\tcbm\tpid.obj"
	-@erase "$(OUTDIR)\tcbm.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\lib\p64 "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\tcbm.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tcbm.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\tcbm.lib" 
LIB32_OBJS= \
	"$(INTDIR)\drive\tcbm\glue1551.obj" \
	"$(INTDIR)\drive\tcbm\mem1551.obj" \
	"$(INTDIR)\drive\tcbm\tcbm-cmdline-options.obj" \
	"$(INTDIR)\drive\tcbm\tcbm-resources.obj" \
	"$(INTDIR)\drive\tcbm\tcbm.obj" \
	"$(INTDIR)\drive\tcbm\tcbmrom.obj" \
	"$(INTDIR)\drive\tcbm\tpid.obj" \
	".\libsbase\Release\base.lib" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\tcbm\Debug
INTDIR=.\libs\tcbm\Debug
# Begin Custom Macros
OutDir=.\libs\tcbm\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\tcbm.lib" 

!ELSE 

ALL : "base - Win32 Debug" "$(OUTDIR)\tcbm.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\drive\tcbm\glue1551.obj"
	-@erase "$(INTDIR)\drive\tcbm\mem1551.obj"
	-@erase "$(INTDIR)\drive\tcbm\tcbm-cmdline-options.obj"
	-@erase "$(INTDIR)\drive\tcbm\tcbm-resources.obj"
	-@erase "$(INTDIR)\drive\tcbm\tcbm.obj"
	-@erase "$(INTDIR)\drive\tcbm\tcbmrom.obj"
	-@erase "$(INTDIR)\drive\tcbm\tpid.obj"
	-@erase "$(OUTDIR)\tcbm.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\lib\p64 "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\tcbm.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\tcbm.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\tcbm.lib" 
LIB32_OBJS= \
	"$(INTDIR)\drive\tcbm\glue1551.obj" \
	"$(INTDIR)\drive\tcbm\mem1551.obj" \
	"$(INTDIR)\drive\tcbm\tcbm-cmdline-options.obj" \
	"$(INTDIR)\drive\tcbm\tcbm-resources.obj" \
	"$(INTDIR)\drive\tcbm\tcbm.obj" \
	"$(INTDIR)\drive\tcbm\tcbmrom.obj" \
	"$(INTDIR)\drive\tcbm\tpid.obj" \
	".\libsbase\Debug\base.lib" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "tcbm - Win32 Release" || "$(CFG)" == "tcbm - Win32 Debug"

!IF  "$(CFG)" == "tcbm - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "tcbm - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\drive\tcbm\glue1551.c

"$(INTDIR)\drive\tcbm\glue1551.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\tcbm\mem1551.c

"$(INTDIR)\drive\tcbm\mem1551.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\tcbm\tcbm-cmdline-options.c

"$(INTDIR)\drive\tcbm\tcbm-cmdline-options.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\tcbm\tcbm-resources.c

"$(INTDIR)\drive\tcbm\tcbm-resources.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\tcbm\tcbm.c

"$(INTDIR)\drive\tcbm\tcbm.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\tcbm\tcbmrom.c

"$(INTDIR)\drive\tcbm\tcbmrom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\tcbm\tpid.c

"$(INTDIR)\drive\tcbm\tpid.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


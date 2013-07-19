# Microsoft Developer Studio Generated NMAKE File, Based on geninfocontrib.dsp
!IF "$(CFG)" == ""
CFG=geninfocontrib - Win32 Release
!MESSAGE No configuration specified. Defaulting to geninfocontrib - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "geninfocontrib - Win32 Release" && "$(CFG)" != "geninfocontrib - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "geninfocontrib.mak" CFG="geninfocontrib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "geninfocontrib - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "geninfocontrib - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "geninfocontrib - Win32 Release"

OUTDIR=.\..\..\..\..\data
INTDIR=.\libs\geninfocontrib\Release
# Begin Custom Macros
OutDir=.\..\..\..\..\data
# End Custom Macros

ALL : "$(OUTDIR)\geninfocontrib.exe" "..\..\..\infocontrib.h" 

CLEAN :
	-@erase "$(INTDIR)\arch\win32\utils\geninfocontrib.obj"
	-@erase "$(OUTDIR)\geninfocontrib.exe"
	-@erase "..\..\..\infocontrib.h"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_CONSOLE" /D "_MBCS" /D "NDEBUG" /Fp"$(INTDIR)\geninfocontrib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\geninfocontrib.bsc" 
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib winmm.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib version.lib SDLmain.lib SDL.lib opengl32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\geninfocontrib.pdb" /machine:MIPS /out:"$(OUTDIR)\geninfocontrib.exe" 
LINK32_OBJS= \
	"$(INTDIR)\arch\win32\utils\geninfocontrib.obj" \


"$(OUTDIR)\geninfocontrib.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

InputPath=..\..\..\..\data\geninfocontrib.exe
SOURCE="$(InputPath)"

"..\..\..\infocontrib.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	..\..\..\..\data\geninfocontrib ..\..\..\..\doc\vice.texi ..\..\..\infocontrib.h ..\..\..\infocontrib.sed
<< 


!ELSEIF  "$(CFG)" == "..\..\..\..\data\geninfocontrib.exe - Win32 Debug"

OUTDIR=.\..\..\..\..\data
INTDIR=.\libs\geninfocontrib\Debug
# Begin Custom Macros
OutDir=.\..\..\..\..\data
# End Custom Macros

ALL : "$(OUTDIR)\geninfocontrib.exe" "..\..\..\infocontrib.h" 

CLEAN :
	-@erase "$(INTDIR)\arch\win32\utils\geninfocontrib.obj"
	-@erase "$(OUTDIR)\geninfocontrib.exe"
	-@erase "$(OUTDIR)\geninfocontrib.ilk"
	-@erase "$(OUTDIR)\geninfocontrib.pdb"
	-@erase "..\..\..\infocontrib.h"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_CONSOLE" /D "_MBCS" /D "_DEBUG" /Fp"$(INTDIR)\geninfocontrib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\geninfocontrib.bsc" 
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib winmm.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib version.lib SDLmain.lib SDL.lib opengl32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\geninfocontrib.pdb" /debug /machine:MIPS /nodefaultlib:"msvcrt.lib" /out:"$(OUTDIR)\geninfocontrib.exe" /pdbtype:sept
LINK32_OBJS= \
	"$(INTDIR)\arch\win32\utils\geninfocontrib.obj" \


"$(OUTDIR)\geninfocontrib.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

InputPath=..\..\..\..\data\geninfocontrib.exe
SOURCE="$(InputPath)"

"..\..\..\infocontrib.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	<<tempfile.bat 
	@echo off 
	..\..\..\..\data\geninfocontrib ..\..\..\..\doc\vice.texi ..\..\..\infocontrib.h ..\..\..\infocontrib.sed
<< 


!ENDIF 


!IF "$(CFG)" == "geninfocontrib - Win32 Release" || "$(CFG)" == "geninfocontrib - Win32 Debug"

SOURCE=..\..\..\arch\win32\utils\geninfocontrib.c

"$(INTDIR)\arch\win32\utils\geninfocontrib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


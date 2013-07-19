# Microsoft Developer Studio Generated NMAKE File, Based on platform.dsp
!IF "$(CFG)" == ""
CFG=platform - Win32 Release
!MESSAGE No configuration specified. Defaulting to platform - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "platform - Win32 Release" && "$(CFG)" != "platform - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "platform.mak" CFG="platform - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "platform - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "platform - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "platform - Win32 Release"

OUTDIR=.\libs\platform\Release
INTDIR=.\libs\platform\Release
# Begin Custom Macros
OutDir=.\libs\platform\Release
# End Custom Macros

ALL : "$(OUTDIR)\platform.lib" 

CLEAN :
	-@erase "$(INTDIR)\platform\platform.obj"
	-@erase "$(INTDIR)\platform\platform_windows_runtime_os.obj"
	-@erase "$(INTDIR)\platform\platform_x86_runtime_cpu.obj"
	-@erase "$(OUTDIR)\platform.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\imagecontents "/I "..\..\..\monitor "/I "..\..\..\rs232drv "/I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\platform.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\platform.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\platform.lib" 
LIB32_OBJS= \
	"$(INTDIR)\platform\platform.obj" \
	"$(INTDIR)\platform\platform_windows_runtime_os.obj" \
	"$(INTDIR)\platform\platform_x86_runtime_cpu.obj" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\platform\Debug
INTDIR=.\libs\platform\Debug
# Begin Custom Macros
OutDir=.\libs\platform\Debug
# End Custom Macros

ALL : "$(OUTDIR)\platform.lib" 

CLEAN :
	-@erase "$(INTDIR)\platform\platform.obj"
	-@erase "$(INTDIR)\platform\platform_windows_runtime_os.obj"
	-@erase "$(INTDIR)\platform\platform_x86_runtime_cpu.obj"
	-@erase "$(OUTDIR)\platform.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\imagecontents "/I "..\..\..\monitor "/I "..\..\..\rs232drv "/I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\platform.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\platform.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\platform.lib" 
LIB32_OBJS= \
	"$(INTDIR)\platform\platform.obj" \
	"$(INTDIR)\platform\platform_windows_runtime_os.obj" \
	"$(INTDIR)\platform\platform_x86_runtime_cpu.obj" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "platform - Win32 Release" || "$(CFG)" == "platform - Win32 Debug"

SOURCE=..\..\..\platform\platform.c

"$(INTDIR)\platform\platform.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\platform\platform_windows_runtime_os.c

"$(INTDIR)\platform\platform_windows_runtime_os.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\platform\platform_x86_runtime_cpu.c

"$(INTDIR)\platform\platform_x86_runtime_cpu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


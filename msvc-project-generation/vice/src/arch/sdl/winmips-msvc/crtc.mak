# Microsoft Developer Studio Generated NMAKE File, Based on crtc.dsp
!IF "$(CFG)" == ""
CFG=crtc - Win32 Release
!MESSAGE No configuration specified. Defaulting to crtc - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "crtc - Win32 Release" && "$(CFG)" != "crtc - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "crtc.mak" CFG="crtc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "crtc - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "crtc - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "crtc - Win32 Release"

OUTDIR=.\libs\crtc\Release
INTDIR=.\libs\crtc\Release
# Begin Custom Macros
OutDir=.\libs\crtc\Release
# End Custom Macros

ALL : "$(OUTDIR)\crtc.lib" 

CLEAN :
	-@erase "$(INTDIR)\crtc\crtc-cmdline-options.obj"
	-@erase "$(INTDIR)\crtc\crtc-color.obj"
	-@erase "$(INTDIR)\crtc\crtc-draw.obj"
	-@erase "$(INTDIR)\crtc\crtc-mem.obj"
	-@erase "$(INTDIR)\crtc\crtc-resources.obj"
	-@erase "$(INTDIR)\crtc\crtc-snapshot.obj"
	-@erase "$(INTDIR)\crtc\crtc.obj"
	-@erase "$(OUTDIR)\crtc.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\raster "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\crtc.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\crtc.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\crtc.lib" 
LIB32_OBJS= \
	"$(INTDIR)\crtc\crtc-cmdline-options.obj" \
	"$(INTDIR)\crtc\crtc-color.obj" \
	"$(INTDIR)\crtc\crtc-draw.obj" \
	"$(INTDIR)\crtc\crtc-mem.obj" \
	"$(INTDIR)\crtc\crtc-resources.obj" \
	"$(INTDIR)\crtc\crtc-snapshot.obj" \
	"$(INTDIR)\crtc\crtc.obj" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\crtc\Debug
INTDIR=.\libs\crtc\Debug
# Begin Custom Macros
OutDir=.\libs\crtc\Debug
# End Custom Macros

ALL : "$(OUTDIR)\crtc.lib" 

CLEAN :
	-@erase "$(INTDIR)\crtc\crtc-cmdline-options.obj"
	-@erase "$(INTDIR)\crtc\crtc-color.obj"
	-@erase "$(INTDIR)\crtc\crtc-draw.obj"
	-@erase "$(INTDIR)\crtc\crtc-mem.obj"
	-@erase "$(INTDIR)\crtc\crtc-resources.obj"
	-@erase "$(INTDIR)\crtc\crtc-snapshot.obj"
	-@erase "$(INTDIR)\crtc\crtc.obj"
	-@erase "$(OUTDIR)\crtc.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\raster "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\crtc.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\crtc.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\crtc.lib" 
LIB32_OBJS= \
	"$(INTDIR)\crtc\crtc-cmdline-options.obj" \
	"$(INTDIR)\crtc\crtc-color.obj" \
	"$(INTDIR)\crtc\crtc-draw.obj" \
	"$(INTDIR)\crtc\crtc-mem.obj" \
	"$(INTDIR)\crtc\crtc-resources.obj" \
	"$(INTDIR)\crtc\crtc-snapshot.obj" \
	"$(INTDIR)\crtc\crtc.obj" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "crtc - Win32 Release" || "$(CFG)" == "crtc - Win32 Debug"

SOURCE=..\..\..\crtc\crtc-cmdline-options.c

"$(INTDIR)\crtc\crtc-cmdline-options.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\crtc\crtc-color.c

"$(INTDIR)\crtc\crtc-color.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\crtc\crtc-draw.c

"$(INTDIR)\crtc\crtc-draw.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\crtc\crtc-mem.c

"$(INTDIR)\crtc\crtc-mem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\crtc\crtc-resources.c

"$(INTDIR)\crtc\crtc-resources.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\crtc\crtc-snapshot.c

"$(INTDIR)\crtc\crtc-snapshot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\crtc\crtc.c

"$(INTDIR)\crtc\crtc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


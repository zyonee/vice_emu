# Microsoft Developer Studio Generated NMAKE File, Based on diskimage.dsp
!IF "$(CFG)" == ""
CFG=diskimage - Win32 Release
!MESSAGE No configuration specified. Defaulting to diskimage - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "diskimage - Win32 Release" && "$(CFG)" != "diskimage - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "diskimage.mak" CFG="diskimage - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "diskimage - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "diskimage - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "diskimage - Win32 Release"

OUTDIR=.\libs\diskimage\Release
INTDIR=.\libs\diskimage\Release
# Begin Custom Macros
OutDir=.\libs\diskimage\Release
# End Custom Macros

ALL : "$(OUTDIR)\diskimage.lib" 

CLEAN :
	-@erase "$(INTDIR)\diskimage\diskimage.obj"
	-@erase "$(INTDIR)\diskimage\fsimage-check.obj"
	-@erase "$(INTDIR)\diskimage\fsimage-create.obj"
	-@erase "$(INTDIR)\diskimage\fsimage-dxx.obj"
	-@erase "$(INTDIR)\diskimage\fsimage-gcr.obj"
	-@erase "$(INTDIR)\diskimage\fsimage-p64.obj"
	-@erase "$(INTDIR)\diskimage\fsimage-probe.obj"
	-@erase "$(INTDIR)\diskimage\fsimage.obj"
	-@erase "$(INTDIR)\diskimage\rawimage.obj"
	-@erase "$(INTDIR)\diskimage\realimage.obj"
	-@erase "$(OUTDIR)\diskimage.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\diskimage "/I "..\..\..\lib\p64 "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\diskimage.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\diskimage.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\diskimage.lib" 
LIB32_OBJS= \
	"$(INTDIR)\diskimage\diskimage.obj" \
	"$(INTDIR)\diskimage\fsimage-check.obj" \
	"$(INTDIR)\diskimage\fsimage-create.obj" \
	"$(INTDIR)\diskimage\fsimage-dxx.obj" \
	"$(INTDIR)\diskimage\fsimage-gcr.obj" \
	"$(INTDIR)\diskimage\fsimage-p64.obj" \
	"$(INTDIR)\diskimage\fsimage-probe.obj" \
	"$(INTDIR)\diskimage\fsimage.obj" \
	"$(INTDIR)\diskimage\rawimage.obj" \
	"$(INTDIR)\diskimage\realimage.obj" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\diskimage\Debug
INTDIR=.\libs\diskimage\Debug
# Begin Custom Macros
OutDir=.\libs\diskimage\Debug
# End Custom Macros

ALL : "$(OUTDIR)\diskimage.lib" 

CLEAN :
	-@erase "$(INTDIR)\diskimage\diskimage.obj"
	-@erase "$(INTDIR)\diskimage\fsimage-check.obj"
	-@erase "$(INTDIR)\diskimage\fsimage-create.obj"
	-@erase "$(INTDIR)\diskimage\fsimage-dxx.obj"
	-@erase "$(INTDIR)\diskimage\fsimage-gcr.obj"
	-@erase "$(INTDIR)\diskimage\fsimage-p64.obj"
	-@erase "$(INTDIR)\diskimage\fsimage-probe.obj"
	-@erase "$(INTDIR)\diskimage\fsimage.obj"
	-@erase "$(INTDIR)\diskimage\rawimage.obj"
	-@erase "$(INTDIR)\diskimage\realimage.obj"
	-@erase "$(OUTDIR)\diskimage.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\diskimage "/I "..\..\..\lib\p64 "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\diskimage.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\diskimage.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\diskimage.lib" 
LIB32_OBJS= \
	"$(INTDIR)\diskimage\diskimage.obj" \
	"$(INTDIR)\diskimage\fsimage-check.obj" \
	"$(INTDIR)\diskimage\fsimage-create.obj" \
	"$(INTDIR)\diskimage\fsimage-dxx.obj" \
	"$(INTDIR)\diskimage\fsimage-gcr.obj" \
	"$(INTDIR)\diskimage\fsimage-p64.obj" \
	"$(INTDIR)\diskimage\fsimage-probe.obj" \
	"$(INTDIR)\diskimage\fsimage.obj" \
	"$(INTDIR)\diskimage\rawimage.obj" \
	"$(INTDIR)\diskimage\realimage.obj" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "diskimage - Win32 Release" || "$(CFG)" == "diskimage - Win32 Debug"

SOURCE=..\..\..\diskimage\diskimage.c

"$(INTDIR)\diskimage\diskimage.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\diskimage\fsimage-check.c

"$(INTDIR)\diskimage\fsimage-check.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\diskimage\fsimage-create.c

"$(INTDIR)\diskimage\fsimage-create.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\diskimage\fsimage-dxx.c

"$(INTDIR)\diskimage\fsimage-dxx.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\diskimage\fsimage-gcr.c

"$(INTDIR)\diskimage\fsimage-gcr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\diskimage\fsimage-p64.c

"$(INTDIR)\diskimage\fsimage-p64.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\diskimage\fsimage-probe.c

"$(INTDIR)\diskimage\fsimage-probe.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\diskimage\fsimage.c

"$(INTDIR)\diskimage\fsimage.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\diskimage\rawimage.c

"$(INTDIR)\diskimage\rawimage.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\diskimage\realimage.c

"$(INTDIR)\diskimage\realimage.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


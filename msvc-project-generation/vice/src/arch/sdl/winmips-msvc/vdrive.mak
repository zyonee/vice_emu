# Microsoft Developer Studio Generated NMAKE File, Based on vdrive.dsp
!IF "$(CFG)" == ""
CFG=vdrive - Win32 Release
!MESSAGE No configuration specified. Defaulting to vdrive - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "vdrive - Win32 Release" && "$(CFG)" != "vdrive - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "vdrive.mak" CFG="vdrive - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "vdrive - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "vdrive - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "vdrive - Win32 Release"

OUTDIR=.\libs\vdrive\Release
INTDIR=.\libs\vdrive\Release
# Begin Custom Macros
OutDir=.\libs\vdrive\Release
# End Custom Macros

ALL : "$(OUTDIR)\vdrive.lib" 

CLEAN :
	-@erase "$(INTDIR)\vdrive\vdrive-bam.obj"
	-@erase "$(INTDIR)\vdrive\vdrive-command.obj"
	-@erase "$(INTDIR)\vdrive\vdrive-dir.obj"
	-@erase "$(INTDIR)\vdrive\vdrive-iec.obj"
	-@erase "$(INTDIR)\vdrive\vdrive-internal.obj"
	-@erase "$(INTDIR)\vdrive\vdrive-rel.obj"
	-@erase "$(INTDIR)\vdrive\vdrive-snapshot.obj"
	-@erase "$(INTDIR)\vdrive\vdrive.obj"
	-@erase "$(OUTDIR)\vdrive.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\lib\p64 "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\vdrive.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\vdrive.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\vdrive.lib" 
LIB32_OBJS= \
	"$(INTDIR)\vdrive\vdrive-bam.obj" \
	"$(INTDIR)\vdrive\vdrive-command.obj" \
	"$(INTDIR)\vdrive\vdrive-dir.obj" \
	"$(INTDIR)\vdrive\vdrive-iec.obj" \
	"$(INTDIR)\vdrive\vdrive-internal.obj" \
	"$(INTDIR)\vdrive\vdrive-rel.obj" \
	"$(INTDIR)\vdrive\vdrive-snapshot.obj" \
	"$(INTDIR)\vdrive\vdrive.obj" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\vdrive\Debug
INTDIR=.\libs\vdrive\Debug
# Begin Custom Macros
OutDir=.\libs\vdrive\Debug
# End Custom Macros

ALL : "$(OUTDIR)\vdrive.lib" 

CLEAN :
	-@erase "$(INTDIR)\vdrive\vdrive-bam.obj"
	-@erase "$(INTDIR)\vdrive\vdrive-command.obj"
	-@erase "$(INTDIR)\vdrive\vdrive-dir.obj"
	-@erase "$(INTDIR)\vdrive\vdrive-iec.obj"
	-@erase "$(INTDIR)\vdrive\vdrive-internal.obj"
	-@erase "$(INTDIR)\vdrive\vdrive-rel.obj"
	-@erase "$(INTDIR)\vdrive\vdrive-snapshot.obj"
	-@erase "$(INTDIR)\vdrive\vdrive.obj"
	-@erase "$(OUTDIR)\vdrive.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\lib\p64 "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\vdrive.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\vdrive.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\vdrive.lib" 
LIB32_OBJS= \
	"$(INTDIR)\vdrive\vdrive-bam.obj" \
	"$(INTDIR)\vdrive\vdrive-command.obj" \
	"$(INTDIR)\vdrive\vdrive-dir.obj" \
	"$(INTDIR)\vdrive\vdrive-iec.obj" \
	"$(INTDIR)\vdrive\vdrive-internal.obj" \
	"$(INTDIR)\vdrive\vdrive-rel.obj" \
	"$(INTDIR)\vdrive\vdrive-snapshot.obj" \
	"$(INTDIR)\vdrive\vdrive.obj" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "vdrive - Win32 Release" || "$(CFG)" == "vdrive - Win32 Debug"

SOURCE=..\..\..\vdrive\vdrive-bam.c

"$(INTDIR)\vdrive\vdrive-bam.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\vdrive\vdrive-command.c

"$(INTDIR)\vdrive\vdrive-command.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\vdrive\vdrive-dir.c

"$(INTDIR)\vdrive\vdrive-dir.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\vdrive\vdrive-iec.c

"$(INTDIR)\vdrive\vdrive-iec.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\vdrive\vdrive-internal.c

"$(INTDIR)\vdrive\vdrive-internal.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\vdrive\vdrive-rel.c

"$(INTDIR)\vdrive\vdrive-rel.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\vdrive\vdrive-snapshot.c

"$(INTDIR)\vdrive\vdrive-snapshot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\vdrive\vdrive.c

"$(INTDIR)\vdrive\vdrive.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


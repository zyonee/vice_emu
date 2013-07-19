# Microsoft Developer Studio Generated NMAKE File, Based on imagecontents.dsp
!IF "$(CFG)" == ""
CFG=imagecontents - Win32 Release
!MESSAGE No configuration specified. Defaulting to imagecontents - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "imagecontents - Win32 Release" && "$(CFG)" != "imagecontents - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "imagecontents.mak" CFG="imagecontents - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "imagecontents - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "imagecontents - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "imagecontents - Win32 Release"

OUTDIR=.\libs\imagecontents\Release
INTDIR=.\libs\imagecontents\Release
# Begin Custom Macros
OutDir=.\libs\imagecontents\Release
# End Custom Macros

ALL : "$(OUTDIR)\imagecontents.lib" 

CLEAN :
	-@erase "$(INTDIR)\imagecontents\diskcontents-block.obj"
	-@erase "$(INTDIR)\imagecontents\diskcontents-iec.obj"
	-@erase "$(INTDIR)\imagecontents\diskcontents.obj"
	-@erase "$(INTDIR)\imagecontents\imagecontents.obj"
	-@erase "$(INTDIR)\imagecontents\tapecontents.obj"
	-@erase "$(OUTDIR)\imagecontents.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\imagecontents.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\imagecontents.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\imagecontents.lib" 
LIB32_OBJS= \
	"$(INTDIR)\imagecontents\diskcontents-block.obj" \
	"$(INTDIR)\imagecontents\diskcontents-iec.obj" \
	"$(INTDIR)\imagecontents\diskcontents.obj" \
	"$(INTDIR)\imagecontents\imagecontents.obj" \
	"$(INTDIR)\imagecontents\tapecontents.obj" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\imagecontents\Debug
INTDIR=.\libs\imagecontents\Debug
# Begin Custom Macros
OutDir=.\libs\imagecontents\Debug
# End Custom Macros

ALL : "$(OUTDIR)\imagecontents.lib" 

CLEAN :
	-@erase "$(INTDIR)\imagecontents\diskcontents-block.obj"
	-@erase "$(INTDIR)\imagecontents\diskcontents-iec.obj"
	-@erase "$(INTDIR)\imagecontents\diskcontents.obj"
	-@erase "$(INTDIR)\imagecontents\imagecontents.obj"
	-@erase "$(INTDIR)\imagecontents\tapecontents.obj"
	-@erase "$(OUTDIR)\imagecontents.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\imagecontents.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\imagecontents.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\imagecontents.lib" 
LIB32_OBJS= \
	"$(INTDIR)\imagecontents\diskcontents-block.obj" \
	"$(INTDIR)\imagecontents\diskcontents-iec.obj" \
	"$(INTDIR)\imagecontents\diskcontents.obj" \
	"$(INTDIR)\imagecontents\imagecontents.obj" \
	"$(INTDIR)\imagecontents\tapecontents.obj" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "imagecontents - Win32 Release" || "$(CFG)" == "imagecontents - Win32 Debug"

SOURCE=..\..\..\imagecontents\diskcontents-block.c

"$(INTDIR)\imagecontents\diskcontents-block.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\imagecontents\diskcontents-iec.c

"$(INTDIR)\imagecontents\diskcontents-iec.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\imagecontents\diskcontents.c

"$(INTDIR)\imagecontents\diskcontents.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\imagecontents\imagecontents.c

"$(INTDIR)\imagecontents\imagecontents.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\imagecontents\tapecontents.c

"$(INTDIR)\imagecontents\tapecontents.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


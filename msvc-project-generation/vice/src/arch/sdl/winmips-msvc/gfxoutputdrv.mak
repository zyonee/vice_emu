# Microsoft Developer Studio Generated NMAKE File, Based on gfxoutputdrv.dsp
!IF "$(CFG)" == ""
CFG=gfxoutputdrv - Win32 Release
!MESSAGE No configuration specified. Defaulting to gfxoutputdrv - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "gfxoutputdrv - Win32 Release" && "$(CFG)" != "gfxoutputdrv - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gfxoutputdrv.mak" CFG="gfxoutputdrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gfxoutputdrv - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "gfxoutputdrv - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "gfxoutputdrv - Win32 Release"

OUTDIR=.\libs\gfxoutputdrv\Release
INTDIR=.\libs\gfxoutputdrv\Release
# Begin Custom Macros
OutDir=.\libs\gfxoutputdrv\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\gfxoutputdrv.lib" 

!ELSE 

ALL : "base - Win32 Release" "$(OUTDIR)\gfxoutputdrv.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\gfxoutputdrv\bmpdrv.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\doodledrv.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\ffmpegdrv.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\ffmpeglib.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\gfxoutput.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\iffdrv.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\koaladrv.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\nativedrv.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\pcxdrv.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\ppmdrv.obj"
	-@erase "$(OUTDIR)\gfxoutputdrv.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\lib\libffmpeg "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\gfxoutputdrv.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gfxoutputdrv.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\gfxoutputdrv.lib" 
LIB32_OBJS= \
	"$(INTDIR)\gfxoutputdrv\bmpdrv.obj" \
	"$(INTDIR)\gfxoutputdrv\doodledrv.obj" \
	"$(INTDIR)\gfxoutputdrv\ffmpegdrv.obj" \
	"$(INTDIR)\gfxoutputdrv\ffmpeglib.obj" \
	"$(INTDIR)\gfxoutputdrv\gfxoutput.obj" \
	"$(INTDIR)\gfxoutputdrv\iffdrv.obj" \
	"$(INTDIR)\gfxoutputdrv\koaladrv.obj" \
	"$(INTDIR)\gfxoutputdrv\nativedrv.obj" \
	"$(INTDIR)\gfxoutputdrv\pcxdrv.obj" \
	"$(INTDIR)\gfxoutputdrv\ppmdrv.obj" \
	".\libsbase\Release\base.lib" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\gfxoutputdrv\Debug
INTDIR=.\libs\gfxoutputdrv\Debug
# Begin Custom Macros
OutDir=.\libs\gfxoutputdrv\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\gfxoutputdrv.lib" 

!ELSE 

ALL : "base - Win32 Debug" "$(OUTDIR)\gfxoutputdrv.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\gfxoutputdrv\bmpdrv.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\doodledrv.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\ffmpegdrv.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\ffmpeglib.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\gfxoutput.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\iffdrv.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\koaladrv.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\nativedrv.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\pcxdrv.obj"
	-@erase "$(INTDIR)\gfxoutputdrv\ppmdrv.obj"
	-@erase "$(OUTDIR)\gfxoutputdrv.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\lib\libffmpeg "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\gfxoutputdrv.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gfxoutputdrv.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\gfxoutputdrv.lib" 
LIB32_OBJS= \
	"$(INTDIR)\gfxoutputdrv\bmpdrv.obj" \
	"$(INTDIR)\gfxoutputdrv\doodledrv.obj" \
	"$(INTDIR)\gfxoutputdrv\ffmpegdrv.obj" \
	"$(INTDIR)\gfxoutputdrv\ffmpeglib.obj" \
	"$(INTDIR)\gfxoutputdrv\gfxoutput.obj" \
	"$(INTDIR)\gfxoutputdrv\iffdrv.obj" \
	"$(INTDIR)\gfxoutputdrv\koaladrv.obj" \
	"$(INTDIR)\gfxoutputdrv\nativedrv.obj" \
	"$(INTDIR)\gfxoutputdrv\pcxdrv.obj" \
	"$(INTDIR)\gfxoutputdrv\ppmdrv.obj" \
	".\libsbase\Debug\base.lib" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "gfxoutputdrv - Win32 Release" || "$(CFG)" == "gfxoutputdrv - Win32 Debug"

!IF  "$(CFG)" == "gfxoutputdrv - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "gfxoutputdrv - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\gfxoutputdrv\bmpdrv.c

"$(INTDIR)\gfxoutputdrv\bmpdrv.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\gfxoutputdrv\doodledrv.c

"$(INTDIR)\gfxoutputdrv\doodledrv.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\gfxoutputdrv\ffmpegdrv.c

"$(INTDIR)\gfxoutputdrv\ffmpegdrv.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\gfxoutputdrv\ffmpeglib.c

"$(INTDIR)\gfxoutputdrv\ffmpeglib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\gfxoutputdrv\gfxoutput.c

"$(INTDIR)\gfxoutputdrv\gfxoutput.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\gfxoutputdrv\iffdrv.c

"$(INTDIR)\gfxoutputdrv\iffdrv.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\gfxoutputdrv\koaladrv.c

"$(INTDIR)\gfxoutputdrv\koaladrv.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\gfxoutputdrv\nativedrv.c

"$(INTDIR)\gfxoutputdrv\nativedrv.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\gfxoutputdrv\pcxdrv.c

"$(INTDIR)\gfxoutputdrv\pcxdrv.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\gfxoutputdrv\ppmdrv.c

"$(INTDIR)\gfxoutputdrv\ppmdrv.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


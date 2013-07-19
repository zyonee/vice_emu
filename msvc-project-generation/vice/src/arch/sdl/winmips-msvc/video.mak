# Microsoft Developer Studio Generated NMAKE File, Based on video.dsp
!IF "$(CFG)" == ""
CFG=video - Win32 Release
!MESSAGE No configuration specified. Defaulting to video - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "video - Win32 Release" && "$(CFG)" != "video - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "video.mak" CFG="video - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "video - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "video - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "video - Win32 Release"

OUTDIR=.\libs\video\Release
INTDIR=.\libs\video\Release
# Begin Custom Macros
OutDir=.\libs\video\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\video.lib" 

!ELSE 

ALL : "base - Win32 Release" "$(OUTDIR)\video.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\video\render1x1.obj"
	-@erase "$(INTDIR)\video\render1x1ntsc.obj"
	-@erase "$(INTDIR)\video\render1x1pal.obj"
	-@erase "$(INTDIR)\video\render1x2.obj"
	-@erase "$(INTDIR)\video\render1x2crt.obj"
	-@erase "$(INTDIR)\video\render2x2.obj"
	-@erase "$(INTDIR)\video\render2x2crt.obj"
	-@erase "$(INTDIR)\video\render2x2ntsc.obj"
	-@erase "$(INTDIR)\video\render2x2pal.obj"
	-@erase "$(INTDIR)\video\render2x4.obj"
	-@erase "$(INTDIR)\video\render2x4crt.obj"
	-@erase "$(INTDIR)\video\renderscale2x.obj"
	-@erase "$(INTDIR)\video\renderyuv.obj"
	-@erase "$(INTDIR)\video\video-canvas.obj"
	-@erase "$(INTDIR)\video\video-cmdline-options.obj"
	-@erase "$(INTDIR)\video\video-color.obj"
	-@erase "$(INTDIR)\video\video-render-1x2.obj"
	-@erase "$(INTDIR)\video\video-render-2x2.obj"
	-@erase "$(INTDIR)\video\video-render-crt.obj"
	-@erase "$(INTDIR)\video\video-render-pal.obj"
	-@erase "$(INTDIR)\video\video-render.obj"
	-@erase "$(INTDIR)\video\video-resources.obj"
	-@erase "$(INTDIR)\video\video-sound.obj"
	-@erase "$(INTDIR)\video\video-viewport.obj"
	-@erase "$(OUTDIR)\video.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\c64 "/I "..\..\..\raster "/I "..\..\..\video "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\video.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\video.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\video.lib" 
LIB32_OBJS= \
	"$(INTDIR)\video\render1x1.obj" \
	"$(INTDIR)\video\render1x1ntsc.obj" \
	"$(INTDIR)\video\render1x1pal.obj" \
	"$(INTDIR)\video\render1x2.obj" \
	"$(INTDIR)\video\render1x2crt.obj" \
	"$(INTDIR)\video\render2x2.obj" \
	"$(INTDIR)\video\render2x2crt.obj" \
	"$(INTDIR)\video\render2x2ntsc.obj" \
	"$(INTDIR)\video\render2x2pal.obj" \
	"$(INTDIR)\video\render2x4.obj" \
	"$(INTDIR)\video\render2x4crt.obj" \
	"$(INTDIR)\video\renderscale2x.obj" \
	"$(INTDIR)\video\renderyuv.obj" \
	"$(INTDIR)\video\video-canvas.obj" \
	"$(INTDIR)\video\video-cmdline-options.obj" \
	"$(INTDIR)\video\video-color.obj" \
	"$(INTDIR)\video\video-render-1x2.obj" \
	"$(INTDIR)\video\video-render-2x2.obj" \
	"$(INTDIR)\video\video-render-crt.obj" \
	"$(INTDIR)\video\video-render-pal.obj" \
	"$(INTDIR)\video\video-render.obj" \
	"$(INTDIR)\video\video-resources.obj" \
	"$(INTDIR)\video\video-sound.obj" \
	"$(INTDIR)\video\video-viewport.obj" \
	".\libsbase\Release\base.lib" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\video\Debug
INTDIR=.\libs\video\Debug
# Begin Custom Macros
OutDir=.\libs\video\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\video.lib" 

!ELSE 

ALL : "base - Win32 Debug" "$(OUTDIR)\video.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\video\render1x1.obj"
	-@erase "$(INTDIR)\video\render1x1ntsc.obj"
	-@erase "$(INTDIR)\video\render1x1pal.obj"
	-@erase "$(INTDIR)\video\render1x2.obj"
	-@erase "$(INTDIR)\video\render1x2crt.obj"
	-@erase "$(INTDIR)\video\render2x2.obj"
	-@erase "$(INTDIR)\video\render2x2crt.obj"
	-@erase "$(INTDIR)\video\render2x2ntsc.obj"
	-@erase "$(INTDIR)\video\render2x2pal.obj"
	-@erase "$(INTDIR)\video\render2x4.obj"
	-@erase "$(INTDIR)\video\render2x4crt.obj"
	-@erase "$(INTDIR)\video\renderscale2x.obj"
	-@erase "$(INTDIR)\video\renderyuv.obj"
	-@erase "$(INTDIR)\video\video-canvas.obj"
	-@erase "$(INTDIR)\video\video-cmdline-options.obj"
	-@erase "$(INTDIR)\video\video-color.obj"
	-@erase "$(INTDIR)\video\video-render-1x2.obj"
	-@erase "$(INTDIR)\video\video-render-2x2.obj"
	-@erase "$(INTDIR)\video\video-render-crt.obj"
	-@erase "$(INTDIR)\video\video-render-pal.obj"
	-@erase "$(INTDIR)\video\video-render.obj"
	-@erase "$(INTDIR)\video\video-resources.obj"
	-@erase "$(INTDIR)\video\video-sound.obj"
	-@erase "$(INTDIR)\video\video-viewport.obj"
	-@erase "$(OUTDIR)\video.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\c64 "/I "..\..\..\raster "/I "..\..\..\video "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\video.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\video.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\video.lib" 
LIB32_OBJS= \
	"$(INTDIR)\video\render1x1.obj" \
	"$(INTDIR)\video\render1x1ntsc.obj" \
	"$(INTDIR)\video\render1x1pal.obj" \
	"$(INTDIR)\video\render1x2.obj" \
	"$(INTDIR)\video\render1x2crt.obj" \
	"$(INTDIR)\video\render2x2.obj" \
	"$(INTDIR)\video\render2x2crt.obj" \
	"$(INTDIR)\video\render2x2ntsc.obj" \
	"$(INTDIR)\video\render2x2pal.obj" \
	"$(INTDIR)\video\render2x4.obj" \
	"$(INTDIR)\video\render2x4crt.obj" \
	"$(INTDIR)\video\renderscale2x.obj" \
	"$(INTDIR)\video\renderyuv.obj" \
	"$(INTDIR)\video\video-canvas.obj" \
	"$(INTDIR)\video\video-cmdline-options.obj" \
	"$(INTDIR)\video\video-color.obj" \
	"$(INTDIR)\video\video-render-1x2.obj" \
	"$(INTDIR)\video\video-render-2x2.obj" \
	"$(INTDIR)\video\video-render-crt.obj" \
	"$(INTDIR)\video\video-render-pal.obj" \
	"$(INTDIR)\video\video-render.obj" \
	"$(INTDIR)\video\video-resources.obj" \
	"$(INTDIR)\video\video-sound.obj" \
	"$(INTDIR)\video\video-viewport.obj" \
	".\libsbase\Debug\base.lib" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "video - Win32 Release" || "$(CFG)" == "video - Win32 Debug"

!IF  "$(CFG)" == "video - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "video - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\video\render1x1.c

"$(INTDIR)\video\render1x1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\render1x1ntsc.c

"$(INTDIR)\video\render1x1ntsc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\render1x1pal.c

"$(INTDIR)\video\render1x1pal.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\render1x2.c

"$(INTDIR)\video\render1x2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\render1x2crt.c

"$(INTDIR)\video\render1x2crt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\render2x2.c

"$(INTDIR)\video\render2x2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\render2x2crt.c

"$(INTDIR)\video\render2x2crt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\render2x2ntsc.c

"$(INTDIR)\video\render2x2ntsc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\render2x2pal.c

"$(INTDIR)\video\render2x2pal.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\render2x4.c

"$(INTDIR)\video\render2x4.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\render2x4crt.c

"$(INTDIR)\video\render2x4crt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\renderscale2x.c

"$(INTDIR)\video\renderscale2x.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\renderyuv.c

"$(INTDIR)\video\renderyuv.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\video-canvas.c

"$(INTDIR)\video\video-canvas.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\video-cmdline-options.c

"$(INTDIR)\video\video-cmdline-options.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\video-color.c

"$(INTDIR)\video\video-color.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\video-render-1x2.c

"$(INTDIR)\video\video-render-1x2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\video-render-2x2.c

"$(INTDIR)\video\video-render-2x2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\video-render-crt.c

"$(INTDIR)\video\video-render-crt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\video-render-pal.c

"$(INTDIR)\video\video-render-pal.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\video-render.c

"$(INTDIR)\video\video-render.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\video-resources.c

"$(INTDIR)\video\video-resources.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\video-sound.c

"$(INTDIR)\video\video-sound.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\video\video-viewport.c

"$(INTDIR)\video\video-viewport.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


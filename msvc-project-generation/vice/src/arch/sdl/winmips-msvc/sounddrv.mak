# Microsoft Developer Studio Generated NMAKE File, Based on sounddrv.dsp
!IF "$(CFG)" == ""
CFG=sounddrv - Win32 Release
!MESSAGE No configuration specified. Defaulting to sounddrv - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "sounddrv - Win32 Release" && "$(CFG)" != "sounddrv - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sounddrv.mak" CFG="sounddrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sounddrv - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sounddrv - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "sounddrv - Win32 Release"

OUTDIR=.\libs\sounddrv\Release
INTDIR=.\libs\sounddrv\Release
# Begin Custom Macros
OutDir=.\libs\sounddrv\Release
# End Custom Macros

ALL : "$(OUTDIR)\sounddrv.lib" 

CLEAN :
	-@erase "$(INTDIR)\sounddrv\soundaiff.obj"
	-@erase "$(INTDIR)\sounddrv\sounddummy.obj"
	-@erase "$(INTDIR)\sounddrv\sounddump.obj"
	-@erase "$(INTDIR)\sounddrv\sounddx.obj"
	-@erase "$(INTDIR)\sounddrv\soundfs.obj"
	-@erase "$(INTDIR)\sounddrv\soundiff.obj"
	-@erase "$(INTDIR)\sounddrv\soundmovie.obj"
	-@erase "$(INTDIR)\sounddrv\soundvoc.obj"
	-@erase "$(INTDIR)\sounddrv\soundwav.obj"
	-@erase "$(INTDIR)\sounddrv\soundwmm.obj"
	-@erase "$(INTDIR)\sounddrv\soundsdl.obj"
	-@erase "$(OUTDIR)\sounddrv.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\sounddrv.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\sounddrv.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\sounddrv.lib" 
LIB32_OBJS= \
	"$(INTDIR)\sounddrv\soundaiff.obj" \
	"$(INTDIR)\sounddrv\sounddummy.obj" \
	"$(INTDIR)\sounddrv\sounddump.obj" \
	"$(INTDIR)\sounddrv\sounddx.obj" \
	"$(INTDIR)\sounddrv\soundfs.obj" \
	"$(INTDIR)\sounddrv\soundiff.obj" \
	"$(INTDIR)\sounddrv\soundmovie.obj" \
	"$(INTDIR)\sounddrv\soundvoc.obj" \
	"$(INTDIR)\sounddrv\soundwav.obj" \
	"$(INTDIR)\sounddrv\soundwmm.obj" \
	"$(INTDIR)\sounddrv\soundsdl.obj" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\sounddrv\Debug
INTDIR=.\libs\sounddrv\Debug
# Begin Custom Macros
OutDir=.\libs\sounddrv\Debug
# End Custom Macros

ALL : "$(OUTDIR)\sounddrv.lib" 

CLEAN :
	-@erase "$(INTDIR)\sounddrv\soundaiff.obj"
	-@erase "$(INTDIR)\sounddrv\sounddummy.obj"
	-@erase "$(INTDIR)\sounddrv\sounddump.obj"
	-@erase "$(INTDIR)\sounddrv\sounddx.obj"
	-@erase "$(INTDIR)\sounddrv\soundfs.obj"
	-@erase "$(INTDIR)\sounddrv\soundiff.obj"
	-@erase "$(INTDIR)\sounddrv\soundmovie.obj"
	-@erase "$(INTDIR)\sounddrv\soundvoc.obj"
	-@erase "$(INTDIR)\sounddrv\soundwav.obj"
	-@erase "$(INTDIR)\sounddrv\soundwmm.obj"
	-@erase "$(INTDIR)\sounddrv\soundsdl.obj"
	-@erase "$(OUTDIR)\sounddrv.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\sounddrv.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\sounddrv.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\sounddrv.lib" 
LIB32_OBJS= \
	"$(INTDIR)\sounddrv\soundaiff.obj" \
	"$(INTDIR)\sounddrv\sounddummy.obj" \
	"$(INTDIR)\sounddrv\sounddump.obj" \
	"$(INTDIR)\sounddrv\sounddx.obj" \
	"$(INTDIR)\sounddrv\soundfs.obj" \
	"$(INTDIR)\sounddrv\soundiff.obj" \
	"$(INTDIR)\sounddrv\soundmovie.obj" \
	"$(INTDIR)\sounddrv\soundvoc.obj" \
	"$(INTDIR)\sounddrv\soundwav.obj" \
	"$(INTDIR)\sounddrv\soundwmm.obj" \
	"$(INTDIR)\sounddrv\soundsdl.obj" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "sounddrv - Win32 Release" || "$(CFG)" == "sounddrv - Win32 Debug"

SOURCE=..\..\..\sounddrv\soundaiff.c

"$(INTDIR)\sounddrv\soundaiff.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\sounddrv\sounddummy.c

"$(INTDIR)\sounddrv\sounddummy.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\sounddrv\sounddump.c

"$(INTDIR)\sounddrv\sounddump.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\sounddrv\sounddx.c

"$(INTDIR)\sounddrv\sounddx.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\sounddrv\soundfs.c

"$(INTDIR)\sounddrv\soundfs.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\sounddrv\soundiff.c

"$(INTDIR)\sounddrv\soundiff.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\sounddrv\soundmovie.c

"$(INTDIR)\sounddrv\soundmovie.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\sounddrv\soundvoc.c

"$(INTDIR)\sounddrv\soundvoc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\sounddrv\soundwav.c

"$(INTDIR)\sounddrv\soundwav.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\sounddrv\soundwmm.c

"$(INTDIR)\sounddrv\soundwmm.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\sounddrv\soundaiff.c

"$(INTDIR)\sounddrv\soundsdl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


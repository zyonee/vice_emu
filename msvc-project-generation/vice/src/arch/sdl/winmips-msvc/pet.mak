# Microsoft Developer Studio Generated NMAKE File, Based on pet.dsp
!IF "$(CFG)" == ""
CFG=pet - Win32 Release
!MESSAGE No configuration specified. Defaulting to pet - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "pet - Win32 Release" && "$(CFG)" != "pet - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pet.mak" CFG="pet - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pet - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pet - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "pet - Win32 Release"

OUTDIR=.\libs\pet\Release
INTDIR=.\libs\pet\Release
# Begin Custom Macros
OutDir=.\libs\pet\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\pet.lib" 

!ELSE 

ALL : "base - Win32 Release" "$(OUTDIR)\pet.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\pet\6809.obj"
	-@erase "$(INTDIR)\pet\pet-cmdline-options.obj"
	-@erase "$(INTDIR)\pet\pet-resources.obj"
	-@erase "$(INTDIR)\pet\pet-sidcart.obj"
	-@erase "$(INTDIR)\pet\pet-snapshot.obj"
	-@erase "$(INTDIR)\pet\pet.obj"
	-@erase "$(INTDIR)\pet\petacia1.obj"
	-@erase "$(INTDIR)\pet\petbus.obj"
	-@erase "$(INTDIR)\pet\petcolour.obj"
	-@erase "$(INTDIR)\pet\petdatasette.obj"
	-@erase "$(INTDIR)\pet\petdrive.obj"
	-@erase "$(INTDIR)\pet\petdww.obj"
	-@erase "$(INTDIR)\pet\pethre.obj"
	-@erase "$(INTDIR)\pet\petiec.obj"
	-@erase "$(INTDIR)\pet\petmem.obj"
	-@erase "$(INTDIR)\pet\petmemsnapshot.obj"
	-@erase "$(INTDIR)\pet\petmodel.obj"
	-@erase "$(INTDIR)\pet\petpia1.obj"
	-@erase "$(INTDIR)\pet\petpia2.obj"
	-@erase "$(INTDIR)\pet\petprinter.obj"
	-@erase "$(INTDIR)\pet\petreu.obj"
	-@erase "$(INTDIR)\pet\petrom.obj"
	-@erase "$(INTDIR)\pet\petromset.obj"
	-@erase "$(INTDIR)\pet\petsound.obj"
	-@erase "$(INTDIR)\pet\petvia.obj"
	-@erase "$(INTDIR)\pet\petvideo.obj"
	-@erase "$(INTDIR)\pet\petcpu.obj"
	-@erase "$(OUTDIR)\pet.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\crtc "/I "..\..\..\drive "/I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\raster "/I "..\..\..\sid "/I "..\..\..\tape "/I "..\..\..\userport "/I "..\..\..\video "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\pet.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\pet.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\pet.lib" 
LIB32_OBJS= \
	"$(INTDIR)\pet\6809.obj" \
	"$(INTDIR)\pet\pet-cmdline-options.obj" \
	"$(INTDIR)\pet\pet-resources.obj" \
	"$(INTDIR)\pet\pet-sidcart.obj" \
	"$(INTDIR)\pet\pet-snapshot.obj" \
	"$(INTDIR)\pet\pet.obj" \
	"$(INTDIR)\pet\petacia1.obj" \
	"$(INTDIR)\pet\petbus.obj" \
	"$(INTDIR)\pet\petcolour.obj" \
	"$(INTDIR)\pet\petdatasette.obj" \
	"$(INTDIR)\pet\petdrive.obj" \
	"$(INTDIR)\pet\petdww.obj" \
	"$(INTDIR)\pet\pethre.obj" \
	"$(INTDIR)\pet\petiec.obj" \
	"$(INTDIR)\pet\petmem.obj" \
	"$(INTDIR)\pet\petmemsnapshot.obj" \
	"$(INTDIR)\pet\petmodel.obj" \
	"$(INTDIR)\pet\petpia1.obj" \
	"$(INTDIR)\pet\petpia2.obj" \
	"$(INTDIR)\pet\petprinter.obj" \
	"$(INTDIR)\pet\petreu.obj" \
	"$(INTDIR)\pet\petrom.obj" \
	"$(INTDIR)\pet\petromset.obj" \
	"$(INTDIR)\pet\petsound.obj" \
	"$(INTDIR)\pet\petvia.obj" \
	"$(INTDIR)\pet\petvideo.obj" \
	"$(INTDIR)\pet\petcpu.obj" \
	".\libsbase\Release\base.lib" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\pet\Debug
INTDIR=.\libs\pet\Debug
# Begin Custom Macros
OutDir=.\libs\pet\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\pet.lib" 

!ELSE 

ALL : "base - Win32 Debug" "$(OUTDIR)\pet.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\pet\6809.obj"
	-@erase "$(INTDIR)\pet\pet-cmdline-options.obj"
	-@erase "$(INTDIR)\pet\pet-resources.obj"
	-@erase "$(INTDIR)\pet\pet-sidcart.obj"
	-@erase "$(INTDIR)\pet\pet-snapshot.obj"
	-@erase "$(INTDIR)\pet\pet.obj"
	-@erase "$(INTDIR)\pet\petacia1.obj"
	-@erase "$(INTDIR)\pet\petbus.obj"
	-@erase "$(INTDIR)\pet\petcolour.obj"
	-@erase "$(INTDIR)\pet\petdatasette.obj"
	-@erase "$(INTDIR)\pet\petdrive.obj"
	-@erase "$(INTDIR)\pet\petdww.obj"
	-@erase "$(INTDIR)\pet\pethre.obj"
	-@erase "$(INTDIR)\pet\petiec.obj"
	-@erase "$(INTDIR)\pet\petmem.obj"
	-@erase "$(INTDIR)\pet\petmemsnapshot.obj"
	-@erase "$(INTDIR)\pet\petmodel.obj"
	-@erase "$(INTDIR)\pet\petpia1.obj"
	-@erase "$(INTDIR)\pet\petpia2.obj"
	-@erase "$(INTDIR)\pet\petprinter.obj"
	-@erase "$(INTDIR)\pet\petreu.obj"
	-@erase "$(INTDIR)\pet\petrom.obj"
	-@erase "$(INTDIR)\pet\petromset.obj"
	-@erase "$(INTDIR)\pet\petsound.obj"
	-@erase "$(INTDIR)\pet\petvia.obj"
	-@erase "$(INTDIR)\pet\petvideo.obj"
	-@erase "$(INTDIR)\pet\petcpu.obj"
	-@erase "$(OUTDIR)\pet.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\crtc "/I "..\..\..\drive "/I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\raster "/I "..\..\..\sid "/I "..\..\..\tape "/I "..\..\..\userport "/I "..\..\..\video "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\pet.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\pet.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\pet.lib" 
LIB32_OBJS= \
	"$(INTDIR)\pet\6809.obj" \
	"$(INTDIR)\pet\pet-cmdline-options.obj" \
	"$(INTDIR)\pet\pet-resources.obj" \
	"$(INTDIR)\pet\pet-sidcart.obj" \
	"$(INTDIR)\pet\pet-snapshot.obj" \
	"$(INTDIR)\pet\pet.obj" \
	"$(INTDIR)\pet\petacia1.obj" \
	"$(INTDIR)\pet\petbus.obj" \
	"$(INTDIR)\pet\petcolour.obj" \
	"$(INTDIR)\pet\petdatasette.obj" \
	"$(INTDIR)\pet\petdrive.obj" \
	"$(INTDIR)\pet\petdww.obj" \
	"$(INTDIR)\pet\pethre.obj" \
	"$(INTDIR)\pet\petiec.obj" \
	"$(INTDIR)\pet\petmem.obj" \
	"$(INTDIR)\pet\petmemsnapshot.obj" \
	"$(INTDIR)\pet\petmodel.obj" \
	"$(INTDIR)\pet\petpia1.obj" \
	"$(INTDIR)\pet\petpia2.obj" \
	"$(INTDIR)\pet\petprinter.obj" \
	"$(INTDIR)\pet\petreu.obj" \
	"$(INTDIR)\pet\petrom.obj" \
	"$(INTDIR)\pet\petromset.obj" \
	"$(INTDIR)\pet\petsound.obj" \
	"$(INTDIR)\pet\petvia.obj" \
	"$(INTDIR)\pet\petvideo.obj" \
	"$(INTDIR)\pet\petcpu.obj" \
	".\libsbase\Debug\base.lib" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "pet - Win32 Release" || "$(CFG)" == "pet - Win32 Debug"

!IF  "$(CFG)" == "pet - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "pet - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\pet\6809.c

"$(INTDIR)\pet\6809.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\pet-cmdline-options.c

"$(INTDIR)\pet\pet-cmdline-options.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\pet-resources.c

"$(INTDIR)\pet\pet-resources.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\pet-sidcart.c

"$(INTDIR)\pet\pet-sidcart.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\pet-snapshot.c

"$(INTDIR)\pet\pet-snapshot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\pet.c

"$(INTDIR)\pet\pet.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petacia1.c

"$(INTDIR)\pet\petacia1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petbus.c

"$(INTDIR)\pet\petbus.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petcolour.c

"$(INTDIR)\pet\petcolour.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petdatasette.c

"$(INTDIR)\pet\petdatasette.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petdrive.c

"$(INTDIR)\pet\petdrive.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petdww.c

"$(INTDIR)\pet\petdww.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\pethre.c

"$(INTDIR)\pet\pethre.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petiec.c

"$(INTDIR)\pet\petiec.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petmem.c

"$(INTDIR)\pet\petmem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petmemsnapshot.c

"$(INTDIR)\pet\petmemsnapshot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petmodel.c

"$(INTDIR)\pet\petmodel.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petpia1.c

"$(INTDIR)\pet\petpia1.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petpia2.c

"$(INTDIR)\pet\petpia2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petprinter.c

"$(INTDIR)\pet\petprinter.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petreu.c

"$(INTDIR)\pet\petreu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petrom.c

"$(INTDIR)\pet\petrom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petromset.c

"$(INTDIR)\pet\petromset.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petsound.c

"$(INTDIR)\pet\petsound.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petvia.c

"$(INTDIR)\pet\petvia.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petvideo.c

"$(INTDIR)\pet\petvideo.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\pet\petcpu.c

!IF  "$(CFG)" == "pet - Win32 Release"


CPP_SWITCHES=/nologo /MD /W3 /GX /Ot /Oa /Ow /Oi /Op /Oy /Ob2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\crtc" /I "..\..\..\drive" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\raster" /I "..\..\..\sid" /I "..\..\..\tape" /I "..\..\..\userport" /I "..\..\..\video" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\pet.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

"$(INTDIR)\pet\petcpu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<

!ELSEIF  "$(CFG)" == "pet - Win32 Debug"


CPP_SWITCHES=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\crtc" /I "..\..\..\drive" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\raster" /I "..\..\..\sid" /I "..\..\..\tape" /I "..\..\..\userport" /I "..\..\..\video" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\pet.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

"$(INTDIR)\pet\petcpu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<

!ENDIF 



!ENDIF 


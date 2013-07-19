# Microsoft Developer Studio Generated NMAKE File, Based on drive.dsp
!IF "$(CFG)" == ""
CFG=drive - Win32 Release
!MESSAGE No configuration specified. Defaulting to drive - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "drive - Win32 Release" && "$(CFG)" != "drive - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "drive.mak" CFG="drive - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "drive - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "drive - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "drive - Win32 Release"

OUTDIR=.\libs\drive\Release
INTDIR=.\libs\drive\Release
# Begin Custom Macros
OutDir=.\libs\drive\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\drive.lib" 

!ELSE 

ALL : "base - Win32 Release" "$(OUTDIR)\drive.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\drive\drive-check.obj"
	-@erase "$(INTDIR)\drive\drive-cmdline-options.obj"
	-@erase "$(INTDIR)\drive\drive-overflow.obj"
	-@erase "$(INTDIR)\drive\drive-resources.obj"
	-@erase "$(INTDIR)\drive\drive-snapshot.obj"
	-@erase "$(INTDIR)\drive\drive-sound.obj"
	-@erase "$(INTDIR)\drive\drive-writeprotect.obj"
	-@erase "$(INTDIR)\drive\drive.obj"
	-@erase "$(INTDIR)\drive\driveimage.obj"
	-@erase "$(INTDIR)\drive\drivemem.obj"
	-@erase "$(INTDIR)\drive\driverom.obj"
	-@erase "$(INTDIR)\drive\drivesync.obj"
	-@erase "$(INTDIR)\drive\rotation.obj"
	-@erase "$(INTDIR)\drive\drivecpu.obj"
	-@erase "$(INTDIR)\drive\drivecpu65c02.obj"
	-@erase "$(OUTDIR)\drive.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\rtc "/I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\drive.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\drive.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\drive.lib" 
LIB32_OBJS= \
	"$(INTDIR)\drive\drive-check.obj" \
	"$(INTDIR)\drive\drive-cmdline-options.obj" \
	"$(INTDIR)\drive\drive-overflow.obj" \
	"$(INTDIR)\drive\drive-resources.obj" \
	"$(INTDIR)\drive\drive-snapshot.obj" \
	"$(INTDIR)\drive\drive-sound.obj" \
	"$(INTDIR)\drive\drive-writeprotect.obj" \
	"$(INTDIR)\drive\drive.obj" \
	"$(INTDIR)\drive\driveimage.obj" \
	"$(INTDIR)\drive\drivemem.obj" \
	"$(INTDIR)\drive\driverom.obj" \
	"$(INTDIR)\drive\drivesync.obj" \
	"$(INTDIR)\drive\rotation.obj" \
	"$(INTDIR)\drive\drivecpu.obj" \
	"$(INTDIR)\drive\drivecpu65c02.obj" \
	".\libsbase\Release\base.lib" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\drive\Debug
INTDIR=.\libs\drive\Debug
# Begin Custom Macros
OutDir=.\libs\drive\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\drive.lib" 

!ELSE 

ALL : "base - Win32 Debug" "$(OUTDIR)\drive.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\drive\drive-check.obj"
	-@erase "$(INTDIR)\drive\drive-cmdline-options.obj"
	-@erase "$(INTDIR)\drive\drive-overflow.obj"
	-@erase "$(INTDIR)\drive\drive-resources.obj"
	-@erase "$(INTDIR)\drive\drive-snapshot.obj"
	-@erase "$(INTDIR)\drive\drive-sound.obj"
	-@erase "$(INTDIR)\drive\drive-writeprotect.obj"
	-@erase "$(INTDIR)\drive\drive.obj"
	-@erase "$(INTDIR)\drive\driveimage.obj"
	-@erase "$(INTDIR)\drive\drivemem.obj"
	-@erase "$(INTDIR)\drive\driverom.obj"
	-@erase "$(INTDIR)\drive\drivesync.obj"
	-@erase "$(INTDIR)\drive\rotation.obj"
	-@erase "$(INTDIR)\drive\drivecpu.obj"
	-@erase "$(INTDIR)\drive\drivecpu65c02.obj"
	-@erase "$(OUTDIR)\drive.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\rtc "/I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\drive.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\drive.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\drive.lib" 
LIB32_OBJS= \
	"$(INTDIR)\drive\drive-check.obj" \
	"$(INTDIR)\drive\drive-cmdline-options.obj" \
	"$(INTDIR)\drive\drive-overflow.obj" \
	"$(INTDIR)\drive\drive-resources.obj" \
	"$(INTDIR)\drive\drive-snapshot.obj" \
	"$(INTDIR)\drive\drive-sound.obj" \
	"$(INTDIR)\drive\drive-writeprotect.obj" \
	"$(INTDIR)\drive\drive.obj" \
	"$(INTDIR)\drive\driveimage.obj" \
	"$(INTDIR)\drive\drivemem.obj" \
	"$(INTDIR)\drive\driverom.obj" \
	"$(INTDIR)\drive\drivesync.obj" \
	"$(INTDIR)\drive\rotation.obj" \
	"$(INTDIR)\drive\drivecpu.obj" \
	"$(INTDIR)\drive\drivecpu65c02.obj" \
	".\libsbase\Debug\base.lib" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "drive - Win32 Release" || "$(CFG)" == "drive - Win32 Debug"

!IF  "$(CFG)" == "drive - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "drive - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\drive\drive-check.c

"$(INTDIR)\drive\drive-check.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\drive-cmdline-options.c

"$(INTDIR)\drive\drive-cmdline-options.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\drive-overflow.c

"$(INTDIR)\drive\drive-overflow.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\drive-resources.c

"$(INTDIR)\drive\drive-resources.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\drive-snapshot.c

"$(INTDIR)\drive\drive-snapshot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\drive-sound.c

"$(INTDIR)\drive\drive-sound.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\drive-writeprotect.c

"$(INTDIR)\drive\drive-writeprotect.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\drive.c

"$(INTDIR)\drive\drive.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\driveimage.c

"$(INTDIR)\drive\driveimage.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\drivemem.c

"$(INTDIR)\drive\drivemem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\driverom.c

"$(INTDIR)\drive\driverom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\drivesync.c

"$(INTDIR)\drive\drivesync.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\rotation.c

"$(INTDIR)\drive\rotation.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\drive\drivecpu.c

!IF  "$(CFG)" == "drive - Win32 Release"


CPP_SWITCHES=/nologo /MD /W3 /GX /Ot /Oa /Ow /Oi /Op /Oy /Ob2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\rtc" /I "..\..\..\vdrive" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\drive.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

"$(INTDIR)\drive\drivecpu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<

!ELSEIF  "$(CFG)" == "drive - Win32 Debug"


CPP_SWITCHES=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\rtc" /I "..\..\..\vdrive" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\drive.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

"$(INTDIR)\drive\drivecpu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<

!ENDIF 

SOURCE=..\..\..\drive\drivecpu65c02.c

!IF  "$(CFG)" == "drive - Win32 Release"


CPP_SWITCHES=/nologo /MD /W3 /GX /Ot /Oa /Ow /Oi /Op /Oy /Ob2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\rtc" /I "..\..\..\vdrive" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\drive.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

"$(INTDIR)\drive\drivecpu65c02.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<

!ELSEIF  "$(CFG)" == "drive - Win32 Debug"


CPP_SWITCHES=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\rtc" /I "..\..\..\vdrive" /D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\drive.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

"$(INTDIR)\drive\drivecpu65c02.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<

!ENDIF 



!ENDIF 


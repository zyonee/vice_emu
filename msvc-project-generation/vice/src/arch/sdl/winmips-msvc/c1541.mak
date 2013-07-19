# Microsoft Developer Studio Generated NMAKE File, Based on c1541.dsp
!IF "$(CFG)" == ""
CFG=c1541 - Win32 Release
!MESSAGE No configuration specified. Defaulting to c1541 - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "c1541 - Win32 Release" && "$(CFG)" != "c1541 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "c1541.mak" CFG="c1541 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "c1541 - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "c1541 - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "c1541 - Win32 Release"

OUTDIR=.\..\..\..\..\data
INTDIR=.\libs\c1541\Release
# Begin Custom Macros
OutDir=.\..\..\..\..\data
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\c1541.exe" 

!ELSE 

ALL : "arch - Win32 Release" "base - Win32 Release" "diskimage - Win32 Release" "fileio - Win32 Release" "imagecontents - Win32 Release" "p64 - Win32 Release" "platform - Win32 Release" "serial - Win32 Release" "tape - Win32 Release" "vdrive - Win32 Release" "$(OUTDIR)\c1541.exe" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"vdrive - Win32 ReleaseCLEAN" "tape - Win32 ReleaseCLEAN" "serial - Win32 ReleaseCLEAN" "platform - Win32 ReleaseCLEAN" "p64 - Win32 ReleaseCLEAN" "imagecontents - Win32 ReleaseCLEAN" "fileio - Win32 ReleaseCLEAN" "diskimage - Win32 ReleaseCLEAN" "base - Win32 ReleaseCLEAN" "arch - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\c1541.obj"
	-@erase "$(INTDIR)\cbmdos.obj"
	-@erase "$(INTDIR)\cbmimage.obj"
	-@erase "$(INTDIR)\charset.obj"
	-@erase "$(INTDIR)\findpath.obj"
	-@erase "$(INTDIR)\gcr.obj"
	-@erase "$(INTDIR)\info.obj"
	-@erase "$(INTDIR)\ioutil.obj"
	-@erase "$(INTDIR)\lib.obj"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\rawfile.obj"
	-@erase "$(INTDIR)\resources.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\zfile.obj"
	-@erase "$(INTDIR)\zipcode.obj"
	-@erase "$(OUTDIR)\c1541.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\imagecontents "/I "..\..\..\lib\p64 "/I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_CONSOLE" /D "_MBCS" /D "NDEBUG" /Fp"$(INTDIR)\c1541.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\c1541.bsc" 
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib winmm.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib version.lib SDLmain.lib SDL.lib opengl32.lib /nologo /subsystem:console /incremental:no /pdb:"$(OUTDIR)\c1541.pdb" /machine:MIPS /out:"$(OUTDIR)\c1541.exe" 
LINK32_OBJS= \
	"$(INTDIR)\c1541.obj" \
	"$(INTDIR)\cbmdos.obj" \
	"$(INTDIR)\cbmimage.obj" \
	"$(INTDIR)\charset.obj" \
	"$(INTDIR)\findpath.obj" \
	"$(INTDIR)\gcr.obj" \
	"$(INTDIR)\info.obj" \
	"$(INTDIR)\ioutil.obj" \
	"$(INTDIR)\lib.obj" \
	"$(INTDIR)\log.obj" \
	"$(INTDIR)\rawfile.obj" \
	"$(INTDIR)\resources.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\zfile.obj" \
	"$(INTDIR)\zipcode.obj" \
	".\libsarch\Release\arch.lib" \
	".\libsbase\Release\base.lib" \
	".\libsdiskimage\Release\diskimage.lib" \
	".\libsfileio\Release\fileio.lib" \
	".\libsimagecontents\Release\imagecontents.lib" \
	".\libsp64\Release\p64.lib" \
	".\libsplatform\Release\platform.lib" \
	".\libsserial\Release\serial.lib" \
	".\libstape\Release\tape.lib" \
	".\libsvdrive\Release\vdrive.lib" \


"$(OUTDIR)\c1541.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "c1541 - Win32 Debug"

OUTDIR=.\..\..\..\..\data
INTDIR=.\libs\c1541\Debug
# Begin Custom Macros
OutDir=.\..\..\..\..\data
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\c1541.exe" 

!ELSE 

ALL : "arch - Win32 Debug" "base - Win32 Debug" "diskimage - Win32 Debug" "fileio - Win32 Debug" "imagecontents - Win32 Debug" "p64 - Win32 Debug" "platform - Win32 Debug" "serial - Win32 Debug" "tape - Win32 Debug" "vdrive - Win32 Debug" "$(OUTDIR)\c1541.exe" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"vdrive - Win32 DebugCLEAN" "tape - Win32 DebugCLEAN" "serial - Win32 DebugCLEAN" "platform - Win32 DebugCLEAN" "p64 - Win32 DebugCLEAN" "imagecontents - Win32 DebugCLEAN" "fileio - Win32 DebugCLEAN" "diskimage - Win32 DebugCLEAN" "base - Win32 DebugCLEAN" "arch - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\c1541.obj"
	-@erase "$(INTDIR)\cbmdos.obj"
	-@erase "$(INTDIR)\cbmimage.obj"
	-@erase "$(INTDIR)\charset.obj"
	-@erase "$(INTDIR)\findpath.obj"
	-@erase "$(INTDIR)\gcr.obj"
	-@erase "$(INTDIR)\info.obj"
	-@erase "$(INTDIR)\ioutil.obj"
	-@erase "$(INTDIR)\lib.obj"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\rawfile.obj"
	-@erase "$(INTDIR)\resources.obj"
	-@erase "$(INTDIR)\util.obj"
	-@erase "$(INTDIR)\zfile.obj"
	-@erase "$(INTDIR)\zipcode.obj"
	-@erase "$(OUTDIR)\c1541.exe"
	-@erase "$(OUTDIR)\c1541.ilk"
	-@erase "$(OUTDIR)\c1541.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\imagecontents "/I "..\..\..\lib\p64 "/I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_CONSOLE" /D "_MBCS" /D "_DEBUG" /Fp"$(INTDIR)\c1541.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\c1541.bsc" 
BSC32_SBRS= \

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib comctl32.lib winmm.lib ole32.lib oleaut32.lib uuid.lib wsock32.lib version.lib SDLmain.lib SDL.lib opengl32.lib /nologo /subsystem:console /incremental:yes /pdb:"$(OUTDIR)\c1541.pdb" /debug /machine:MIPS /nodefaultlib:"msvcrt.lib" /out:"$(OUTDIR)\c1541.exe" /pdbtype:sept
LINK32_OBJS= \
	"$(INTDIR)\c1541.obj" \
	"$(INTDIR)\cbmdos.obj" \
	"$(INTDIR)\cbmimage.obj" \
	"$(INTDIR)\charset.obj" \
	"$(INTDIR)\findpath.obj" \
	"$(INTDIR)\gcr.obj" \
	"$(INTDIR)\info.obj" \
	"$(INTDIR)\ioutil.obj" \
	"$(INTDIR)\lib.obj" \
	"$(INTDIR)\log.obj" \
	"$(INTDIR)\rawfile.obj" \
	"$(INTDIR)\resources.obj" \
	"$(INTDIR)\util.obj" \
	"$(INTDIR)\zfile.obj" \
	"$(INTDIR)\zipcode.obj" \
	".\libsarch\Debug\arch.lib" \
	".\libsbase\Debug\base.lib" \
	".\libsdiskimage\Debug\diskimage.lib" \
	".\libsfileio\Debug\fileio.lib" \
	".\libsimagecontents\Debug\imagecontents.lib" \
	".\libsp64\Debug\p64.lib" \
	".\libsplatform\Debug\platform.lib" \
	".\libsserial\Debug\serial.lib" \
	".\libstape\Debug\tape.lib" \
	".\libsvdrive\Debug\vdrive.lib" \


"$(OUTDIR)\c1541.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "c1541 - Win32 Release" || "$(CFG)" == "c1541 - Win32 Debug"

!IF  "$(CFG)" == "c1541 - Win32 Release"

"arch - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\arch.mak" CFG="arch - Win32 Release" 
   cd "."

"arch - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\arch.mak" CFG="arch - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "c1541 - Win32 Debug"

"arch - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\arch.mak" CFG="arch - Win32 Debug" 
   cd "."

"arch - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\arch.mak" CFG="arch - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "c1541 - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "c1541 - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "c1541 - Win32 Release"

"diskimage - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\diskimage.mak" CFG="diskimage - Win32 Release" 
   cd "."

"diskimage - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\diskimage.mak" CFG="diskimage - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "c1541 - Win32 Debug"

"diskimage - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\diskimage.mak" CFG="diskimage - Win32 Debug" 
   cd "."

"diskimage - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\diskimage.mak" CFG="diskimage - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "c1541 - Win32 Release"

"fileio - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\fileio.mak" CFG="fileio - Win32 Release" 
   cd "."

"fileio - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\fileio.mak" CFG="fileio - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "c1541 - Win32 Debug"

"fileio - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\fileio.mak" CFG="fileio - Win32 Debug" 
   cd "."

"fileio - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\fileio.mak" CFG="fileio - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "c1541 - Win32 Release"

"imagecontents - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\imagecontents.mak" CFG="imagecontents - Win32 Release" 
   cd "."

"imagecontents - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\imagecontents.mak" CFG="imagecontents - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "c1541 - Win32 Debug"

"imagecontents - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\imagecontents.mak" CFG="imagecontents - Win32 Debug" 
   cd "."

"imagecontents - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\imagecontents.mak" CFG="imagecontents - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "c1541 - Win32 Release"

"p64 - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\p64.mak" CFG="p64 - Win32 Release" 
   cd "."

"p64 - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\p64.mak" CFG="p64 - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "c1541 - Win32 Debug"

"p64 - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\p64.mak" CFG="p64 - Win32 Debug" 
   cd "."

"p64 - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\p64.mak" CFG="p64 - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "c1541 - Win32 Release"

"platform - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\platform.mak" CFG="platform - Win32 Release" 
   cd "."

"platform - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\platform.mak" CFG="platform - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "c1541 - Win32 Debug"

"platform - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\platform.mak" CFG="platform - Win32 Debug" 
   cd "."

"platform - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\platform.mak" CFG="platform - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "c1541 - Win32 Release"

"serial - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\serial.mak" CFG="serial - Win32 Release" 
   cd "."

"serial - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\serial.mak" CFG="serial - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "c1541 - Win32 Debug"

"serial - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\serial.mak" CFG="serial - Win32 Debug" 
   cd "."

"serial - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\serial.mak" CFG="serial - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "c1541 - Win32 Release"

"tape - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\tape.mak" CFG="tape - Win32 Release" 
   cd "."

"tape - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\tape.mak" CFG="tape - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "c1541 - Win32 Debug"

"tape - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\tape.mak" CFG="tape - Win32 Debug" 
   cd "."

"tape - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\tape.mak" CFG="tape - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

!IF  "$(CFG)" == "c1541 - Win32 Release"

"vdrive - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\vdrive.mak" CFG="vdrive - Win32 Release" 
   cd "."

"vdrive - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\vdrive.mak" CFG="vdrive - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "c1541 - Win32 Debug"

"vdrive - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\vdrive.mak" CFG="vdrive - Win32 Debug" 
   cd "."

"vdrive - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\vdrive.mak" CFG="vdrive - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\c1541.c

"$(INTDIR)\c1541.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\cbmdos.c

"$(INTDIR)\cbmdos.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\cbmimage.c

"$(INTDIR)\cbmimage.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\charset.c

"$(INTDIR)\charset.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\findpath.c

"$(INTDIR)\findpath.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\gcr.c

"$(INTDIR)\gcr.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\info.c

"$(INTDIR)\info.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\ioutil.c

"$(INTDIR)\ioutil.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\lib.c

"$(INTDIR)\lib.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\log.c

"$(INTDIR)\log.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\rawfile.c

"$(INTDIR)\rawfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\resources.c

"$(INTDIR)\resources.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\util.c

"$(INTDIR)\util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\zfile.c

"$(INTDIR)\zfile.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\zipcode.c

"$(INTDIR)\zipcode.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


# Microsoft Developer Studio Generated NMAKE File, Based on core.dsp
!IF "$(CFG)" == ""
CFG=core - Win32 Release
!MESSAGE No configuration specified. Defaulting to core - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "core - Win32 Release" && "$(CFG)" != "core - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "core.mak" CFG="core - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "core - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "core - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "core - Win32 Release"

OUTDIR=.\libs\core\Release
INTDIR=.\libs\core\Release
# Begin Custom Macros
OutDir=.\libs\core\Release
# End Custom Macros

ALL : "$(OUTDIR)\core.lib" 

CLEAN :
	-@erase "$(INTDIR)\core\ata.obj"
	-@erase "$(INTDIR)\core\ciacore.obj"
	-@erase "$(INTDIR)\core\ciatimer.obj"
	-@erase "$(INTDIR)\core\cs8900.obj"
	-@erase "$(INTDIR)\core\flash040core.obj"
	-@erase "$(INTDIR)\core\fmopl.obj"
	-@erase "$(INTDIR)\core\mc6821core.obj"
	-@erase "$(INTDIR)\core\riotcore.obj"
	-@erase "$(INTDIR)\core\ser-eeprom.obj"
	-@erase "$(INTDIR)\core\spi-sdcard.obj"
	-@erase "$(INTDIR)\core\t6721.obj"
	-@erase "$(INTDIR)\core\tpicore.obj"
	-@erase "$(INTDIR)\core\viacore.obj"
	-@erase "$(OUTDIR)\core.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\core "/I "..\..\..\monitor "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\core.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\core.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\core.lib" 
LIB32_OBJS= \
	"$(INTDIR)\core\ata.obj" \
	"$(INTDIR)\core\ciacore.obj" \
	"$(INTDIR)\core\ciatimer.obj" \
	"$(INTDIR)\core\cs8900.obj" \
	"$(INTDIR)\core\flash040core.obj" \
	"$(INTDIR)\core\fmopl.obj" \
	"$(INTDIR)\core\mc6821core.obj" \
	"$(INTDIR)\core\riotcore.obj" \
	"$(INTDIR)\core\ser-eeprom.obj" \
	"$(INTDIR)\core\spi-sdcard.obj" \
	"$(INTDIR)\core\t6721.obj" \
	"$(INTDIR)\core\tpicore.obj" \
	"$(INTDIR)\core\viacore.obj" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\core\Debug
INTDIR=.\libs\core\Debug
# Begin Custom Macros
OutDir=.\libs\core\Debug
# End Custom Macros

ALL : "$(OUTDIR)\core.lib" 

CLEAN :
	-@erase "$(INTDIR)\core\ata.obj"
	-@erase "$(INTDIR)\core\ciacore.obj"
	-@erase "$(INTDIR)\core\ciatimer.obj"
	-@erase "$(INTDIR)\core\cs8900.obj"
	-@erase "$(INTDIR)\core\flash040core.obj"
	-@erase "$(INTDIR)\core\fmopl.obj"
	-@erase "$(INTDIR)\core\mc6821core.obj"
	-@erase "$(INTDIR)\core\riotcore.obj"
	-@erase "$(INTDIR)\core\ser-eeprom.obj"
	-@erase "$(INTDIR)\core\spi-sdcard.obj"
	-@erase "$(INTDIR)\core\t6721.obj"
	-@erase "$(INTDIR)\core\tpicore.obj"
	-@erase "$(INTDIR)\core\viacore.obj"
	-@erase "$(OUTDIR)\core.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\core "/I "..\..\..\monitor "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\core.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\core.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\core.lib" 
LIB32_OBJS= \
	"$(INTDIR)\core\ata.obj" \
	"$(INTDIR)\core\ciacore.obj" \
	"$(INTDIR)\core\ciatimer.obj" \
	"$(INTDIR)\core\cs8900.obj" \
	"$(INTDIR)\core\flash040core.obj" \
	"$(INTDIR)\core\fmopl.obj" \
	"$(INTDIR)\core\mc6821core.obj" \
	"$(INTDIR)\core\riotcore.obj" \
	"$(INTDIR)\core\ser-eeprom.obj" \
	"$(INTDIR)\core\spi-sdcard.obj" \
	"$(INTDIR)\core\t6721.obj" \
	"$(INTDIR)\core\tpicore.obj" \
	"$(INTDIR)\core\viacore.obj" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "core - Win32 Release" || "$(CFG)" == "core - Win32 Debug"

SOURCE=..\..\..\core\ata.c

"$(INTDIR)\core\ata.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\core\ciacore.c

"$(INTDIR)\core\ciacore.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\core\ciatimer.c

"$(INTDIR)\core\ciatimer.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\core\cs8900.c

"$(INTDIR)\core\cs8900.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\core\flash040core.c

"$(INTDIR)\core\flash040core.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\core\fmopl.c

"$(INTDIR)\core\fmopl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\core\mc6821core.c

"$(INTDIR)\core\mc6821core.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\core\riotcore.c

"$(INTDIR)\core\riotcore.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\core\ser-eeprom.c

"$(INTDIR)\core\ser-eeprom.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\core\spi-sdcard.c

"$(INTDIR)\core\spi-sdcard.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\core\t6721.c

"$(INTDIR)\core\t6721.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\core\tpicore.c

"$(INTDIR)\core\tpicore.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\core\viacore.c

"$(INTDIR)\core\viacore.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


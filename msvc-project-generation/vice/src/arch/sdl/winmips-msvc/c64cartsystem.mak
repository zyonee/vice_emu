# Microsoft Developer Studio Generated NMAKE File, Based on c64cartsystem.dsp
!IF "$(CFG)" == ""
CFG=c64cartsystem - Win32 Release
!MESSAGE No configuration specified. Defaulting to c64cartsystem - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "c64cartsystem - Win32 Release" && "$(CFG)" != "c64cartsystem - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "c64cartsystem.mak" CFG="c64cartsystem - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "c64cartsystem - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "c64cartsystem - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "c64cartsystem - Win32 Release"

OUTDIR=.\libs\c64cartsystem\Release
INTDIR=.\libs\c64cartsystem\Release
# Begin Custom Macros
OutDir=.\libs\c64cartsystem\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\c64cartsystem.lib" 

!ELSE 

ALL : "base - Win32 Release" "$(OUTDIR)\c64cartsystem.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\c64\cart\c64cart.obj"
	-@erase "$(INTDIR)\c64\cart\c64carthooks.obj"
	-@erase "$(INTDIR)\c64\cart\c64cartmem.obj"
	-@erase "$(INTDIR)\c64\cart\crt.obj"
	-@erase "$(OUTDIR)\c64cartsystem.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\c64 "/I "..\..\..\core "/I "..\..\..\drive "/I "..\..\..\monitor "/I "..\..\..\rtc "/I "..\..\..\sid "/I "..\..\..\vicii "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\c64cartsystem.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\c64cartsystem.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\c64cartsystem.lib" 
LIB32_OBJS= \
	"$(INTDIR)\c64\cart\c64cart.obj" \
	"$(INTDIR)\c64\cart\c64carthooks.obj" \
	"$(INTDIR)\c64\cart\c64cartmem.obj" \
	"$(INTDIR)\c64\cart\crt.obj" \
	".\libsbase\Release\base.lib" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\c64cartsystem\Debug
INTDIR=.\libs\c64cartsystem\Debug
# Begin Custom Macros
OutDir=.\libs\c64cartsystem\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\c64cartsystem.lib" 

!ELSE 

ALL : "base - Win32 Debug" "$(OUTDIR)\c64cartsystem.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\c64\cart\c64cart.obj"
	-@erase "$(INTDIR)\c64\cart\c64carthooks.obj"
	-@erase "$(INTDIR)\c64\cart\c64cartmem.obj"
	-@erase "$(INTDIR)\c64\cart\crt.obj"
	-@erase "$(OUTDIR)\c64cartsystem.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\c64 "/I "..\..\..\core "/I "..\..\..\drive "/I "..\..\..\monitor "/I "..\..\..\rtc "/I "..\..\..\sid "/I "..\..\..\vicii "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\c64cartsystem.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\c64cartsystem.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\c64cartsystem.lib" 
LIB32_OBJS= \
	"$(INTDIR)\c64\cart\c64cart.obj" \
	"$(INTDIR)\c64\cart\c64carthooks.obj" \
	"$(INTDIR)\c64\cart\c64cartmem.obj" \
	"$(INTDIR)\c64\cart\crt.obj" \
	".\libsbase\Debug\base.lib" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "c64cartsystem - Win32 Release" || "$(CFG)" == "c64cartsystem - Win32 Debug"

!IF  "$(CFG)" == "c64cartsystem - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "c64cartsystem - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\c64\cart\c64cart.c

"$(INTDIR)\c64\cart\c64cart.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\c64carthooks.c

"$(INTDIR)\c64\cart\c64carthooks.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\c64cartmem.c

"$(INTDIR)\c64\cart\c64cartmem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\crt.c

"$(INTDIR)\c64\cart\crt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


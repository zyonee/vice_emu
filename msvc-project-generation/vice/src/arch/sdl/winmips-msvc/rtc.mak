# Microsoft Developer Studio Generated NMAKE File, Based on rtc.dsp
!IF "$(CFG)" == ""
CFG=rtc - Win32 Release
!MESSAGE No configuration specified. Defaulting to rtc - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "rtc - Win32 Release" && "$(CFG)" != "rtc - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "rtc.mak" CFG="rtc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "rtc - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "rtc - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "rtc - Win32 Release"

OUTDIR=.\libs\rtc\Release
INTDIR=.\libs\rtc\Release
# Begin Custom Macros
OutDir=.\libs\rtc\Release
# End Custom Macros

ALL : "$(OUTDIR)\rtc.lib" 

CLEAN :
	-@erase "$(INTDIR)\rtc\bq4830y.obj"
	-@erase "$(INTDIR)\rtc\ds1202_1302.obj"
	-@erase "$(INTDIR)\rtc\ds1216e.obj"
	-@erase "$(INTDIR)\rtc\ds12c887.obj"
	-@erase "$(INTDIR)\rtc\rtc.obj"
	-@erase "$(INTDIR)\rtc\rtc-58321a.obj"
	-@erase "$(OUTDIR)\rtc.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\imagecontents "/I "..\..\..\monitor "/I "..\..\..\rs232drv "/I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\rtc.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\rtc.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\rtc.lib" 
LIB32_OBJS= \
	"$(INTDIR)\rtc\bq4830y.obj" \
	"$(INTDIR)\rtc\ds1202_1302.obj" \
	"$(INTDIR)\rtc\ds1216e.obj" \
	"$(INTDIR)\rtc\ds12c887.obj" \
	"$(INTDIR)\rtc\rtc.obj" \
	"$(INTDIR)\rtc\rtc-58321a.obj" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\rtc\Debug
INTDIR=.\libs\rtc\Debug
# Begin Custom Macros
OutDir=.\libs\rtc\Debug
# End Custom Macros

ALL : "$(OUTDIR)\rtc.lib" 

CLEAN :
	-@erase "$(INTDIR)\rtc\bq4830y.obj"
	-@erase "$(INTDIR)\rtc\ds1202_1302.obj"
	-@erase "$(INTDIR)\rtc\ds1216e.obj"
	-@erase "$(INTDIR)\rtc\ds12c887.obj"
	-@erase "$(INTDIR)\rtc\rtc.obj"
	-@erase "$(INTDIR)\rtc\rtc-58321a.obj"
	-@erase "$(OUTDIR)\rtc.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\imagecontents "/I "..\..\..\monitor "/I "..\..\..\rs232drv "/I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\rtc.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\rtc.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\rtc.lib" 
LIB32_OBJS= \
	"$(INTDIR)\rtc\bq4830y.obj" \
	"$(INTDIR)\rtc\ds1202_1302.obj" \
	"$(INTDIR)\rtc\ds1216e.obj" \
	"$(INTDIR)\rtc\ds12c887.obj" \
	"$(INTDIR)\rtc\rtc.obj" \
	"$(INTDIR)\rtc\rtc-58321a.obj" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "rtc - Win32 Release" || "$(CFG)" == "rtc - Win32 Debug"

SOURCE=..\..\..\rtc\bq4830y.c

"$(INTDIR)\rtc\bq4830y.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\rtc\ds1202_1302.c

"$(INTDIR)\rtc\ds1202_1302.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\rtc\ds1216e.c

"$(INTDIR)\rtc\ds1216e.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\rtc\ds12c887.c

"$(INTDIR)\rtc\ds12c887.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\rtc\rtc.c

"$(INTDIR)\rtc\rtc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\rtc\rtc-58321a.c

"$(INTDIR)\rtc\rtc-58321a.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


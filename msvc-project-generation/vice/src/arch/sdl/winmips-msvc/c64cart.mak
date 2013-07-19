# Microsoft Developer Studio Generated NMAKE File, Based on c64cart.dsp
!IF "$(CFG)" == ""
CFG=c64cart - Win32 Release
!MESSAGE No configuration specified. Defaulting to c64cart - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "c64cart - Win32 Release" && "$(CFG)" != "c64cart - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "c64cart.mak" CFG="c64cart - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "c64cart - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "c64cart - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "c64cart - Win32 Release"

OUTDIR=.\libs\c64cart\Release
INTDIR=.\libs\c64cart\Release
# Begin Custom Macros
OutDir=.\libs\c64cart\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\c64cart.lib" 

!ELSE 

ALL : "base - Win32 Release" "$(OUTDIR)\c64cart.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\c64\cart\actionreplay.obj"
	-@erase "$(INTDIR)\c64\cart\actionreplay2.obj"
	-@erase "$(INTDIR)\c64\cart\actionreplay3.obj"
	-@erase "$(INTDIR)\c64\cart\actionreplay4.obj"
	-@erase "$(INTDIR)\c64\cart\atomicpower.obj"
	-@erase "$(INTDIR)\c64\cart\c64-generic.obj"
	-@erase "$(INTDIR)\c64\cart\c64-midi.obj"
	-@erase "$(INTDIR)\c64\cart\c64tpi.obj"
	-@erase "$(INTDIR)\c64\cart\capture.obj"
	-@erase "$(INTDIR)\c64\cart\comal80.obj"
	-@erase "$(INTDIR)\c64\cart\delaep256.obj"
	-@erase "$(INTDIR)\c64\cart\delaep64.obj"
	-@erase "$(INTDIR)\c64\cart\delaep7x8.obj"
	-@erase "$(INTDIR)\c64\cart\diashowmaker.obj"
	-@erase "$(INTDIR)\c64\cart\dinamic.obj"
	-@erase "$(INTDIR)\c64\cart\dqbb.obj"
	-@erase "$(INTDIR)\c64\cart\easyflash.obj"
	-@erase "$(INTDIR)\c64\cart\epyxfastload.obj"
	-@erase "$(INTDIR)\c64\cart\exos.obj"
	-@erase "$(INTDIR)\c64\cart\expert.obj"
	-@erase "$(INTDIR)\c64\cart\final.obj"
	-@erase "$(INTDIR)\c64\cart\final3.obj"
	-@erase "$(INTDIR)\c64\cart\finalplus.obj"
	-@erase "$(INTDIR)\c64\cart\formel64.obj"
	-@erase "$(INTDIR)\c64\cart\freezeframe.obj"
	-@erase "$(INTDIR)\c64\cart\freezemachine.obj"
	-@erase "$(INTDIR)\c64\cart\funplay.obj"
	-@erase "$(INTDIR)\c64\cart\gamekiller.obj"
	-@erase "$(INTDIR)\c64\cart\gs.obj"
	-@erase "$(INTDIR)\c64\cart\ide64.obj"
	-@erase "$(INTDIR)\c64\cart\isepic.obj"
	-@erase "$(INTDIR)\c64\cart\kcs.obj"
	-@erase "$(INTDIR)\c64\cart\kingsoft.obj"
	-@erase "$(INTDIR)\c64\cart\mach5.obj"
	-@erase "$(INTDIR)\c64\cart\magicdesk.obj"
	-@erase "$(INTDIR)\c64\cart\magicformel.obj"
	-@erase "$(INTDIR)\c64\cart\magicvoice.obj"
	-@erase "$(INTDIR)\c64\cart\mikroass.obj"
	-@erase "$(INTDIR)\c64\cart\mmc64.obj"
	-@erase "$(INTDIR)\c64\cart\mmcreplay.obj"
	-@erase "$(INTDIR)\c64\cart\ocean.obj"
	-@erase "$(INTDIR)\c64\cart\pagefox.obj"
	-@erase "$(INTDIR)\c64\cart\prophet64.obj"
	-@erase "$(INTDIR)\c64\cart\ramcart.obj"
	-@erase "$(INTDIR)\c64\cart\retroreplay.obj"
	-@erase "$(INTDIR)\c64\cart\reu.obj"
	-@erase "$(INTDIR)\c64\cart\rexep256.obj"
	-@erase "$(INTDIR)\c64\cart\rexutility.obj"
	-@erase "$(INTDIR)\c64\cart\ross.obj"
	-@erase "$(INTDIR)\c64\cart\silverrock128.obj"
	-@erase "$(INTDIR)\c64\cart\simonsbasic.obj"
	-@erase "$(INTDIR)\c64\cart\snapshot64.obj"
	-@erase "$(INTDIR)\c64\cart\stardos.obj"
	-@erase "$(INTDIR)\c64\cart\stb.obj"
	-@erase "$(INTDIR)\c64\cart\superexplode5.obj"
	-@erase "$(INTDIR)\c64\cart\supergames.obj"
	-@erase "$(INTDIR)\c64\cart\supersnapshot.obj"
	-@erase "$(INTDIR)\c64\cart\supersnapshot4.obj"
	-@erase "$(INTDIR)\c64\cart\warpspeed.obj"
	-@erase "$(INTDIR)\c64\cart\westermann.obj"
	-@erase "$(INTDIR)\c64\cart\zaxxon.obj"
	-@erase "$(OUTDIR)\c64cart.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\c64 "/I "..\..\..\core "/I "..\..\..\drive "/I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\rtc "/I "..\..\..\sid "/I "..\..\..\vicii "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\c64cart.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\c64cart.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\c64cart.lib" 
LIB32_OBJS= \
	"$(INTDIR)\c64\cart\actionreplay.obj" \
	"$(INTDIR)\c64\cart\actionreplay2.obj" \
	"$(INTDIR)\c64\cart\actionreplay3.obj" \
	"$(INTDIR)\c64\cart\actionreplay4.obj" \
	"$(INTDIR)\c64\cart\atomicpower.obj" \
	"$(INTDIR)\c64\cart\c64-generic.obj" \
	"$(INTDIR)\c64\cart\c64-midi.obj" \
	"$(INTDIR)\c64\cart\c64tpi.obj" \
	"$(INTDIR)\c64\cart\capture.obj" \
	"$(INTDIR)\c64\cart\comal80.obj" \
	"$(INTDIR)\c64\cart\delaep256.obj" \
	"$(INTDIR)\c64\cart\delaep64.obj" \
	"$(INTDIR)\c64\cart\delaep7x8.obj" \
	"$(INTDIR)\c64\cart\diashowmaker.obj" \
	"$(INTDIR)\c64\cart\dinamic.obj" \
	"$(INTDIR)\c64\cart\dqbb.obj" \
	"$(INTDIR)\c64\cart\easyflash.obj" \
	"$(INTDIR)\c64\cart\epyxfastload.obj" \
	"$(INTDIR)\c64\cart\exos.obj" \
	"$(INTDIR)\c64\cart\expert.obj" \
	"$(INTDIR)\c64\cart\final.obj" \
	"$(INTDIR)\c64\cart\final3.obj" \
	"$(INTDIR)\c64\cart\finalplus.obj" \
	"$(INTDIR)\c64\cart\formel64.obj" \
	"$(INTDIR)\c64\cart\freezeframe.obj" \
	"$(INTDIR)\c64\cart\freezemachine.obj" \
	"$(INTDIR)\c64\cart\funplay.obj" \
	"$(INTDIR)\c64\cart\gamekiller.obj" \
	"$(INTDIR)\c64\cart\gs.obj" \
	"$(INTDIR)\c64\cart\ide64.obj" \
	"$(INTDIR)\c64\cart\isepic.obj" \
	"$(INTDIR)\c64\cart\kcs.obj" \
	"$(INTDIR)\c64\cart\kingsoft.obj" \
	"$(INTDIR)\c64\cart\mach5.obj" \
	"$(INTDIR)\c64\cart\magicdesk.obj" \
	"$(INTDIR)\c64\cart\magicformel.obj" \
	"$(INTDIR)\c64\cart\magicvoice.obj" \
	"$(INTDIR)\c64\cart\mikroass.obj" \
	"$(INTDIR)\c64\cart\mmc64.obj" \
	"$(INTDIR)\c64\cart\mmcreplay.obj" \
	"$(INTDIR)\c64\cart\ocean.obj" \
	"$(INTDIR)\c64\cart\pagefox.obj" \
	"$(INTDIR)\c64\cart\prophet64.obj" \
	"$(INTDIR)\c64\cart\ramcart.obj" \
	"$(INTDIR)\c64\cart\retroreplay.obj" \
	"$(INTDIR)\c64\cart\reu.obj" \
	"$(INTDIR)\c64\cart\rexep256.obj" \
	"$(INTDIR)\c64\cart\rexutility.obj" \
	"$(INTDIR)\c64\cart\ross.obj" \
	"$(INTDIR)\c64\cart\silverrock128.obj" \
	"$(INTDIR)\c64\cart\simonsbasic.obj" \
	"$(INTDIR)\c64\cart\snapshot64.obj" \
	"$(INTDIR)\c64\cart\stardos.obj" \
	"$(INTDIR)\c64\cart\stb.obj" \
	"$(INTDIR)\c64\cart\superexplode5.obj" \
	"$(INTDIR)\c64\cart\supergames.obj" \
	"$(INTDIR)\c64\cart\supersnapshot.obj" \
	"$(INTDIR)\c64\cart\supersnapshot4.obj" \
	"$(INTDIR)\c64\cart\warpspeed.obj" \
	"$(INTDIR)\c64\cart\westermann.obj" \
	"$(INTDIR)\c64\cart\zaxxon.obj" \
	".\libsbase\Release\base.lib" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\c64cart\Debug
INTDIR=.\libs\c64cart\Debug
# Begin Custom Macros
OutDir=.\libs\c64cart\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\c64cart.lib" 

!ELSE 

ALL : "base - Win32 Debug" "$(OUTDIR)\c64cart.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\c64\cart\actionreplay.obj"
	-@erase "$(INTDIR)\c64\cart\actionreplay2.obj"
	-@erase "$(INTDIR)\c64\cart\actionreplay3.obj"
	-@erase "$(INTDIR)\c64\cart\actionreplay4.obj"
	-@erase "$(INTDIR)\c64\cart\atomicpower.obj"
	-@erase "$(INTDIR)\c64\cart\c64-generic.obj"
	-@erase "$(INTDIR)\c64\cart\c64-midi.obj"
	-@erase "$(INTDIR)\c64\cart\c64tpi.obj"
	-@erase "$(INTDIR)\c64\cart\capture.obj"
	-@erase "$(INTDIR)\c64\cart\comal80.obj"
	-@erase "$(INTDIR)\c64\cart\delaep256.obj"
	-@erase "$(INTDIR)\c64\cart\delaep64.obj"
	-@erase "$(INTDIR)\c64\cart\delaep7x8.obj"
	-@erase "$(INTDIR)\c64\cart\diashowmaker.obj"
	-@erase "$(INTDIR)\c64\cart\dinamic.obj"
	-@erase "$(INTDIR)\c64\cart\dqbb.obj"
	-@erase "$(INTDIR)\c64\cart\easyflash.obj"
	-@erase "$(INTDIR)\c64\cart\epyxfastload.obj"
	-@erase "$(INTDIR)\c64\cart\exos.obj"
	-@erase "$(INTDIR)\c64\cart\expert.obj"
	-@erase "$(INTDIR)\c64\cart\final.obj"
	-@erase "$(INTDIR)\c64\cart\final3.obj"
	-@erase "$(INTDIR)\c64\cart\finalplus.obj"
	-@erase "$(INTDIR)\c64\cart\formel64.obj"
	-@erase "$(INTDIR)\c64\cart\freezeframe.obj"
	-@erase "$(INTDIR)\c64\cart\freezemachine.obj"
	-@erase "$(INTDIR)\c64\cart\funplay.obj"
	-@erase "$(INTDIR)\c64\cart\gamekiller.obj"
	-@erase "$(INTDIR)\c64\cart\gs.obj"
	-@erase "$(INTDIR)\c64\cart\ide64.obj"
	-@erase "$(INTDIR)\c64\cart\isepic.obj"
	-@erase "$(INTDIR)\c64\cart\kcs.obj"
	-@erase "$(INTDIR)\c64\cart\kingsoft.obj"
	-@erase "$(INTDIR)\c64\cart\mach5.obj"
	-@erase "$(INTDIR)\c64\cart\magicdesk.obj"
	-@erase "$(INTDIR)\c64\cart\magicformel.obj"
	-@erase "$(INTDIR)\c64\cart\magicvoice.obj"
	-@erase "$(INTDIR)\c64\cart\mikroass.obj"
	-@erase "$(INTDIR)\c64\cart\mmc64.obj"
	-@erase "$(INTDIR)\c64\cart\mmcreplay.obj"
	-@erase "$(INTDIR)\c64\cart\ocean.obj"
	-@erase "$(INTDIR)\c64\cart\pagefox.obj"
	-@erase "$(INTDIR)\c64\cart\prophet64.obj"
	-@erase "$(INTDIR)\c64\cart\ramcart.obj"
	-@erase "$(INTDIR)\c64\cart\retroreplay.obj"
	-@erase "$(INTDIR)\c64\cart\reu.obj"
	-@erase "$(INTDIR)\c64\cart\rexep256.obj"
	-@erase "$(INTDIR)\c64\cart\rexutility.obj"
	-@erase "$(INTDIR)\c64\cart\ross.obj"
	-@erase "$(INTDIR)\c64\cart\silverrock128.obj"
	-@erase "$(INTDIR)\c64\cart\simonsbasic.obj"
	-@erase "$(INTDIR)\c64\cart\snapshot64.obj"
	-@erase "$(INTDIR)\c64\cart\stardos.obj"
	-@erase "$(INTDIR)\c64\cart\stb.obj"
	-@erase "$(INTDIR)\c64\cart\superexplode5.obj"
	-@erase "$(INTDIR)\c64\cart\supergames.obj"
	-@erase "$(INTDIR)\c64\cart\supersnapshot.obj"
	-@erase "$(INTDIR)\c64\cart\supersnapshot4.obj"
	-@erase "$(INTDIR)\c64\cart\warpspeed.obj"
	-@erase "$(INTDIR)\c64\cart\westermann.obj"
	-@erase "$(INTDIR)\c64\cart\zaxxon.obj"
	-@erase "$(OUTDIR)\c64cart.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\c64 "/I "..\..\..\core "/I "..\..\..\drive "/I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\rtc "/I "..\..\..\sid "/I "..\..\..\vicii "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\c64cart.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\c64cart.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\c64cart.lib" 
LIB32_OBJS= \
	"$(INTDIR)\c64\cart\actionreplay.obj" \
	"$(INTDIR)\c64\cart\actionreplay2.obj" \
	"$(INTDIR)\c64\cart\actionreplay3.obj" \
	"$(INTDIR)\c64\cart\actionreplay4.obj" \
	"$(INTDIR)\c64\cart\atomicpower.obj" \
	"$(INTDIR)\c64\cart\c64-generic.obj" \
	"$(INTDIR)\c64\cart\c64-midi.obj" \
	"$(INTDIR)\c64\cart\c64tpi.obj" \
	"$(INTDIR)\c64\cart\capture.obj" \
	"$(INTDIR)\c64\cart\comal80.obj" \
	"$(INTDIR)\c64\cart\delaep256.obj" \
	"$(INTDIR)\c64\cart\delaep64.obj" \
	"$(INTDIR)\c64\cart\delaep7x8.obj" \
	"$(INTDIR)\c64\cart\diashowmaker.obj" \
	"$(INTDIR)\c64\cart\dinamic.obj" \
	"$(INTDIR)\c64\cart\dqbb.obj" \
	"$(INTDIR)\c64\cart\easyflash.obj" \
	"$(INTDIR)\c64\cart\epyxfastload.obj" \
	"$(INTDIR)\c64\cart\exos.obj" \
	"$(INTDIR)\c64\cart\expert.obj" \
	"$(INTDIR)\c64\cart\final.obj" \
	"$(INTDIR)\c64\cart\final3.obj" \
	"$(INTDIR)\c64\cart\finalplus.obj" \
	"$(INTDIR)\c64\cart\formel64.obj" \
	"$(INTDIR)\c64\cart\freezeframe.obj" \
	"$(INTDIR)\c64\cart\freezemachine.obj" \
	"$(INTDIR)\c64\cart\funplay.obj" \
	"$(INTDIR)\c64\cart\gamekiller.obj" \
	"$(INTDIR)\c64\cart\gs.obj" \
	"$(INTDIR)\c64\cart\ide64.obj" \
	"$(INTDIR)\c64\cart\isepic.obj" \
	"$(INTDIR)\c64\cart\kcs.obj" \
	"$(INTDIR)\c64\cart\kingsoft.obj" \
	"$(INTDIR)\c64\cart\mach5.obj" \
	"$(INTDIR)\c64\cart\magicdesk.obj" \
	"$(INTDIR)\c64\cart\magicformel.obj" \
	"$(INTDIR)\c64\cart\magicvoice.obj" \
	"$(INTDIR)\c64\cart\mikroass.obj" \
	"$(INTDIR)\c64\cart\mmc64.obj" \
	"$(INTDIR)\c64\cart\mmcreplay.obj" \
	"$(INTDIR)\c64\cart\ocean.obj" \
	"$(INTDIR)\c64\cart\pagefox.obj" \
	"$(INTDIR)\c64\cart\prophet64.obj" \
	"$(INTDIR)\c64\cart\ramcart.obj" \
	"$(INTDIR)\c64\cart\retroreplay.obj" \
	"$(INTDIR)\c64\cart\reu.obj" \
	"$(INTDIR)\c64\cart\rexep256.obj" \
	"$(INTDIR)\c64\cart\rexutility.obj" \
	"$(INTDIR)\c64\cart\ross.obj" \
	"$(INTDIR)\c64\cart\silverrock128.obj" \
	"$(INTDIR)\c64\cart\simonsbasic.obj" \
	"$(INTDIR)\c64\cart\snapshot64.obj" \
	"$(INTDIR)\c64\cart\stardos.obj" \
	"$(INTDIR)\c64\cart\stb.obj" \
	"$(INTDIR)\c64\cart\superexplode5.obj" \
	"$(INTDIR)\c64\cart\supergames.obj" \
	"$(INTDIR)\c64\cart\supersnapshot.obj" \
	"$(INTDIR)\c64\cart\supersnapshot4.obj" \
	"$(INTDIR)\c64\cart\warpspeed.obj" \
	"$(INTDIR)\c64\cart\westermann.obj" \
	"$(INTDIR)\c64\cart\zaxxon.obj" \
	".\libsbase\Debug\base.lib" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "c64cart - Win32 Release" || "$(CFG)" == "c64cart - Win32 Debug"

!IF  "$(CFG)" == "c64cart - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "c64cart - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\c64\cart\actionreplay.c

"$(INTDIR)\c64\cart\actionreplay.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\actionreplay2.c

"$(INTDIR)\c64\cart\actionreplay2.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\actionreplay3.c

"$(INTDIR)\c64\cart\actionreplay3.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\actionreplay4.c

"$(INTDIR)\c64\cart\actionreplay4.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\atomicpower.c

"$(INTDIR)\c64\cart\atomicpower.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\c64-generic.c

"$(INTDIR)\c64\cart\c64-generic.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\c64-midi.c

"$(INTDIR)\c64\cart\c64-midi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\c64tpi.c

"$(INTDIR)\c64\cart\c64tpi.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\capture.c

"$(INTDIR)\c64\cart\capture.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\comal80.c

"$(INTDIR)\c64\cart\comal80.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\delaep256.c

"$(INTDIR)\c64\cart\delaep256.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\delaep64.c

"$(INTDIR)\c64\cart\delaep64.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\delaep7x8.c

"$(INTDIR)\c64\cart\delaep7x8.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\diashowmaker.c

"$(INTDIR)\c64\cart\diashowmaker.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\dinamic.c

"$(INTDIR)\c64\cart\dinamic.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\dqbb.c

"$(INTDIR)\c64\cart\dqbb.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\easyflash.c

"$(INTDIR)\c64\cart\easyflash.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\epyxfastload.c

"$(INTDIR)\c64\cart\epyxfastload.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\exos.c

"$(INTDIR)\c64\cart\exos.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\expert.c

"$(INTDIR)\c64\cart\expert.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\final.c

"$(INTDIR)\c64\cart\final.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\final3.c

"$(INTDIR)\c64\cart\final3.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\finalplus.c

"$(INTDIR)\c64\cart\finalplus.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\formel64.c

"$(INTDIR)\c64\cart\formel64.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\freezeframe.c

"$(INTDIR)\c64\cart\freezeframe.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\freezemachine.c

"$(INTDIR)\c64\cart\freezemachine.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\funplay.c

"$(INTDIR)\c64\cart\funplay.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\gamekiller.c

"$(INTDIR)\c64\cart\gamekiller.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\gs.c

"$(INTDIR)\c64\cart\gs.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\ide64.c

"$(INTDIR)\c64\cart\ide64.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\isepic.c

"$(INTDIR)\c64\cart\isepic.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\kcs.c

"$(INTDIR)\c64\cart\kcs.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\kingsoft.c

"$(INTDIR)\c64\cart\kingsoft.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\mach5.c

"$(INTDIR)\c64\cart\mach5.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\magicdesk.c

"$(INTDIR)\c64\cart\magicdesk.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\magicformel.c

"$(INTDIR)\c64\cart\magicformel.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\magicvoice.c

"$(INTDIR)\c64\cart\magicvoice.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\mikroass.c

"$(INTDIR)\c64\cart\mikroass.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\mmc64.c

"$(INTDIR)\c64\cart\mmc64.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\mmcreplay.c

"$(INTDIR)\c64\cart\mmcreplay.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\ocean.c

"$(INTDIR)\c64\cart\ocean.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\pagefox.c

"$(INTDIR)\c64\cart\pagefox.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\prophet64.c

"$(INTDIR)\c64\cart\prophet64.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\ramcart.c

"$(INTDIR)\c64\cart\ramcart.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\retroreplay.c

"$(INTDIR)\c64\cart\retroreplay.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\reu.c

"$(INTDIR)\c64\cart\reu.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\rexep256.c

"$(INTDIR)\c64\cart\rexep256.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\rexutility.c

"$(INTDIR)\c64\cart\rexutility.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\ross.c

"$(INTDIR)\c64\cart\ross.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\silverrock128.c

"$(INTDIR)\c64\cart\silverrock128.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\simonsbasic.c

"$(INTDIR)\c64\cart\simonsbasic.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\snapshot64.c

"$(INTDIR)\c64\cart\snapshot64.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\stardos.c

"$(INTDIR)\c64\cart\stardos.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\stb.c

"$(INTDIR)\c64\cart\stb.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\superexplode5.c

"$(INTDIR)\c64\cart\superexplode5.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\supergames.c

"$(INTDIR)\c64\cart\supergames.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\supersnapshot.c

"$(INTDIR)\c64\cart\supersnapshot.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\supersnapshot4.c

"$(INTDIR)\c64\cart\supersnapshot4.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\warpspeed.c

"$(INTDIR)\c64\cart\warpspeed.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\westermann.c

"$(INTDIR)\c64\cart\westermann.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\c64\cart\zaxxon.c

"$(INTDIR)\c64\cart\zaxxon.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


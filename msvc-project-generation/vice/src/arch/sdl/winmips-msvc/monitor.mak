# Microsoft Developer Studio Generated NMAKE File, Based on monitor.dsp
!IF "$(CFG)" == ""
CFG=monitor - Win32 Release
!MESSAGE No configuration specified. Defaulting to monitor - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "monitor - Win32 Release" && "$(CFG)" != "monitor - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "monitor.mak" CFG="monitor - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "monitor - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "monitor - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "monitor - Win32 Release"

OUTDIR=.\libs\monitor\Release
INTDIR=.\libs\monitor\Release
# Begin Custom Macros
OutDir=.\libs\monitor\Release
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\monitor.lib" 

!ELSE 

ALL : "base - Win32 Release" "$(OUTDIR)\monitor.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 ReleaseCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\monitor\asm6502.obj"
	-@erase "$(INTDIR)\monitor\asm6502dtv.obj"
	-@erase "$(INTDIR)\monitor\asm65816.obj"
	-@erase "$(INTDIR)\monitor\asm6809.obj"
	-@erase "$(INTDIR)\monitor\asmR65C02.obj"
	-@erase "$(INTDIR)\monitor\asmz80.obj"
	-@erase "$(INTDIR)\monitor\mon_assemble6502.obj"
	-@erase "$(INTDIR)\monitor\mon_assemble65816.obj"
	-@erase "$(INTDIR)\monitor\mon_assemble6809.obj"
	-@erase "$(INTDIR)\monitor\mon_assembleR65C02.obj"
	-@erase "$(INTDIR)\monitor\mon_assemblez80.obj"
	-@erase "$(INTDIR)\monitor\mon_breakpoint.obj"
	-@erase "$(INTDIR)\monitor\mon_command.obj"
	-@erase "$(INTDIR)\monitor\mon_disassemble.obj"
	-@erase "$(INTDIR)\monitor\mon_drive.obj"
	-@erase "$(INTDIR)\monitor\mon_file.obj"
	-@erase "$(INTDIR)\monitor\mon_lex.obj"
	-@erase "$(INTDIR)\monitor\mon_memory.obj"
	-@erase "$(INTDIR)\monitor\mon_parse.obj"
	-@erase "$(INTDIR)\monitor\mon_register6502.obj"
	-@erase "$(INTDIR)\monitor\mon_register6502dtv.obj"
	-@erase "$(INTDIR)\monitor\mon_register65816.obj"
	-@erase "$(INTDIR)\monitor\mon_registerR65C02.obj"
	-@erase "$(INTDIR)\monitor\mon_register6809.obj"
	-@erase "$(INTDIR)\monitor\mon_registerz80.obj"
	-@erase "$(INTDIR)\monitor\mon_ui.obj"
	-@erase "$(INTDIR)\monitor\mon_util.obj"
	-@erase "$(INTDIR)\monitor\monitor.obj"
	-@erase "$(INTDIR)\monitor\monitor_network.obj"
	-@erase "$(OUTDIR)\monitor.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\imagecontents "/I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "NDEBUG" /Fp"$(INTDIR)\monitor.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\monitor.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\monitor.lib" 
LIB32_OBJS= \
	"$(INTDIR)\monitor\asm6502.obj" \
	"$(INTDIR)\monitor\asm6502dtv.obj" \
	"$(INTDIR)\monitor\asm65816.obj" \
	"$(INTDIR)\monitor\asm6809.obj" \
	"$(INTDIR)\monitor\asmR65C02.obj" \
	"$(INTDIR)\monitor\asmz80.obj" \
	"$(INTDIR)\monitor\mon_assemble6502.obj" \
	"$(INTDIR)\monitor\mon_assemble65816.obj" \
	"$(INTDIR)\monitor\mon_assemble6809.obj" \
	"$(INTDIR)\monitor\mon_assembleR65C02.obj" \
	"$(INTDIR)\monitor\mon_assemblez80.obj" \
	"$(INTDIR)\monitor\mon_breakpoint.obj" \
	"$(INTDIR)\monitor\mon_command.obj" \
	"$(INTDIR)\monitor\mon_disassemble.obj" \
	"$(INTDIR)\monitor\mon_drive.obj" \
	"$(INTDIR)\monitor\mon_file.obj" \
	"$(INTDIR)\monitor\mon_lex.obj" \
	"$(INTDIR)\monitor\mon_memory.obj" \
	"$(INTDIR)\monitor\mon_parse.obj" \
	"$(INTDIR)\monitor\mon_register6502.obj" \
	"$(INTDIR)\monitor\mon_register6502dtv.obj" \
	"$(INTDIR)\monitor\mon_register65816.obj" \
	"$(INTDIR)\monitor\mon_registerR65C02.obj" \
	"$(INTDIR)\monitor\mon_register6809.obj" \
	"$(INTDIR)\monitor\mon_registerz80.obj" \
	"$(INTDIR)\monitor\mon_ui.obj" \
	"$(INTDIR)\monitor\mon_util.obj" \
	"$(INTDIR)\monitor\monitor.obj" \
	"$(INTDIR)\monitor\monitor_network.obj" \
	".\libsbase\Release\base.lib" \


"$(OUTDIR)\Release.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ELSEIF  "$(CFG)" == "Release - Win32 Debug"

OUTDIR=.\libs\monitor\Debug
INTDIR=.\libs\monitor\Debug
# Begin Custom Macros
OutDir=.\libs\monitor\Debug
# End Custom Macros

!IF "$(RECURSE)" == "0"

ALL : "$(OUTDIR)\monitor.lib" 

!ELSE 

ALL : "base - Win32 Debug" "$(OUTDIR)\monitor.lib" 

!ENDIF 

!IF "$(RECURSE)" == "1"
CLEAN :"base - Win32 DebugCLEAN" 
!ELSE 
CLEAN :
!ENDIF 
	-@erase "$(INTDIR)\monitor\asm6502.obj"
	-@erase "$(INTDIR)\monitor\asm6502dtv.obj"
	-@erase "$(INTDIR)\monitor\asm65816.obj"
	-@erase "$(INTDIR)\monitor\asm6809.obj"
	-@erase "$(INTDIR)\monitor\asmR65C02.obj"
	-@erase "$(INTDIR)\monitor\asmz80.obj"
	-@erase "$(INTDIR)\monitor\mon_assemble6502.obj"
	-@erase "$(INTDIR)\monitor\mon_assemble65816.obj"
	-@erase "$(INTDIR)\monitor\mon_assemble6809.obj"
	-@erase "$(INTDIR)\monitor\mon_assembleR65C02.obj"
	-@erase "$(INTDIR)\monitor\mon_assemblez80.obj"
	-@erase "$(INTDIR)\monitor\mon_breakpoint.obj"
	-@erase "$(INTDIR)\monitor\mon_command.obj"
	-@erase "$(INTDIR)\monitor\mon_disassemble.obj"
	-@erase "$(INTDIR)\monitor\mon_drive.obj"
	-@erase "$(INTDIR)\monitor\mon_file.obj"
	-@erase "$(INTDIR)\monitor\mon_lex.obj"
	-@erase "$(INTDIR)\monitor\mon_memory.obj"
	-@erase "$(INTDIR)\monitor\mon_parse.obj"
	-@erase "$(INTDIR)\monitor\mon_register6502.obj"
	-@erase "$(INTDIR)\monitor\mon_register6502dtv.obj"
	-@erase "$(INTDIR)\monitor\mon_register65816.obj"
	-@erase "$(INTDIR)\monitor\mon_registerR65C02.obj"
	-@erase "$(INTDIR)\monitor\mon_register6809.obj"
	-@erase "$(INTDIR)\monitor\mon_registerz80.obj"
	-@erase "$(INTDIR)\monitor\mon_ui.obj"
	-@erase "$(INTDIR)\monitor\mon_util.obj"
	-@erase "$(INTDIR)\monitor\monitor.obj"
	-@erase "$(INTDIR)\monitor\monitor_network.obj"
	-@erase "$(OUTDIR)\monitor.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /GX /Z7 /Od /I ".\\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive "/I "..\..\..\imagecontents "/I "..\..\..\lib\p64 "/I "..\..\..\monitor "/I "..\..\..\vdrive "/D "WIN32" /D "WINMIPS" /D "IDE_COMPILE" /D "_WINDOWS" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /Fp"$(INTDIR)\monitor.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\"  /c 

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
BSC32_FLAGS=/nologo /o"$(OUTDIR)\monitor.bsc" 
BSC32_SBRS= \

LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\monitor.lib" 
LIB32_OBJS= \
	"$(INTDIR)\monitor\asm6502.obj" \
	"$(INTDIR)\monitor\asm6502dtv.obj" \
	"$(INTDIR)\monitor\asm65816.obj" \
	"$(INTDIR)\monitor\asm6809.obj" \
	"$(INTDIR)\monitor\asmR65C02.obj" \
	"$(INTDIR)\monitor\asmz80.obj" \
	"$(INTDIR)\monitor\mon_assemble6502.obj" \
	"$(INTDIR)\monitor\mon_assemble65816.obj" \
	"$(INTDIR)\monitor\mon_assemble6809.obj" \
	"$(INTDIR)\monitor\mon_assembleR65C02.obj" \
	"$(INTDIR)\monitor\mon_assemblez80.obj" \
	"$(INTDIR)\monitor\mon_breakpoint.obj" \
	"$(INTDIR)\monitor\mon_command.obj" \
	"$(INTDIR)\monitor\mon_disassemble.obj" \
	"$(INTDIR)\monitor\mon_drive.obj" \
	"$(INTDIR)\monitor\mon_file.obj" \
	"$(INTDIR)\monitor\mon_lex.obj" \
	"$(INTDIR)\monitor\mon_memory.obj" \
	"$(INTDIR)\monitor\mon_parse.obj" \
	"$(INTDIR)\monitor\mon_register6502.obj" \
	"$(INTDIR)\monitor\mon_register6502dtv.obj" \
	"$(INTDIR)\monitor\mon_register65816.obj" \
	"$(INTDIR)\monitor\mon_registerR65C02.obj" \
	"$(INTDIR)\monitor\mon_register6809.obj" \
	"$(INTDIR)\monitor\mon_registerz80.obj" \
	"$(INTDIR)\monitor\mon_ui.obj" \
	"$(INTDIR)\monitor\mon_util.obj" \
	"$(INTDIR)\monitor\monitor.obj" \
	"$(INTDIR)\monitor\monitor_network.obj" \
	".\libsbase\Debug\base.lib" \


"$(OUTDIR)\Debug.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF 


!IF "$(CFG)" == "monitor - Win32 Release" || "$(CFG)" == "monitor - Win32 Debug"

!IF  "$(CFG)" == "monitor - Win32 Release"

"base - Win32 Release" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" 
   cd "."

"base - Win32 ReleaseCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Release" RECURSE=1 CLEAN 
   cd "."

!ELSEIF  "$(CFG)" == "monitor - Win32 Debug"

"base - Win32 Debug" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" 
   cd "."

"base - Win32 DebugCLEAN" : 
   cd "."
   $(MAKE) /$(MAKEFLAGS) /F ".\base.mak" CFG="base - Win32 Debug" RECURSE=1 CLEAN 
   cd "."

!ENDIF 

SOURCE=..\..\..\monitor\asm6502.c

"$(INTDIR)\monitor\asm6502.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\asm6502dtv.c

"$(INTDIR)\monitor\asm6502dtv.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\asm65816.c

"$(INTDIR)\monitor\asm65816.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\asm6809.c

"$(INTDIR)\monitor\asm6809.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\asmR65C02.c

"$(INTDIR)\monitor\asmR65C02.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\asmz80.c

"$(INTDIR)\monitor\asmz80.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_assemble6502.c

"$(INTDIR)\monitor\mon_assemble6502.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_assemble65816.c

"$(INTDIR)\monitor\mon_assemble65816.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_assemble6809.c

"$(INTDIR)\monitor\mon_assemble6809.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_assembleR65C02.c

"$(INTDIR)\monitor\mon_assembleR65C02.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_assemblez80.c

"$(INTDIR)\monitor\mon_assemblez80.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_breakpoint.c

"$(INTDIR)\monitor\mon_breakpoint.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_command.c

"$(INTDIR)\monitor\mon_command.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_disassemble.c

"$(INTDIR)\monitor\mon_disassemble.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_drive.c

"$(INTDIR)\monitor\mon_drive.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_file.c

"$(INTDIR)\monitor\mon_file.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_lex.c

"$(INTDIR)\monitor\mon_lex.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_memory.c

"$(INTDIR)\monitor\mon_memory.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_parse.c

"$(INTDIR)\monitor\mon_parse.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_register6502.c

"$(INTDIR)\monitor\mon_register6502.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_register6502dtv.c

"$(INTDIR)\monitor\mon_register6502dtv.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_register65816.c

"$(INTDIR)\monitor\mon_register65816.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_registerR65C02.c

"$(INTDIR)\monitor\mon_registerR65C02.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_register6809.c

"$(INTDIR)\monitor\mon_register6809.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_registerz80.c

"$(INTDIR)\monitor\mon_registerz80.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_ui.c

"$(INTDIR)\monitor\mon_ui.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\mon_util.c

"$(INTDIR)\monitor\mon_util.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\monitor.c

"$(INTDIR)\monitor\monitor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)

SOURCE=..\..\..\monitor\monitor_network.c

"$(INTDIR)\monitor\monitor_network.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF 


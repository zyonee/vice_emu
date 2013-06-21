# Microsoft Developer Studio Project File - Name="x64dtv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=x64dtv - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "x64dtv.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "x64dtv.mak" CFG="x64dtv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "x64dtv - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "x64dtv - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "x64dtv - Win32 DX Release" (based on "Win32 (x86) Application")
!MESSAGE "x64dtv - Win32 DX Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "x64dtv - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\data"
# PROP Intermediate_Dir "libs\x64dtv\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "NODIRECTX" /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\msvc" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\platform" /I "..\..\..\vdrive"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "NODIRECTX" /D "NDEBUG" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE" /d "NODIRECTX"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE" /d "NODIRECTX"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 advapi32.lib comctl32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winmm.lib winspool.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 advapi32.lib comctl32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winmm.lib winspool.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "x64dtv - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\data"
# PROP Intermediate_Dir "libs\x64dtv\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "NODIRECTX" /D "_DEBUG" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "..\msvc" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\platform" /I "..\..\..\vdrive"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "NODIRECTX" /D "_DEBUG" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE" /d "NODIRECTX"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE" /d "NODIRECTX"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 advapi32.lib comctl32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winmm.lib winspool.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 advapi32.lib comctl32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winmm.lib winspool.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "x64dtv - Win32 DX Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DXRelease"
# PROP BASE Intermediate_Dir "DXRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\data"
# PROP Intermediate_Dir "libs\x64dtv\DXRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\msvc" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\platform" /I "..\..\..\vdrive"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 advapi32.lib comctl32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winmm.lib winspool.lib wsock32.lib dsound.lib dxguid.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 advapi32.lib comctl32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winmm.lib winspool.lib wsock32.lib dsound.lib dxguid.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ELSEIF  "$(CFG)" == "x64dtv - Win32 DX Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DXDebug"
# PROP BASE Intermediate_Dir "DXDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\data"
# PROP Intermediate_Dir "libs\x64dtv\DXDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "..\msvc" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive" /I "..\..\..\lib\p64" /I "..\..\..\monitor" /I "..\..\..\platform" /I "..\..\..\vdrive"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /YX /FD /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /o "NUL" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 advapi32.lib comctl32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winmm.lib winspool.lib wsock32.lib dsound.lib dxguid.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 advapi32.lib comctl32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winmm.lib winspool.lib wsock32.lib dsound.lib dxguid.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF

# Begin Target

# Name "x64dtv - Win32 Release"
# Name "x64dtv - Win32 Debug"
# Name "x64dtv - Win32 DX Release"
# Name "x64dtv - Win32 DX Debug"
# Begin Source File

SOURCE="..\..\..\main.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\ps2mouse.c"
# End Source File
# Begin Source File

SOURCE="..\resc64dtv.rc"

!IF  "$(CFG)" == "x64dtv - Win32 Release"

# PROP Ignore_Default_Tool 1
USERDEP__RESC6="..\..\..\debug.h"	"..\resc64dtv.rc"	"..\res.rc"	"..\resdrivec64.rc"	"..\ressiddtv.rc"	"..\resautostart.rc"	"..\resvicii.rc"
# Begin Custom Build
InputPath="..\resc64dtv.rc"

"resc64dtvcat.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy /b ..\resc64dtv.rc + ..\res.rc + ..\resdrivec64.rc + ..\ressiddtv.rc + ..\resautostart.rc + ..\resvicii.rc resc64dtvcat.rc /b

# End Custom Build

!ELSEIF  "$(CFG)" == "x64dtv - Win32 Debug"

# PROP Ignore_Default_Tool 1
USERDEP__RESC6="..\..\..\debug.h"	"..\resc64dtv.rc"	"..\res.rc"	"..\resdrivec64.rc"	"..\ressiddtv.rc"	"..\resautostart.rc"	"..\resvicii.rc"
# Begin Custom Build
InputPath="..\resc64dtv.rc"

"resc64dtvcat.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy /b ..\resc64dtv.rc + ..\res.rc + ..\resdrivec64.rc + ..\ressiddtv.rc + ..\resautostart.rc + ..\resvicii.rc resc64dtvcat.rc /b

# End Custom Build

!ELSEIF  "$(CFG)" == "x64dtv - Win32 DX Release"

# PROP Ignore_Default_Tool 1
USERDEP__RESC6="..\..\..\debug.h"	"..\resc64dtv.rc"	"..\res.rc"	"..\resdrivec64.rc"	"..\ressiddtv.rc"	"..\resautostart.rc"	"..\resvicii.rc"
# Begin Custom Build
InputPath="..\resc64dtv.rc"

"resc64dtvcat.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy /b ..\resc64dtv.rc + ..\res.rc + ..\resdrivec64.rc + ..\ressiddtv.rc + ..\resautostart.rc + ..\resvicii.rc resc64dtvcat.rc /b

# End Custom Build

!ELSEIF  "$(CFG)" == "x64dtv - Win32 DX Debug"

# PROP Ignore_Default_Tool 1
USERDEP__RESC6="..\..\..\debug.h"	"..\resc64dtv.rc"	"..\res.rc"	"..\resdrivec64.rc"	"..\ressiddtv.rc"	"..\resautostart.rc"	"..\resvicii.rc"
# Begin Custom Build
InputPath="..\resc64dtv.rc"

"resc64dtvcat.rc" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	copy /b ..\resc64dtv.rc + ..\res.rc + ..\resdrivec64.rc + ..\ressiddtv.rc + ..\resautostart.rc + ..\resvicii.rc resc64dtvcat.rc /b

# End Custom Build

!ENDIF

# End Source File
# Begin Source File

SOURCE=".\resc64dtvcat.rc"
# End Source File
# Begin Source File

SOURCE="..\vice.manifest"
# End Source File
# End Target
# End Project

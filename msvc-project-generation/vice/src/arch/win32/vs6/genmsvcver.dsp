# Microsoft Developer Studio Project File - Name="genmsvcver" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=genmsvcver - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "genmsvcver.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "genmsvcver.mak" CFG="genmsvcver - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "genmsvcver - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "genmsvcver - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "genmsvcver - Win32 DX Release" (based on "Win32 (x86) Console Application")
!MESSAGE "genmsvcver - Win32 DX Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "genmsvcver - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\data"
# PROP Intermediate_Dir "libs\genmsvcver\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2  /D "WIN32" /D "_CONSOLE" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_MBCS" /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\msvc" /I "..\\" /I "..\..\..\\"  /D "WIN32" /D "_CONSOLE" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_MBCS" /D "NDEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE" /d "NODIRECTX"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE" /d "NODIRECTX"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 advapi32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winspool.lib wsock32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 advapi32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winspool.lib wsock32.lib /nologo /subsystem:console /machine:I386

# Begin Custom Build - Generating msvc_ver.h
InputPath="..\..\..\..\data\genmsvcver.exe"
SOURCE="$(InputPath)"

"..\msvc\msvc_ver.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\..\..\data\genmsvcver ..\msvc\msvc_ver.h

# End Custom Build

!ELSEIF  "$(CFG)" == "genmsvcver - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\data"
# PROP Intermediate_Dir "libs\genmsvcver\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od  /D "WIN32" /D "_CONSOLE" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H "/D "_MBCS" /D "_DEBUG" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /I "..\msvc" /I "..\\" /I "..\..\..\\"  /D "WIN32" /D "_CONSOLE" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H "/D "_MBCS" /D "_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE" /d "NODIRECTX"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE" /d "NODIRECTX"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 advapi32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winspool.lib wsock32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 advapi32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winspool.lib wsock32.lib /nologo /subsystem:console /machine:I386

# Begin Custom Build - Generating msvc_ver.h
InputPath="..\..\..\..\data\genmsvcver.exe"
SOURCE="$(InputPath)"

"..\msvc\msvc_ver.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\..\..\data\genmsvcver ..\msvc\msvc_ver.h

# End Custom Build

!ELSEIF  "$(CFG)" == "genmsvcver - Win32 DX Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "DXRelease"
# PROP BASE Intermediate_Dir "DXRelease"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\data"
# PROP Intermediate_Dir "libs\genmsvcver\DXRelease"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2  /D "WIN32" /D "_CONSOLE" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_MBCS" /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "..\msvc" /I "..\\" /I "..\..\..\\"  /D "WIN32" /D "_CONSOLE" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_MBCS" /D "NDEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 advapi32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winspool.lib wsock32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 advapi32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winspool.lib wsock32.lib /nologo /subsystem:console /machine:I386

# Begin Custom Build - Generating msvc_ver.h
InputPath="..\..\..\..\data\genmsvcver.exe"
SOURCE="$(InputPath)"

"..\msvc\msvc_ver.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\..\..\data\genmsvcver ..\msvc\msvc_ver.h

# End Custom Build

!ELSEIF  "$(CFG)" == "genmsvcver - Win32 DX Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DXDebug"
# PROP BASE Intermediate_Dir "DXDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\data"
# PROP Intermediate_Dir "libs\genmsvcver\DXDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /GX /Z7 /Od  /D "WIN32" /D "_CONSOLE" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_MBCS" /D "_DEBUG" /YX /FD /c
# ADD CPP /nologo /MTd /W3 /GX /Z7 /Od /I "..\msvc" /I "..\\" /I "..\..\..\\"  /D "WIN32" /D "_CONSOLE" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_MBCS" /D "_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 advapi32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winspool.lib wsock32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 advapi32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winspool.lib wsock32.lib /nologo /subsystem:console /machine:I386

# Begin Custom Build - Generating msvc_ver.h
InputPath="..\..\..\..\data\genmsvcver.exe"
SOURCE="$(InputPath)"

"..\msvc\msvc_ver.h" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	..\..\..\..\data\genmsvcver ..\msvc\msvc_ver.h

# End Custom Build

!ENDIF

# Begin Target

# Name "genmsvcver - Win32 Release"
# Name "genmsvcver - Win32 Debug"
# Name "genmsvcver - Win32 DX Release"
# Name "genmsvcver - Win32 DX Debug"
# Begin Source File

SOURCE="..\..\..\arch\win32\utils\genmsvcver.c"
# End Source File
# End Target
# End Project

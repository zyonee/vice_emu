# Microsoft Developer Studio Project File - Name="petcat" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=petcat - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "petcat.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "petcat.mak" CFG="petcat - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "petcat - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "petcat - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "petcat - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "..\..\..\..\data"
# PROP Intermediate_Dir "libs\petcat\Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2  /D "WIN32" /D "_CONSOLE" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_MBCS" /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive" /I "..\..\..\vdrive"  /D "WIN32" /D "_CONSOLE" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_MBCS" /D "NDEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 advapi32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winspool.lib wsock32.lib  SDLmain.lib SDL.lib opengl32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 advapi32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winspool.lib wsock32.lib  SDLmain.lib SDL.lib opengl32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "petcat - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "..\..\..\..\data"
# PROP Intermediate_Dir "libs\petcat\Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /Z7 /Od  /D "WIN32" /D "_CONSOLE" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H "/D "_MBCS" /D "_DEBUG" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I ".\" /I "..\\" /I "..\..\..\\" /I "..\..\..\drive" /I "..\..\..\vdrive"  /D "WIN32" /D "_CONSOLE" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H "/D "_MBCS" /D "_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 advapi32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winspool.lib wsock32.lib  SDLmain.lib SDL.lib opengl32.lib /nologo /subsystem:console /machine:I386 /NODEFAULTLIB:msvcrt.lib
# ADD LINK32 advapi32.lib comdlg32.lib gdi32.lib kernel32.lib odbc32.lib odbccp32.lib ole32.lib oleaut32.lib shell32.lib user32.lib uuid.lib version.lib winspool.lib wsock32.lib  SDLmain.lib SDL.lib opengl32.lib /nologo /subsystem:console /machine:I386 /NODEFAULTLIB:msvcrt.lib

!ENDIF

# Begin Target

# Name "petcat - Win32 Release"
# Name "petcat - Win32 Debug"
# Begin Source File

SOURCE="..\..\..\charset.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\findpath.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\ioutil.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\lib.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\log.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\petcat.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\rawfile.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\resources.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\util.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\zipcode.c"
# End Source File
# End Target
# End Project

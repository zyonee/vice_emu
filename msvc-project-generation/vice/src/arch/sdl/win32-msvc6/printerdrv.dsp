# Microsoft Developer Studio Project File - Name="printerdrv" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=printerdrv - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "printerdrv.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "printerdrv.mak" CFG="printerdrv - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "printerdrv - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "printerdrv - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "printerdrv - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "libs\printerdrv\Release"
# PROP Intermediate_Dir "libs\printerdrv\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\" /I "..\\" /I "..\..\..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "printerdrv - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "libs\printerdrv\Debug"
# PROP Intermediate_Dir "libs\printerdrv\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /Z7 /Od  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I ".\" /I "..\\" /I "..\..\..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "_DEBUG" /d "WIN32" /d "IDE_COMPILE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF

# Begin Target

# Name "printerdrv - Win32 Release"
# Name "printerdrv - Win32 Debug"
# Begin Source File

SOURCE="..\..\..\printerdrv\driver-select.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\printerdrv\drv-ascii.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\printerdrv\drv-mps803.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\printerdrv\drv-nl10.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\printerdrv\drv-raw.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\printerdrv\interface-serial.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\printerdrv\interface-userport.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\printerdrv\output-graphics.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\printerdrv\output-select.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\printerdrv\output-text.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\printerdrv\printer-serial.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\printerdrv\printer-userport.c"
# End Source File
# Begin Source File

SOURCE="..\..\..\printerdrv\printer.c"
# End Source File
# End Target
# End Project

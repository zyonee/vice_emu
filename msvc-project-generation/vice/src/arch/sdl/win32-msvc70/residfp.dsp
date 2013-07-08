# Microsoft Developer Studio Project File - Name="residfp" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=residfp - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "residfp.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "residfp.mak" CFG="residfp - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "residfp - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "residfp - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "residfp - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "libs\residfp\Release"
# PROP Intermediate_Dir "libs\residfp\Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /D PACKAGE=\"residfp\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I ".\" /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /D PACKAGE=\"residfp\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE"
# ADD RSC /l 0x409 /i "..\msvc" /i "..\\" /i "..\..\..\\" /d "NDEBUG" /d "WIN32" /d "IDE_COMPILE"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "residfp - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "libs\residfp\Debug"
# PROP Intermediate_Dir "libs\residfp\Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /GX /Z7 /Od  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /D PACKAGE=\"residfp\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /YX /FD /c
# ADD CPP /nologo /MDd /W3 /GX /Z7 /Od /I ".\" /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /D PACKAGE=\"residfp\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /YX /FD /c
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

# Name "residfp - Win32 Release"
# Name "residfp - Win32 Debug"
# Begin Source File

SOURCE="..\..\..\resid-fp\convolve-sse.cc"

!IF  "$(CFG)" == "residfp - Win32 Release"

# Begin Custom Build
InputPath="..\..\..\resid-fp\convolve-sse.cc"
InputName=convolve-sse

"libs\residfp\Release\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MD /W3 /GX /O2 /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Release\residfp.pch" /Fo"libs\residfp\Release\\" /Fd"libs\residfp\Release\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "residfp - Win32 Debug"

# Begin Custom Build
InputPath="..\..\..\resid-fp\convolve-sse.cc"
InputName=convolve-sse

"libs\residfp\Debug\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MDd /W3 /GX /Z7 /Od /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Debug\residfp.pch" /Fo"libs\residfp\Debug\\" /Fd"libs\residfp\Debug\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ENDIF

# End Source File
# Begin Source File

SOURCE="..\..\..\resid-fp\convolve.cc"

!IF  "$(CFG)" == "residfp - Win32 Release"

# Begin Custom Build
InputPath="..\..\..\resid-fp\convolve.cc"
InputName=convolve

"libs\residfp\Release\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MD /W3 /GX /O2 /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Release\residfp.pch" /Fo"libs\residfp\Release\\" /Fd"libs\residfp\Release\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "residfp - Win32 Debug"

# Begin Custom Build
InputPath="..\..\..\resid-fp\convolve.cc"
InputName=convolve

"libs\residfp\Debug\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MDd /W3 /GX /Z7 /Od /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Debug\residfp.pch" /Fo"libs\residfp\Debug\\" /Fd"libs\residfp\Debug\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ENDIF

# End Source File
# Begin Source File

SOURCE="..\..\..\resid-fp\envelope.cc"

!IF  "$(CFG)" == "residfp - Win32 Release"

# Begin Custom Build
InputPath="..\..\..\resid-fp\envelope.cc"
InputName=envelope

"libs\residfp\Release\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MD /W3 /GX /O2 /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Release\residfp.pch" /Fo"libs\residfp\Release\\" /Fd"libs\residfp\Release\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "residfp - Win32 Debug"

# Begin Custom Build
InputPath="..\..\..\resid-fp\envelope.cc"
InputName=envelope

"libs\residfp\Debug\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MDd /W3 /GX /Z7 /Od /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Debug\residfp.pch" /Fo"libs\residfp\Debug\\" /Fd"libs\residfp\Debug\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ENDIF

# End Source File
# Begin Source File

SOURCE="..\..\..\resid-fp\extfilt.cc"

!IF  "$(CFG)" == "residfp - Win32 Release"

# Begin Custom Build
InputPath="..\..\..\resid-fp\extfilt.cc"
InputName=extfilt

"libs\residfp\Release\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MD /W3 /GX /O2 /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Release\residfp.pch" /Fo"libs\residfp\Release\\" /Fd"libs\residfp\Release\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "residfp - Win32 Debug"

# Begin Custom Build
InputPath="..\..\..\resid-fp\extfilt.cc"
InputName=extfilt

"libs\residfp\Debug\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MDd /W3 /GX /Z7 /Od /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Debug\residfp.pch" /Fo"libs\residfp\Debug\\" /Fd"libs\residfp\Debug\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ENDIF

# End Source File
# Begin Source File

SOURCE="..\..\..\resid-fp\filter.cc"

!IF  "$(CFG)" == "residfp - Win32 Release"

# Begin Custom Build
InputPath="..\..\..\resid-fp\filter.cc"
InputName=filter

"libs\residfp\Release\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MD /W3 /GX /O2 /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Release\residfp.pch" /Fo"libs\residfp\Release\\" /Fd"libs\residfp\Release\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "residfp - Win32 Debug"

# Begin Custom Build
InputPath="..\..\..\resid-fp\filter.cc"
InputName=filter

"libs\residfp\Debug\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MDd /W3 /GX /Z7 /Od /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Debug\residfp.pch" /Fo"libs\residfp\Debug\\" /Fd"libs\residfp\Debug\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ENDIF

# End Source File
# Begin Source File

SOURCE="..\..\..\resid-fp\pot.cc"

!IF  "$(CFG)" == "residfp - Win32 Release"

# Begin Custom Build
InputPath="..\..\..\resid-fp\pot.cc"
InputName=pot

"libs\residfp\Release\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MD /W3 /GX /O2 /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Release\residfp.pch" /Fo"libs\residfp\Release\\" /Fd"libs\residfp\Release\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "residfp - Win32 Debug"

# Begin Custom Build
InputPath="..\..\..\resid-fp\pot.cc"
InputName=pot

"libs\residfp\Debug\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MDd /W3 /GX /Z7 /Od /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Debug\residfp.pch" /Fo"libs\residfp\Debug\\" /Fd"libs\residfp\Debug\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ENDIF

# End Source File
# Begin Source File

SOURCE="..\..\..\resid-fp\sid.cc"

!IF  "$(CFG)" == "residfp - Win32 Release"

# Begin Custom Build
InputPath="..\..\..\resid-fp\sid.cc"
InputName=sid

"libs\residfp\Release\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MD /W3 /GX /O2 /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Release\residfp.pch" /Fo"libs\residfp\Release\\" /Fd"libs\residfp\Release\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "residfp - Win32 Debug"

# Begin Custom Build
InputPath="..\..\..\resid-fp\sid.cc"
InputName=sid

"libs\residfp\Debug\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MDd /W3 /GX /Z7 /Od /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Debug\residfp.pch" /Fo"libs\residfp\Debug\\" /Fd"libs\residfp\Debug\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ENDIF

# End Source File
# Begin Source File

SOURCE="..\..\..\resid-fp\version.cc"

!IF  "$(CFG)" == "residfp - Win32 Release"

# Begin Custom Build
InputPath="..\..\..\resid-fp\version.cc"
InputName=version

"libs\residfp\Release\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MD /W3 /GX /O2 /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Release\residfp.pch" /Fo"libs\residfp\Release\\" /Fd"libs\residfp\Release\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "residfp - Win32 Debug"

# Begin Custom Build
InputPath="..\..\..\resid-fp\version.cc"
InputName=version

"libs\residfp\Debug\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MDd /W3 /GX /Z7 /Od /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Debug\residfp.pch" /Fo"libs\residfp\Debug\\" /Fd"libs\residfp\Debug\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ENDIF

# End Source File
# Begin Source File

SOURCE="..\..\..\resid-fp\voice.cc"

!IF  "$(CFG)" == "residfp - Win32 Release"

# Begin Custom Build
InputPath="..\..\..\resid-fp\voice.cc"
InputName=voice

"libs\residfp\Release\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MD /W3 /GX /O2 /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Release\residfp.pch" /Fo"libs\residfp\Release\\" /Fd"libs\residfp\Release\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "residfp - Win32 Debug"

# Begin Custom Build
InputPath="..\..\..\resid-fp\voice.cc"
InputName=voice

"libs\residfp\Debug\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MDd /W3 /GX /Z7 /Od /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Debug\residfp.pch" /Fo"libs\residfp\Debug\\" /Fd"libs\residfp\Debug\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ENDIF

# End Source File
# Begin Source File

SOURCE="..\..\..\resid-fp\wave.cc"

!IF  "$(CFG)" == "residfp - Win32 Release"

# Begin Custom Build
InputPath="..\..\..\resid-fp\wave.cc"
InputName=wave

"libs\residfp\Release\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MD /W3 /GX /O2 /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H"  /D "NDEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Release\residfp.pch" /Fo"libs\residfp\Release\\" /Fd"libs\residfp\Release\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ELSEIF  "$(CFG)" == "residfp - Win32 Debug"

# Begin Custom Build
InputPath="..\..\..\resid-fp\wave.cc"
InputName=wave

"libs\residfp\Debug\$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	cl /nologo /MDd /W3 /GX /Z7 /Od /EHsc /I ".\\" /I "..\\"  /D "WIN32" /D "_WINDOWS" /D "IDE_COMPILE" /D "DONT_USE_UNISTD_H" /D "_DEBUG" /D PACKAGE=\"%s\" /D VERSION=\"0.7\" /D SIZEOF_INT=4 /Fp"libs\residfp\Debug\residfp.pch" /Fo"libs\residfp\Debug\\" /Fd"libs\residfp\Debug\\" /FD /TP /c "$(InputPath)"

# End Custom Build

!ENDIF

# End Source File
# End Target
# End Project

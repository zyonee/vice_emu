@echo off

rem genwt.bat - generate files for the OS/2 SDL port on windows
rem
rem Written by
rem  Marco van den Heuvel <blackystardust68@yahoo.com>
rem
rem This file is part of VICE, the Versatile Commodore Emulator.
rem See README for copyright notice.
rem
rem  This program is free software; you can redistribute it and/or modify
rem  it under the terms of the GNU General Public License as published by
rem  the Free Software Foundation; either version 2 of the License, or
rem  (at your option) any later version.
rem
rem  This program is distributed in the hope that it will be useful,
rem  but WITHOUT ANY WARRANTY; without even the implied warranty of
rem  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
rem  GNU General Public License for more details.
rem
rem  You should have received a copy of the GNU General Public License
rem  along with this program; if not, write to the Free Software
rem  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
rem  02111-1307  USA.

echo "Generating files for os/2 SDL"
copy ..\..\..\translate.txt trans.txt
copy ..\..\..\..\doc\vice.texi vice.tex
copy ..\..\..\buildtools\infocontrib.sed ic.sed
copy ..\..\..\..\AUTHORS authors.txt
copy ..\..\unix\macosx\Resources\Credits.html credits.htm
copy ..\..\..\..\README readme.tx1
copy ..\..\..\..\doc\html\index.html index.htm
copy ..\..\..\..\man\vice.1 vice1.txt
if Windows_NT == %OS% goto winnt
goto use16
:winnt
if defined ProgramFiles(x86) GOTO use32
:use16
genic ..\ ..\ os2-watcom\vice.tex os2-watcom\ic.h os2-watcom\ic.sed os2-watcom\authors.txt os2-watcom\credits.htm os2-watcom\readme.tx1 os2-watcom\index.htm os2-watcom\vice1.txt
gentr trans.txt trans.h transtab.h
goto after
:use32
geninfocontrib32 ..\ ..\ os2-watcom\vice.tex os2-watcom\ic.h os2-watcom\ic.sed os2-watcom\authors.txt os2-watcom\credits.htm os2-watcom\readme.tx1 os2-watcom\index.htm os2-watcom\vice1.txt
gentranslate32 trans.txt trans.h transtab.h
:after
copy ic.h ..\..\..\infocontrib.h 
copy trans.h ..\..\..\translate.h
copy transtab.h ..\..\..\translate_table.h
copy authors.txt ..\..\..\..\AUTHORS
copy credits.htm ..\..\unix\macosx\Resources\Credits.html
copy readme.tx1 ..\..\..\..\README
copy index.htm ..\..\..\..\doc\html\index.html
copy vice1.txt ..\..\..\..\man\vice.1
del ic.h vice.tex ic.sed trans.txt trans.h transtab.h authors.txt credits.htm readme.tx1 index.htm vice1.txt

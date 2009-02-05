#!/bin/sh
# make-bindist_win32.sh for the SDL port
#
# written by Marco van den Heuvel <blackystardust68@yahoo.com>
#
# make-bindist.sh <strip> <vice-version> <zip|nozip> <top-srcdir>
#                 $1      $2             $3          $4

STRIP=$1
VICEVERSION=$2
ZIPKIND=$3
TOPSRCDIR=$4

if [ ! -e src/x64.exe -o ! -e src/x64dtv.exe -o ! -e src/x128.exe -o ! -e src/xvic.exe -o ! -e src/xpet.exe -o ! -e src/xplus4.exe -o ! -e src/xcbm2.exe -o ! -e src/c1541.exe -o ! -e src/petcat.exe -o ! -e src/cartconv.exe ]
then
  echo Error: executable file\(s\) not found, do a \"make\" first
  exit 1
fi

echo Generating win32 SDL port binary distribution.
rm -f -r SDLVICE-$VICEVERSION-win32
mkdir SDLVICE-$VICEVERSION-win32
$STRIP src/x64.exe
$STRIP src/x64dtv.exe
$STRIP src/x128.exe
$STRIP src/xvic.exe
$STRIP src/xpet.exe
$STRIP src/xplus4.exe
$STRIP src/xcbm2.exe
$STRIP src/c1541.exe
$STRIP src/petcat.exe
$STRIP src/cartconv.exe
cp src/x64.exe src/x64dtv.exe src/x128.exe SDLVICE-$VICEVERSION-win32
cp src/xvic.exe src/xpet.exe src/xplus4.exe SDLVICE-$VICEVERSION-win32
cp src/xcbm2.exe src/c1541.exe src/petcat.exe SDLVICE-$VICEVERSION-win32
cp src/cartconv.exe SDLVICE-$VICEVERSION-win32
cp -a $TOPSRCDIR/data/C128 $TOPSRCDIR/data/C64 SDLVICE-$VICEVERSION-win32
cp -a $TOPSRCDIR/data/C64DTV $TOPSRCDIR/data/CBM-II SDLVICE-$VICEVERSION-win32
cp -a $TOPSRCDIR/data/DRIVES $TOPSRCDIR/data/PET SDLVICE-$VICEVERSION-win32
cp -a $TOPSRCDIR/data/PLUS4 $TOPSRCDIR/data/PRINTER SDLVICE-$VICEVERSION-win32
cp -a $TOPSRCDIR/data/VIC20 SDLVICE-$VICEVERSION-win32
cp -a $TOPSRCDIR/data/fonts SDLVICE-$VICEVERSION-win32
cp -a $TOPSRCDIR/doc/html SDLVICE-$VICEVERSION-win32
cp $TOPSRCDIR/FEEDBACK $TOPSRCDIR/README SDLVICE-$VICEVERSION-win32
cp $TOPSRCDIR/doc/cartconv.txt SDLVICE-$VICEVERSION-win32
rm `find SDLVICE-$VICEVERSION-win32 -name "Makefile*"`
rm `find SDLVICE-$VICEVERSION-win32 -name "amiga_*.vkm"`
rm `find SDLVICE-$VICEVERSION-win32 -name "dos_*.vkm"`
rm `find SDLVICE-$VICEVERSION-win32 -name "os2*.vkm"`
rm `find SDLVICE-$VICEVERSION-win32 -name "osx*.vkm"`
rm `find SDLVICE-$VICEVERSION-win32 -name "beos_*.vkm"`
rm `find SDLVICE-$VICEVERSION-win32 -name "x11_*.vkm"`
rm `find SDLVICE-$VICEVERSION-win32 -name "RO*.vkm"`
rm `find SDLVICE-$VICEVERSION-win32 -name "win*.vkm"`
rm SDLVICE-$VICEVERSION-win32/html/texi2html
if test x"$ZIPKIND" = "xzip"; then
  if test x"$ZIP" = "x"; then
    zip -r -9 -q SDLVICE-$VICEVERSION-win32.zip SDLVICE-$VICEVERSION-win32
  else
    $ZIP SDLVICE-$VICEVERSION-win32.zip SDLVICE-$VICEVERSION-win32
  fi
  rm -f -r SDLVICE-$VICEVERSION-win32
  echo win32 SDL port binary distribution archive generated as SDLVICE-$VICEVERSION-win32.zip
else
  echo win32 SDL port binary distribution directory generated as SDLVICE-$VICEVERSION-win32
fi

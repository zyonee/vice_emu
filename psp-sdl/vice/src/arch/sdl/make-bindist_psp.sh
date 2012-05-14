#!/bin/sh
# make-bindist.sh for the PSP SDL port
#
# written by Marco van den Heuvel <blackystardust68@yahoo.com>
#
# make-bindist.sh <strip> <vice-version> <zip|nozip> <top-srcdir>
#                 $1      $2             $3          $4

STRIP=$1
VICEVERSION=$2
ZIPKIND=$3
TOPSRCDIR=$4

EMULATORS="x64 x64dtv x128 xcbm2 xcbm5x0 xpet xplus4 xvic vsid"
CONSOLE_TOOLS="c1541 cartconv petcat"
EXECUTABLES="$EMULATORS $CONSOLE_TOOLS"

for i in $EXECUTABLES
do
  if [ ! -e src/$i ]
  then
    echo Error: executable file\(s\) not found, do a \"make\" first
    exit 1
  fi
done

echo Generating PSP SDL port binary distribution.
rm -f -r SDLVICE-psp-$VICEVERSION
mkdir SDLVICE-psp-$VICEVERSION
for i in $EXECUTABLES
do
  $STRIP src/$i
  cp src/$i SDLVICE-psp-$VICEVERSION
done
mkdir SDLVICE-psp-$VICEVERSION/data
cp -a $TOPSRCDIR/data/C128 $TOPSRCDIR/data/C64 SDLVICE-psp-$VICEVERSION/data
cp -a $TOPSRCDIR/data/C64DTV SDLVICE-psp-$VICEVERSION/data
cp -a $TOPSRCDIR/data/CBM-II SDLVICE-psp-$VICEVERSION/data
cp -a $TOPSRCDIR/data/DRIVES $TOPSRCDIR/data/PET SDLVICE-psp-$VICEVERSION/data
cp -a $TOPSRCDIR/data/PLUS4 $TOPSRCDIR/data/VIC20 SDLVICE-psp-$VICEVERSION/data
cp $TOPSRCDIR/doc/readmes/Readme-SDL.txt SDLVICE-psp-$VICEVERSION
rm `find SDLVICE-psp-$VICEVERSION -name "Makefile*"`
rm `find SDLVICE-psp-$VICEVERSION -name "amiga_*.vkm"`
rm `find SDLVICE-psp-$VICEVERSION -name "beos_*.vkm"`
rm `find SDLVICE-psp-$VICEVERSION -name "dos_*.vkm"`
rm `find SDLVICE-psp-$VICEVERSION -name "os2*.vkm"`
rm `find SDLVICE-psp-$VICEVERSION -name "win_*.v*"`
rm `find SDLVICE-psp-$VICEVERSION -name "*.vsc"`

mksfo 'SDLVICE-$VICEVERSION' vice.sfo
for i in $EMULATORS
do
  pack-pbp SDLVICE-psp-$VICEVERSION/EBOOT.PBP.$i vice.sfo $TOPSRCDIR/src/arch/sdl/psp-files/x64-icon.png NULL NULL NULL NULL SDLVICE-psp-$VICEVERSION/$i NULL
  rm -f -r SDLVICE-psp-$VICEVERSION/$i
done
rm -f -r vice.sfo

if test x"$ZIPKIND" = "xzip"; then
  if test x"$ZIP" = "x"; then
    zip -r -9 -q SDLVICE-psp-$VICEVERSION.zip SDLVICE-psp-$VICEVERSION
  else
    $ZIP SDLVICE-psp-$VICEVERSION.zip SDLVICE-psp-$VICEVERSION
  fi
  rm -f -r SDLVICE-psp-$VICEVERSION
  echo PSP SDL port binary distribution archive generated as SDLVICE-psp-$VICEVERSION.zip
else
  echo PSP SDL port binary distribution directory generated as SDLVICE-psp-$VICEVERSION
fi

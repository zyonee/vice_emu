#!/bin/sh

# see if we are in the top of the tree
if [ ! -f configure.in ]; then
  cd ../..
  if [ ! -f configure.in ]; then
    echo "please run this script from the base of the VICE directory"
    echo "or from the appropriate build directory"
    exit 1
  fi
fi

curdir=`pwd`

SDKPATH=`psp-config --pspsdk-path`
LIBS="-L$SDKPATH/lib -lc -lpspuser" ./configure -v --host=psp --enable-sdlui
make
make bindist

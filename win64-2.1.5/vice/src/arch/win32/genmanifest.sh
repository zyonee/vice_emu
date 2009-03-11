#!/bin/sh
# genmanifest.sh for the WIN32 port
#
# written by Marco van den Heuvel <blackystardust68@yahoo.com>
#
# genmanifest.sh <top-srcdir> <cpu>
#                 $1          $2

TOPSRCDIR=$1
CPU=$2

if test x"$CPU" = "xia64"; then
    cp $TOPSRCDIR/vice.manifest_ia64 vice.manifest
else
    if test x"$CPU" = "xamd64" -o x"$CPU" = "xx86_64"; then
        cp $TOPSRCDIR/vice.manifest_amd64 vice.manifest
    else
        cp $TOPSRCDIR/vice.manifest_x86 vice.manifest
    fi
fi

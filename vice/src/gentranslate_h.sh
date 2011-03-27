#!/bin/sh
#
# gentranstale_h.sh - translate.h generator script
#
# written by Marco van den Heuvel <blackystardust68@yahoo.com>

DEBUGBUILD=0

echo "/*"
echo " * translate.h - Global internationalization routines."
echo " *"
echo " * Autogenerated by gentranslate_h.sh, DO NOT EDIT !!!"
echo " *"
echo " * Written by"
echo " *  Marco van den Heuvel <blackystardust68@yahoo.com>"
echo " *"
echo " * This file is part of VICE, the Versatile Commodore Emulator."
echo " * See README for copyright notice."
echo " *"
echo " *  This program is free software; you can redistribute it and/or modify"
echo " *  it under the terms of the GNU General Public License as published by"
echo " *  the Free Software Foundation; either version 2 of the License, or"
echo " *  (at your option) any later version."
echo " *"
echo " *  This program is distributed in the hope that it will be useful,"
echo " *  but WITHOUT ANY WARRANTY; without even the implied warranty of"
echo " *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
echo " *  GNU General Public License for more details."
echo " *"
echo " *  You should have received a copy of the GNU General Public License"
echo " *  along with this program; if not, write to the Free Software"
echo " *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA"
echo " *  02111-1307  USA."
echo " *"
echo " */"
echo ""
echo "#ifndef VICE_TRANSLATE_H"
echo "#define VICE_TRANSLATE_H"
echo ""
echo "#include \"translate_funcs.h\""
echo ""
echo "#define USE_PARAM_STRING   0"
echo "#define USE_PARAM_ID       1"
echo ""
echo "#define USE_DESCRIPTION_STRING   0"
echo "#define USE_DESCRIPTION_ID       1"
echo ""
echo "#define IDGS_UNUSED IDCLS_UNUSED"
echo ""
echo "#define IDCLS_SPECIFY_SIDCART_ENGINE_MODEL   0xffffff /* special case translation */"
echo "#define IDCLS_SPECIFY_SID_ENGINE_MODEL       0xfffffe /* special case translation */"
echo "#define IDCLS_SPECIFY_SIDDTV_ENGINE_MODEL    0xfffffd /* special case translation */"
echo ""
echo "enum { ID_START_65536=65536,"
echo "IDCLS_UNUSED,"
echo ""

# generating the debug version of translate.h takes
# alot more time, so it only gets done when
# --enable-debug is given.

if test x"$DEBUGBUILD" = "xyes"; then
  count=65538
  while read data
  do
    ok="no"
    case ${data%%_*} in
      ID*)
           echo $data", /* "$count" */"
           count=`expr $count + 1`
           echo $data"_DA, /* "$count" */"
           count=`expr $count + 1`
           echo $data"_DE, /* "$count" */"
           count=`expr $count + 1`
           echo $data"_FR, /* "$count" */"
           count=`expr $count + 1`
           echo $data"_HU, /* "$count" */"
           count=`expr $count + 1`
           echo $data"_IT, /* "$count" */"
           count=`expr $count + 1`
           echo $data"_NL, /* "$count" */"
           count=`expr $count + 1`
           echo $data"_PL, /* "$count" */"
           count=`expr $count + 1`
           echo $data"_RU, /* "$count" */"
           count=`expr $count + 1`
           echo $data"_SV, /* "$count" */"
           count=`expr $count + 1`
           echo $data"_TR, /* "$count" */"
           count=`expr $count + 1`
           ok="yes"
      ;;
    esac
    if test $ok = "no";
    then
      echo "$data"
    fi
  done
else
  while read data
  do
    ok="no"
    case ${data%%_*} in
      ID*)
           echo $data","
           echo $data"_DA,"
           echo $data"_DE,"
           echo $data"_FR,"
           echo $data"_HU,"
           echo $data"_IT,"
           echo $data"_NL,"
           echo $data"_PL,"
           echo $data"_RU,"
           echo $data"_SV,"
           echo $data"_TR,"
           ok="yes"
      ;;
    esac
    if test $ok = "no";
    then
      echo "$data"
    fi
  done
fi

echo "};"
echo "#endif"

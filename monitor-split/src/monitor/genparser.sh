#! /bin/sh

#
# genparser.sh - generate the Flex/Bison based parsers
#
# Written by
#  Thomas Giesel  <skoe@directbox.com>
#
# This file is part of VICE, the Versatile Commodore Emulator.
# See README for copyright notice.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
#  02111-1307  USA.
#

exec_cmd() {
    echo `pwd`: $@
    $@
    ret=$?
    if [ ! $ret = 0 ] ; then
        echo "ERROR: $1 failed"
        exit 1
    fi
}

# taken from LKML
do_version_check() {
    [ "$1" = "$2" ] && return 0

    ver1front=`echo $1 | cut -d "." -f -1`
    ver1back=`echo $1 | cut -d "." -f 2-`
    ver2front=`echo $2 | cut -d "." -f -1`
    ver2back=`echo $2 | cut -d "." -f 2-`

    if [ "$ver1front" != "$1" ] || [ "$ver2front" != "$2" ]; then
        [ "$ver1front" -gt "$ver2front" ] && return 0
        [ "$ver1front" -lt "$ver2front" ] && return 1
        [ "$ver1front" = "$1" ] || [ -z "$ver1back" ] && ver1back=0
        [ "$ver2front" = "$2" ] || [ -z "$ver2back" ] && ver2back=0
        do_version_check "$ver1back" "$ver2back"
        return $?
    else
        [ "$1" -gt "$2" ] && return 0 || return 1
    fi
}

check_min_version() {
    p=$1 # program name
    a=$2 # version string to check
    b=$3 # minimal version
    # return 0 if the version is ok, 1 if it is too old
    if [ x"$a" = x"" ]; then
        echo "Error: $p missing"
        exit 1
    else
        va=`echo "$a" | head -n 1 | sed "s/.* //g"` # last word of first line
        do_version_check "$va" "$b"
        if [ $? -ne 0 ]; then
            echo "Warning: $p is too old, need >= $b"
            echo "DO NOT COMMIT files created by an old version of this tool"
        fi
    fi
}

check_min_version flex  "`flex -V`"  2.5.35
check_min_version bison "`bison -V`" 2.4.1

bison_opts=-dtv

exec_cmd bison ${bison_opts} -o mon_parse.c             mon_parse.y
exec_cmd flex                -o mon_lex.c               mon_lex.l
exec_cmd bison ${bison_opts} -o mon_asm6502_parse.c     mon_asm6502_parse.y
exec_cmd flex                -o mon_asm6502_lex.c       mon_asm6502_lex.l


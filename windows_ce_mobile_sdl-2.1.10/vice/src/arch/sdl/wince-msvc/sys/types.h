/*
 * types.h - sys/types.h replacement for Windows CE.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#ifndef VICE_WINCE_SYS_TYPES_H
#define VICE_WINCE_SYS_TYPES_H

#define BIG_ENDIAN    1234
#define LITTLE_ENDIAN 4321

#ifdef MIPS
#define BYTE_ORDER LITTLE_ENDIAN
#endif

#if UNDER_CE > 201
   typedef unsigned long time_t;
#  define _TIME_T_DEFINED_
#endif

typedef unsigned long dev_t;
typedef unsigned long ino_t;

#ifndef _MODE_T_DEFINED_
   typedef unsigned long mode_t;
#  define _MODE_T_DEFINED_
#endif

typedef unsigned short gid_t;
typedef unsigned short uid_t;
typedef long clock_t;

#ifndef _PTRDIFF_T_DEFINED
   typedef long ptrdiff_t;
#  define _PTRDIFF_T_DEFINED
#endif

typedef long off_t;

typedef unsigned char u_char;
typedef unsigned short u_short;

#ifndef _CADDR_T_DEFINED_
   typedef unsigned char * caddr_t;
#  define _CADDR_T_DEFINED_
#endif

#ifndef _SIZE_T_DEFINED_
   typedef unsigned int size_t;
#  define _SIZE_T_DEFINED_
#endif

typedef unsigned char u_int8_t;

typedef short int16_t;
typedef unsigned short u_int16_t;

typedef int int32_t;
typedef unsigned int u_int32_t;

typedef unsigned long u_long;
typedef unsigned int u_int;

#ifndef _TIME_T_DEFINED_
   typedef unsigned long time_t;
#  define _TIME_T_DEFINED_
#endif

#ifndef _PID_T_DEFINED_
   typedef unsigned long pid_t;
#  define _PID_T_DEFINED_
#endif

#endif

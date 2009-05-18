/*
 * sys/stat.h - sys/stat.h replacement for Windows CE.
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

#ifndef VICE_WINCE_SYS_STAT_H
#define VICE_WINCE_SYS_STAT_H

#include <sys/types.h>

#define _S_IFMT   0170000
#define _S_IFDIR  0040000
#define _S_IFCHR  0020000
#define _S_IFIFO  0010000
#define _S_IFREG  0100000
#define _S_IREAD  0000400
#define _S_IWRITE 0000200
#define _S_IEXEC  0000100

#define S_IFMT   _S_IFMT
#define S_IFREG  _S_IFREG
#define S_IFCHR  _S_IFCHR
#define S_IFDIR  _S_IFDIR
#define S_IREAD  _S_IREAD
#define S_IWRITE _S_IWRITE
#define S_IEXEC  _S_IEXEC

#ifndef S_ISDIR
#define S_ISDIR(X) (((X) & S_IFMT) == S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(X) (((X) & S_IFMT) == S_IFREG)
#endif

struct stat 
{
  dev_t st_dev;
  ino_t st_ino;
  unsigned short st_mode;
  short st_nlink;
  short st_uid;
  short st_gid;
  dev_t st_rdev;
  off_t st_size;
  time_t st_atime;
  time_t st_mtime;
  time_t st_ctime;
};

extern int stat(const char *fname, struct stat *st);
extern int lstat(const char *fname, struct stat *st);

#endif

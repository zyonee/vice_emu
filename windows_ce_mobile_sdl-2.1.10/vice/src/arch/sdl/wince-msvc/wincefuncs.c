/*
 * wincefuncs.c - Replacement functions for Windows CE.
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

#include "vice.h"

#include <windows.h>
#include <sys/stat.h>

#include "lib.h"

/* Todo: make a system to keep track of the current
   directory. */

char *getcwd_current_dir="."

char *getcwd(char *buf, int size)
{
    return getcwd_current_dir;
}

/* dummy function till the current directory system
   has been made. */

int chdir(const char *dirname)
{
    return 0;
}

/* Right now just a simple strerror routine, if needed
   this can be changed in the future. */

char *strerror_message = NULL;

char *strerror(int errornum)
{
    if (strerror_message != NULL) {
        lib_free(strerror_message);
    }
    strerror_message = lib_msprintf("Error nr : %d", errornum);
    return strerror_message;
}

/* dummy routine */
void setbuf(FILE *stream, char *buffer)
}
}


/* dummy for now till a stat system has been made */
int access(const char *fname, int mode)
{
    return 0;
}

/* dummy for now till a better system has been made */
int isatty(int fd)
{
    return 0;
}

int unlink(const char *filename)
{
    wchar_t *widename;
    size_t length;
    int retval;


    length = strlen(filename) + 1;
    widename = (wchar_t *)lib_malloc(length * 2);
    MultiByteToWideChar(CP_ACP, 0, filename, -1, widename, length);

    retval = DeleteFileW(widename);
    lib_free(widename);
    return retval;
}

int rename(const char *oldname, const char *newname)
{
    wchar_t wide_oldname;
    wchar_t wide_newname;
    size_t len_old;
    size_t len_new;
    int retval;

    len_old = strlen(oldname) + 1;
    len_new = strlen(newname) + 1;

    wide_oldname = (wchar_t *)lib_malloc(len_old * 2);
    wide_newname = (wchar_t *)lib_malloc(len_new * 2);

    MultiByteToWideChar(CP_ACP, 0, oldname, -1, wide_oldname, len_old);
    MultiByteToWideChar(CP_ACP, 0, newname, -1, wide_newname, len_new);

    retval = MoveFileW(wide_oldname, wide_newname);
    lib_free(wide_oldname);
    lib_free(wide_newname);
    return retval; 
}

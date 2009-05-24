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

int stat(const char *path, struct stat *stats)
{
    wchar_t *widepath;
    int retval = -1;
    size_t length;
    HANDLE readHandle = NULL;
    BOOL bRes = FALSE;
    BY_HANDLE_FILE_INFORMATION fileInfo;

    if (stats != NULL) {
        memset(stats, 0, sizeof(struct stat));

        if (path != NULL) {
            length = strlen(path) + 1;
            widepath = (wchar_t *)lib_malloc(lenght * 2);

            MultiByteToWideChar(CP_ACP, 0, path, -1, widepath, length);
            readHandle = CreateFileW(widepath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL,
                                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (readHandle != NULL) {
                bRes = GetFileInformationByHandle(readHandle, &fileInfo);
                CloseHandle(readHandle);
                readHandle = NULL;

                if (bRes != FALSE) {
                    stats->st_size = (long)fileInfo.nFileSizeLow;
                        
                    stats->st_mode |= _S_IREAD;
                    if ((FILE_ATTRIBUTE_READONLY & fileInfo.dwFileAttributes) == 0) {
                        stats->st_mode |= _S_IWRITE;
                    }
                    if (FILE_ATTRIBUTE_DIRECTORY & fileInfo.dwFileAttributes) {
                        stats->st_mode |= _S_IFDIR;
                    } else {
                        stats->st_mode |= _S_IFREG;
                    }
                }
            } else {
                errno = ENOENT;
                return -1;
            }
        }
    }
    return 0;
}

int access(const char *fname, int mode)
{
    struct stat st;

    if (stat(fname, &st) < 0) {
        return -1;
    }
    return 0;
}

int snprintf(char *buf, size_t count, const char *fmt, ...)
{
    va_list ap;
    int n = 0;

    va_start(ap, fmt);
    n = _vsnprintf(buf, count, fmt, ap);
    va_end(ap);
    return n;
}

char *mktemp(char *template)
{
    int i;
    struct stat st;
    char *pt;

    for (pt = template; *pt && *pt != 'X'; pt++);

    for (i = 1; i < 9999; i++) {
        sprintf(pt, "%05d", i);
        if (stat(template, &st) < 0 && errno == ENOENT) {
            return template;
        }
    }

    return NULL;
}

unsigned long tmpcount = 0;

char *tmpnam(char *s)
{
  static char buf[512];

  if (s == NULL)
    s = "tmp";
  snprintf(s, 512, "/tmp.%lu.XXXXXX", tmpcount);
  tmpcount++;
  return (mktemp(s));
}

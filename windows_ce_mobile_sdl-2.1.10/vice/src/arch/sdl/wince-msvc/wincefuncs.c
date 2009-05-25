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
#include <errno.h>
#include <time.h>

#include "lib.h"

#define countof(array) (sizeof(array) / sizeof((array)[0]))

int errno;

/* Todo: make a system to keep track of the current
   directory. */

char *getcwd_current_dir=".";

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
{
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
    wchar_t *wide_oldname;
    wchar_t *wide_newname;
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
            widepath = (wchar_t *)lib_malloc(length * 2);

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

void rewind(FILE *f)
{
    fseek(f, 0L, SEEK_SET);
}

static void wincemsgbox(const char *fmt, ...)
{
    va_list ap;
    char buf[512];
    wchar_t widebuf[512];

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    va_end(ap);

    MultiByteToWideChar(CP_ACP, 0, buf, -1, widebuf, countof(widebuf));

    MessageBoxW(NULL, widebuf, _T("Message"), MB_OK);
}

void abort(void)
{
    char buf[256];
    wchar_t *progname = NULL;
    DWORD retval;

    progname = (wchar_t *)lib_malloc(256 * 2);
    retval = GetModuleFileNameW(NULL, progname, 256);
    if (retval != 0)
    {
        WideCharToMultiByte(CP_ACP, 0, progname, 256, buf, 256, NULL, NULL);
    }
    lib_free(progname);
    wincemsgbox("Aborted %s", buf);

    DebugBreak();

    exit(1);
}

#define DELTA_EPOCH 116444736000000000LL
#define fraction 10000000LL

static long long ft_to_ll(const FILETIME *f)
{
    long long t;
    t = (long long)f->dwHighDateTime << 32;
    t |= f->dwLowDateTime;
    return t;
}

static time_t ft_to_tt(const FILETIME* f)
{
    long long t;
    t = ft_to_ll(f);
    t -= DELTA_EPOCH;
    return (time_t)(t / fraction);
}

static void ll_to_ft(long long t, FILETIME* f)
{
    f->dwHighDateTime = (DWORD)((t >> 32) & 0x00000000FFFFFFFF);
    f->dwLowDateTime = (DWORD)(t & 0x00000000FFFFFFFF);
}

static void tt_to_ft(time_t t, FILETIME *f)
{
    long long time;

    time = t;
    time *= fraction;
    time += DELTA_EPOCH;

    ll_to_ft(time, f);
}

static void ft_from_yr(WORD year, FILETIME *f)
{
    SYSTEMTIME s = {0};

    s.wYear = year;
    s.wMonth = 1;
    s.wDayOfWeek = 1;
    s.wDay = 1;

    SystemTimeToFileTime (&s, f);
}

static int yd_from_st(const SYSTEMTIME *s)
{
    long long t;
    FILETIME f1, f2;

    ft_from_yr(s->wYear, &f1);
    SystemTimeToFileTime (s, &f2);

    t = ft_to_ll(&f2) - ft_to_ll(&f1);

    return ((t / fraction) / (60 * 60 * 24));
}

static void st_to_tm(SYSTEMTIME *s, struct tm *tm)
{
    tm->tm_year = s->wYear - 1900;
    tm->tm_mon = s->wMonth- 1;
    tm->tm_wday = s->wDayOfWeek;
    tm->tm_mday = s->wDay;
    tm->tm_yday = yd_from_st(s);
    tm->tm_hour = s->wHour;
    tm->tm_min = s->wMinute;
    tm->tm_sec = s->wSecond;
    tm->tm_isdst = 0;
}

time_t time(time_t *timer)
{
    SYSTEMTIME s;
    FILETIME f;
    time_t t;

    if (timer == NULL) {
        timer = &t;
    }

    GetSystemTime(&s);
    SystemTimeToFileTime(&s, &f);
    *timer = ft_to_tt(&f);

    return *timer;
}

struct tm *localtime(const time_t *timer)
{
    SYSTEMTIME ss, ls, s;
    FILETIME sf, lf, f;
    long long t, diff;
    struct tm tms;

    GetSystemTime(&ss);
    GetLocalTime(&ls);

    SystemTimeToFileTime(&ss, &sf);
    SystemTimeToFileTime(&ls, &lf);

    diff = ft_to_ll(&sf) - ft_to_ll(&lf);

    tt_to_ft(*timer, &f);
    t = ft_to_ll(&f) - diff;
    ll_to_ft(t, &f);
    FileTimeToSystemTime(&f, &s);
    st_to_tm(&s, &tms);

  return &tms;
}

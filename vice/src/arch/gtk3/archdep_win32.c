/** \file   archdep_win32.c
 * \brief   Miscellaneous Windows-specific stuff
 *
 * \author  Marco van den Heuvel <blackystardust68@yahoo.com>
 */

/*
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

#include <stdio.h>
#include <glib.h>
#include <windows.h>
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#if defined(HAVE_IO_H) && !defined(WINMIPS)
#include <io.h>
#endif

#ifdef HAVE_PROCESS_H
#include <process.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "lib.h"
#include "util.h"

#include "not_implemented.h"

#include "archdep.h"

/* fix VICE userdir */
#ifdef VICEUSERDIR
# undef VICEUSERDIR
#endif
#define VICEUSERDIR "vice"

/** \brief  Path separator used in GLib code
 */
static const gchar *path_separator = "\\";

/** \brief  Tokens that are illegal in a path/filename
 */
static const char *illegal_name_tokens = "/\\?*:|\"<>";


/** \brief  String containing search paths
 *
 * Allocated in the first call to archdep_default_sysfile_pathlist(),
 * deallocated in archdep_shutdown().
 */
static char *default_path = NULL;

#if 0
char *archdep_default_fliplist_file_name(void)
{
    NOT_IMPLEMENTED();
    return NULL;
}
#endif

/** \brief  Write message to Windows debugger/logger
 *
 * param[in]    level_string    log level string
 * param[in]    txt             log message
 *
 * \note    Shamelessly copied from win32/archdep.c
 *
 * \return  0 on success, < 0 on failure
 */
int archdep_default_logger(const char *level_string, const char *txt)
{
    char *out = lib_msprintf("*** %s %s\n", level_string, txt);
    OutputDebugString(out);
    lib_free(out);
    return 0;
}


/** \brief  Generate path to vice.ini
 *
 * The value returned needs to be freed using lib_free()
 *
 * \return  absolute path to vice.ini
 */
char *archdep_default_resource_file_name(void)
{
    char *cfg;
    gchar *tmp;
    char *path;

    cfg = archdep_user_config_path();
    tmp = g_build_path(path_separator, cfg, "vice.ini", NULL);
    /* transfer ownership to VICE */
    path = lib_stralloc(tmp);
    g_free(tmp);
    lib_free(cfg);
    return path;
}


char *archdep_default_save_resource_file_name(void)
{
    /* XXX: taken from SDL, but is wrong, this returns the directoru in which
     *      $emu is executing plus 'vice.ini', while the proper dir would be
     *      %APPDATA%/vice
     */
    return archdep_default_resource_file_name();
}


/** \brief  Build a list of search paths for emulator \a emu_id
 *
 * \param[in]   emu_id  emulator name (without '.exe')
 *
 * \return  string containing search paths
 */
char *archdep_default_sysfile_pathlist(const char *emu_id)
{

    if (default_path == NULL) {
        const char *boot_path = archdep_boot_path();

        default_path = util_concat(
                boot_path, "\\", emu_id, ARCHDEP_FINDPATH_SEPARATOR_STRING,
                boot_path, "\\DRIVES", ARCHDEP_FINDPATH_SEPARATOR_STRING,
                boot_path, "\\PRINTER", NULL);
    }

    return lib_stralloc(default_path);
}


/** \brief  Generate a heap-allocated absolute path from \a orig_name
 *
 * \param[out]  return_path object to store path in
 * \param[in]   orig_name   the (relative) path to convert
 *
 * \return  0 on success
 *
 * FIXME:   There is a GetFullPathName() function on Windows, perhaps use that?
 */
int archdep_expand_path(char **return_path, const char *orig_name)
{
    /* taken from src/arch/win32/archdep.c */
    *return_path = lib_stralloc(orig_name);
    return 0;
}

/** \brief  Get the absolute path to the VICE dir
 *
 * \return  Path to VICE's directory
 */
char boot_path[MAX_PATH];
const char *archdep_boot_path(void)
{
    char *checkpath;

    GetModuleFileName(NULL, boot_path, MAX_PATH);

    checkpath = boot_path + strlen(boot_path);

    while (*checkpath != '\\') {
        checkpath--;
    }
    *checkpath = 0;

    return boot_path;
}



char *archdep_make_backup_filename(const char *fname)
{
    return util_concat(fname, ".bak", NULL);
}


void archdep_shutdown(void)
{
    if (default_path != NULL) {
        lib_free(default_path);
    }
    if (argv0 != NULL) {
        lib_free(argv0);
        argv0 = NULL;
    }

    if (program_name != NULL) {
        lib_free(program_name);
        program_name = NULL;
    }

    /* archdep_network_shutdown(); */

    /* partially implemented */
    INCOMPLETE_IMPLEMENTATION();
}


/** \brief  Spawn new process
 *
 * Shamelessly stolen from arch/sdl/archdep_win32.c
 */
int archdep_spawn(const char *name, char **argv, char **pstdout_redir, const char *stderr_redir)
{
    int new_stdout, new_stderr;
    int old_stdout_mode, old_stderr_mode;
    int old_stdout, old_stderr;
    int retval;
    char *stdout_redir = NULL;

    if (pstdout_redir != NULL) {
        if (*pstdout_redir == NULL) {
            *pstdout_redir = archdep_tmpnam();
        }
        stdout_redir = *pstdout_redir;
    }

    new_stdout = new_stderr = old_stdout = old_stderr = -1;

    /* Make sure we are in binary mode.  */
    old_stdout_mode = _setmode(STDOUT_FILENO, _O_BINARY);
    old_stderr_mode = _setmode(STDERR_FILENO, _O_BINARY);

    /* Redirect stdout and stderr as requested, saving the old
       descriptors.  */
    if (stdout_redir != NULL) {
        old_stdout = _dup(STDOUT_FILENO);
        new_stdout = _open(stdout_redir, _O_WRONLY | _O_TRUNC | _O_CREAT, _S_IWRITE | _S_IREAD);
        if (new_stdout == -1) {
            log_error(LOG_DEFAULT, "open(\"%s\") failed: %s.", stdout_redir, strerror(errno));
            retval = -1;
            goto cleanup;
        }
        _dup2(new_stdout, STDOUT_FILENO);
    }
    if (stderr_redir != NULL) {
        old_stderr = _dup(STDERR_FILENO);
        new_stderr = _open(stderr_redir, _O_WRONLY | _O_TRUNC | _O_CREAT, _S_IWRITE | _S_IREAD);
        if (new_stderr == -1) {
            log_error(LOG_DEFAULT, "open(\"%s\") failed: %s.", stderr_redir, strerror(errno));
            retval = -1;
            goto cleanup;
        }
        _dup2(new_stderr, STDERR_FILENO);
    }

    /* Spawn the child process.  */
    retval = (int)_spawnvp(_P_WAIT, name, (const char **)argv);

cleanup:
    if (old_stdout >= 0) {
        _dup2(old_stdout, STDOUT_FILENO);
        _close(old_stdout);
    }
    if (old_stderr >= 0) {
        _dup2(old_stderr, STDERR_FILENO);
        _close(old_stderr);
    }
    if (old_stdout_mode >= 0) {
        _setmode(STDOUT_FILENO, old_stdout_mode);
    }
    if (old_stderr_mode >= 0) {
        _setmode(STDERR_FILENO, old_stderr_mode);
    }
    if (new_stdout >= 0) {
        _close(new_stdout);
    }
    if (new_stderr >= 0) {
        _close(new_stderr);
    }

    return retval;
}




int archdep_stat(const char *file_name, unsigned int *len, unsigned int *isdir)
{
    struct stat statbuf;

    if (stat(file_name, &statbuf) < 0) {
        return -1;
    }

    *len = statbuf.st_size;
    *isdir = S_ISDIR(statbuf.st_mode);
    return 0;
}


/** \brief  Create a unique temporary filename
 *
 * \return  unique filename in the %TEMP% directory
 */
char *archdep_tmpnam(void)
{
    if (getenv("temp")) {
        return util_concat(getenv("temp"), tmpnam(NULL), NULL);
    } else if (getenv("tmp")) {
        return util_concat(getenv("tmp"), tmpnam(NULL), NULL);
    } else {
        return lib_stralloc(tmpnam(NULL));
    }
}

#if 0
void archdep_signals_pipe_set(void)
{
    NOT_IMPLEMENTED();
}

void archdep_signals_pipe_unset(void)
{
    NOT_IMPLEMENTED();
}
#endif

char *archdep_default_rtc_file_name(void)
{
    /* XXX: temp hack, should resolve %APPDATA%\\vice
     *      2017-09-13 -- Compyx */
    return util_concat(archdep_boot_path(), "\\vice.rtc", NULL);
}

int archdep_file_is_chardev(const char *name)
{
    INCOMPLETE_IMPLEMENTATION();
    return 0;
}

int archdep_file_is_blockdev(const char *name)
{
    return 0;
}

int archdep_fix_permissions(const char *file_name)
{
    NOT_IMPLEMENTED();
    return 0;
}

/* Provide a usleep replacement */
void vice_usleep(uint64_t waitTime)
{
    uint64_t time1 = 0, time2 = 0, freq = 0;

    QueryPerformanceCounter((LARGE_INTEGER *) &time1);
    QueryPerformanceFrequency((LARGE_INTEGER *)&freq);

    do {
        QueryPerformanceCounter((LARGE_INTEGER *) &time2);
    } while((time2-time1) < waitTime);
}

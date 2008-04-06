/*
 * archdep.c - Miscellaneous system-specific stuff.
 *
 * Written by
 *  Andreas Matthies <andreas.matthies@gmx.net>
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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "ui.h"

#ifdef HAVE_DIR_H
#include <dir.h>
#endif
#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "archdep.h"
#include "log.h"
#include "utils.h"

static char *orig_workdir;
static char *argv0;

int archdep_startup(int *argc, char **argv)
{
   	argv0 = stralloc(argv[0]);
    orig_workdir = getcwd(NULL, GET_PATH_MAX);

    return 0;
}

static char *program_name=NULL;

const char *archdep_program_name(void)
{
    if (program_name == NULL) {
        char *s, *e;
        int len;

        s = strrchr(argv0, '/');
        if (s == NULL)
            s = argv0;
        else
            s++;
        e = strchr(s, '.');
        if (e == NULL)
            e = argv0 + strlen(argv0);

        len = e - s + 1;
        program_name = xmalloc(len);
        memcpy(program_name, s, len - 1);
        program_name[len - 1] = 0;
    }

    return program_name;
}

static char *boot_path=NULL;

const char *archdep_boot_path(void)
{
    if (boot_path == NULL) {
        fname_split(argv0, &boot_path, NULL);

        /* This should not happen, but you never know...  */
        if (boot_path == NULL)
            boot_path = stralloc("./xxx");
    }

    return boot_path;
}

const char *archdep_default_sysfile_pathlist(const char *emu_id)
{
    static char *default_path;

    if (default_path == NULL) {
        const char *boot_path = archdep_boot_path();
        default_path = concat(boot_path,"/",emu_id,
                              FINDPATH_SEPARATOR_STRING,
                              boot_path,"/","DRIVES", NULL);
    }

    return default_path;
}

/* Return a malloc'ed backup file name for file `fname'.  */
char *archdep_make_backup_filename(const char *fname)
{
char    *tmp;

    tmp=concat(fname,NULL);
    tmp[strlen(tmp)-1]='~';
    return tmp;
}

const char *archdep_default_save_resource_file_name(void) {
    return archdep_default_resource_file_name();
}

const char *archdep_default_resource_file_name(void)
{
    static char *fname;

    if (fname != NULL)
        free(fname);

    fname = concat(archdep_boot_path(), "/vice.ini", NULL);
    return fname;
}

FILE *archdep_open_default_log_file(void)
{
    char *fname;
    FILE *f;

    fname = concat(archdep_boot_path(), "/vice.log", NULL);
    f = fopen(fname, "wt");
    free(fname);

    return f;
}

int archdep_num_text_lines(void)
{
    return 25;
}

int archdep_num_text_columns(void)
{
    return 80;
}

int archdep_default_logger(const char *level_string, const char *txt)
{
    return 0;
}

static RETSIGTYPE break64(int sig)
{
#ifdef SYS_SIGLIST_DECLARED
    log_message(LOG_DEFAULT, "Received signal %d (%s).",
                sig, sys_siglist[sig]);
#else
    log_message(LOG_DEFAULT, "Received signal %d.", sig);
#endif

    exit (-1);
}

void archdep_setup_signals(int do_core_dumps)
{

/* Activate this later...
    signal(SIGINT, break64);
    signal(SIGTERM, break64);

    if (!do_core_dumps) {
        signal(SIGSEGV,  break64);
        signal(SIGILL,   break64);
    }
*/
}

int archdep_path_is_relative(const char *path)
{
    if (path == NULL)
        return 0;

    /* `c:\foo', `c:/foo', `c:foo', `\foo' and `/foo' are absolute.  */

    return !((isalpha(path[0]) && path[1] == ':')
            || path[0] == '/' || path[0] == '\\');
}

int archdep_spawn(const char *name, char **argv,
                  const char *stdout_redir, const char *stderr_redir)
{
    pid_t child_pid;
    int child_status;

    child_pid = vfork();
    if (child_pid < 0) {
    log_error(LOG_DEFAULT, "vfork() failed: %s.", strerror(errno));
    return -1;
    } else if (child_pid == 0) {
    if (stdout_redir && freopen(stdout_redir, "w", stdout) == NULL) {
        log_error(LOG_DEFAULT, "freopen(\"%s\") failed: %s.",
                      stdout_redir, strerror(errno));
        _exit(-1);
    }
    if (stderr_redir && freopen(stderr_redir, "w", stderr) == NULL) {
        log_error(LOG_DEFAULT, "freopen(\"%s\") failed: %s.",
                      stderr_redir, strerror(errno));
        _exit(-1);
    }
    execvp(name, argv);
    _exit(-1);
    }

    if (waitpid(child_pid, &child_status, 0) != child_pid) {
        log_error(LOG_DEFAULT, "waitpid() failed: %s", strerror(errno));
    return -1;
    }

    if (WIFEXITED(child_status))
    return WEXITSTATUS(child_status);
    else
    return -1;

}


/* return malloced version of full pathname of orig_name */
int archdep_expand_path(char **return_path, const char *orig_name)
{
    /*  BeOS version   */
    *return_path = stralloc(orig_name);
    return 0;
}

void archdep_startup_log_error(const char *format, ...)
{
        char tmp[1024];
        va_list args;

        va_start(args, format);
        vsprintf(tmp, format, args);
        va_end(args);
}

char *archdep_quote_parameter(const char *name)
{
    return stralloc(name);
}

char *archdep_filename_parameter(const char *name)
{
    char *exp;
    char *a;
    archdep_expand_path(&exp, name);
    a = archdep_quote_parameter(exp);
    free(exp);
    return a;
}

char *archdep_tmpnam(void)
{
    return stralloc(tmpnam(NULL));
}

int archdep_file_is_gzip(const char *name)
{
    size_t l = strlen(name);

    if ((l < 4 || strcasecmp(name + l - 3, ".gz"))
        && (l < 3 || strcasecmp(name + l - 2, ".z"))
        && (l < 4 || toupper(name[l - 1]) != 'Z' || name[l - 4] != '.'))
        return 0;
    return 1;
}

int archdep_file_set_gzip(const char *name)
{
  return 0;
}

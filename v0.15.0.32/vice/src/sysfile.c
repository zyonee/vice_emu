/*
 * sysfile.h - Simple locator for VICE system files.
 *
 * Written by
 *  Ettore Perazzoli (ettore@comm2000.it)
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

#ifdef STDC_HEADERS
#include <stdio.h>
#endif

#include "sysfile.h"

#include "cmdline.h"
#include "findpath.h"
#include "resources.h"
#include "sysdep.h"
#include "utils.h"

#ifdef __riscos
#include "ROlib.h"
#include "machine.h"
#endif

/* ------------------------------------------------------------------------- */

/* Resources.  */

static char *default_path;
static char *system_path;
static char *expanded_system_path;

static int set_system_path(resource_value_t v)
{
    string_set(&system_path, (char *) v);

    if (expanded_system_path != NULL)
        free(expanded_system_path);

    expanded_system_path = subst(system_path, "$$", default_path);

    return 0;
}

static resource_t resources[] = {
    { "Directory", RES_STRING, (resource_value_t) "$$",
      (resource_value_t *) &system_path, set_system_path },
    { NULL },
};

/* Command-line options.  */

static cmdline_option_t cmdline_options[] = {
    { "-directory", SET_RESOURCE, 1, NULL, NULL, "Directory", NULL,
      "<path>", "Define search path to locate system files" },
    { NULL },
};


/* ------------------------------------------------------------------------- */

int sysfile_init(const char *emu_id)
{
    default_path = stralloc(sysdep_default_sysfile_pathlist(emu_id));

    return 0;
}

int sysfile_init_resources(void)
{
    return resources_register(resources);
}

int sysfile_init_cmdline_options(void)
{
    return cmdline_register_options(cmdline_options);
}

/* Locate a system file called `name' by using the search path in
   `Directory', checking that the file can be accesses in mode `mode', and
   return an open stdio stream for that file.  If `complete_path_return' is
   not NULL, `*complete_path_return' points to a malloced string with the
   complete path if the file was found or is NULL if not.  */
FILE *sysfile_open(const char *name, char **complete_path_return)
{
    char *p = NULL;
    FILE *f;

#ifdef __riscos
    f = NULL;

    p = (char*)malloc(strlen(default_path) + strlen(name) + 1);
    sprintf(p, "%s%s", default_path, name);
    if (access(p, R_OK))
    {
        free(p);
        p = (char*)malloc(strlen("Vice:DRIVES.") + strlen(name) + 1);
        sprintf(p, "Vice:DRIVES.%s", name);
    }
    if (complete_path_return != NULL)
       f = fopen(p, "r");
    if (f == NULL)
    {
       free(p);
       if (complete_path_return != NULL)
           *complete_path_return = NULL;
       return NULL;
    }
    else
    {
       if (complete_path_return != NULL)
          *complete_path_return = p;
       return f;
    }
#else
    p = findpath(name, expanded_system_path, R_OK);

    if (p == NULL) {
        if (complete_path_return != NULL)
            *complete_path_return = NULL;
        return NULL;
    } else {
        f = fopen(p, "r");

        if (f == NULL || complete_path_return == NULL) {
            free(p);
            p = NULL;
        }
        if (complete_path_return != NULL)
            *complete_path_return = p;
        return f;
    }
#endif
}

/* As `sysfile_open', but do not open the file.  Just return 0 if the file is
   found and is readable, or -1 if an error occurs.  */
int sysfile_locate(const char *name, char **complete_path_return)
{
    FILE *f = sysfile_open(name, complete_path_return);

    if (f != NULL) {
        fclose(f);
        return 0;
    } else
        return -1;
}
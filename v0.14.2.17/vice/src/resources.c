/*
 * resources.c - Resource handling for VICE.
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

/* This implements simple facilities to handle the resources and command-line
   options.  All the resources for the emulators can be stored in a single
   file, and they are separated by an `emulator identifier', i.e. the machine
   name between brackets (e.g. ``[C64]'').  All the resources are stored in
   the form ``ResourceName=ResourceValue'', and separated by newline
   characters.  Leading and trailing spaces are removed from the
   ResourceValue unless it is put between quotes (").  */

#include "vice.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "resources.h"
#include "utils.h"
#include "ui.h"

static int num_resources, num_allocated_resources;
static resource_t *resources;
static const char *machine_id;

/* FIXME: I don't like this to be here.  */
#ifdef __MSDOS__
#define RESOURCE_FILE_NAME	"VICERC"
#else
#define RESOURCE_FILE_NAME	".vicerc"
#endif

/* ------------------------------------------------------------------------- */

/* FIXME: We might want to use a hash table instead of a linear search some
   day.  */

int resources_register(const resource_t *r)
{
    const resource_t *sp;
    resource_t *dp;

    sp = r;
    dp = resources + num_resources;
    while (sp->name != NULL) {
        if (num_allocated_resources <= num_resources) {
            num_allocated_resources *= 2;
            resources = xrealloc(resources,
                                 num_allocated_resources * sizeof(resource_t));
            dp = resources + num_resources;
        }
        dp->name = sp->name;
        dp->type = sp->type;
        dp->factory_value = sp->factory_value;
        dp->value_ptr = sp->value_ptr;
        dp->set_func = sp->set_func;
        num_resources++, sp++, dp++;
    }

    return 0;
}

static resource_t *lookup(const char *name)
{
    int i;

    for (i = 0; i < num_resources; i++)
        if (strcasecmp(resources[i].name, name) == 0)
            return resources + i;

    return NULL;
}

/* ------------------------------------------------------------------------- */

int resources_init(const char *machine)
{
    int i;

    machine_id = stralloc(machine);
    num_allocated_resources = 100;
    num_resources = 0;
    resources = (resource_t *) xmalloc(num_allocated_resources
                                       * sizeof(resource_t));

    return 0;
}

int resources_set_value(const char *name, resource_value_t value)
{
    resource_t *r = lookup(name);

    if (r == NULL) {
        fprintf(stderr, "%s: Warning: unknown resource `%s'\n",
                __FUNCTION__, name);
        return -1;
    }

    return r->set_func(value);
}

int resources_get_value(const char *name, resource_value_t *value_return)
{
    resource_t *r = lookup(name);

    if (r == NULL) {
        fprintf(stderr, "%s: Warning: unknown resource `%s'\n",
                __FUNCTION__, name);
        return -1;
    }

    *value_return = *r->value_ptr;
    return 0;
}

void resources_set_defaults(void)
{
    int i;

    for (i = 0; i < num_resources; i++)
        resources[i].set_func(resources[i].factory_value);

    UiUpdateMenus();
}

/* ------------------------------------------------------------------------- */

#ifndef __MSDOS__

/* Return a malloced string with the name of the backup file corresponding to
   `fname'.  */
static char *make_backup_file_name(const char *fname)
{
    return concat(fname, "~", NULL);
}

#else  /* __MSDOS__ */

/* Return a malloced string with the name of the backup file corresponding to
   `fname'.  */
static char *make_backup_file_name(const char *fname)
{
    static char backup_name[MAXPATH];
    char drive[MAXDRIVE];
    char dir[MAXDIR];
    char name[MAXFILE];
    char ext[MAXEXT];

    fnsplit(fname, drive, dir, name, ext);
    fnmerge(backup_name, drive, dir, name, "BAK");

    return stralloc(backup_name);
}

#endif /* __MSDOS__ */

/* Input one line from the file descriptor `f'.  */
static int get_line(char *buf, int bufsize, FILE *f)
{
    char *r;
    int len;

    r = fgets(buf, bufsize, f);
    if (r == NULL)
	return -1;

    len = strlen(buf);

    if (len > 0) {
	char *p;

	/* Remove trailing newline character.  */
	if (*(buf + len - 1) == '\n')
	    len--;

	/* Remove useless spaces.  */
	while (*(buf + len - 1) == ' ')
	    len--;
	for (p = buf; *p == ' '; p++, len--)
	    ;
	memmove(buf, p, len + 1);
	*(buf + len) = '\0';
    }

    return len;
}

/* Check whether `buf' is the emulator ID for the machine we are emulating.  */
static int check_emu_id(const char *buf)
{
    int machine_id_len, buf_len;

    buf_len = strlen(buf);
    if (*buf != '[' || *(buf + buf_len - 1) != ']')
	return 0;

    if (machine_id == NULL)
	return 1;

    machine_id_len = strlen(machine_id);
    if (machine_id_len != buf_len - 2)
	return 0;

    if (strncmp(buf + 1, machine_id, machine_id_len) == 0)
	return 1;
    else
	return 0;
}

/* Return a malloced string containing the name of the default user-specific
   resource file.  Warning: assumes `boot_path' does not change (it should be
   always so).  */
static const char *default_resource_file(void)
{
    static char *fname = NULL;

    if (fname == NULL) {
#ifdef __MSDOS__
	/* On MS-DOS, always boot from the directory in which the binary is
	   stored. */
	fname = concat(boot_path, RESOURCE_FILE_NAME, NULL);
#else
	fname = concat(getenv("HOME"), "/", RESOURCE_FILE_NAME, NULL);
#endif
    }

    return (const char *)fname;
}

/* ------------------------------------------------------------------------- */

/* Read one resource line from the file descriptor `f'.  Return 1 on success,
   -1 on failure, 0 on EOF.  */
static int read_resource_item(FILE *f)
{
    char buf[1024];
    char *arg_ptr;
    int line_len, resname_len, arg_len;
    resource_t *r;

    line_len = get_line(buf, 1024, f);

    if (line_len < 0)
	return 0;

    /* Ignore empty lines.  */
    if (*buf == '\0')
	return 1;

    if (check_emu_id(buf)) {
	/* End of emulator-specific section.  */
	return 0;
    }

    arg_ptr = strchr(buf, '=');
    if (!arg_ptr)
	return -1;

    resname_len = arg_ptr - buf;
    arg_ptr++;
    arg_len = strlen(arg_ptr);

    /* If the value is between quotes, remove them.  */
    if (*arg_ptr == '"' && *(arg_ptr + arg_len - 1) == '"') {
	*(arg_ptr + arg_len - 1) = '\0';
	arg_ptr++;
    }

    *(buf + resname_len) = '\0';

    {
        int result;

        r = lookup(buf);
        if (r == NULL) {
            fprintf(stderr, "Unknown resource `%s'.\n", buf);
            return -1;
        }

        switch (r->type) {
          case RES_INTEGER:
            result = r->set_func((resource_value_t) atoi(arg_ptr));
            break;
          case RES_STRING:
            result = r->set_func((resource_value_t) arg_ptr);
            break;
          default:
            fprintf(stderr, "Warning: Unknown resource type for `%s'.\n",
                    r->name);
            result = -1;
        }

        if (result < 0)
            fprintf(stderr, "Warning: Cannot assign value to resource `%s'.\n",
                    r->name);

        return result;
    }
}

/* Load the resources from file `fname'.  If `fname' is NULL, load them from
   the default resource file. */
int resources_load(const char *fname)
{
    FILE *f;
    int retval;
    int line_num;
    int err = 0;

    if (fname == NULL)
	fname = default_resource_file();

    printf("Reading configuration file `%s'.\n", fname);

#ifdef __MSDOS__
    f = fopen(fname, "rt");
#else
    f = fopen(fname, "r");
#endif

    if (f == NULL) {
	/*perror(fname);*/
	return -1;
    }

    /* Find the start of the configuration section for this emulator. */
    for (line_num = 1; ; line_num++) {
	char buf[1024];

	if (get_line(buf, 1024, f) < 0) {
	    fclose(f);
	    return -1;
	}

	if (check_emu_id(buf)) {
	    line_num++;
	    break;
	}
    }

    do {
	retval = read_resource_item(f);
	if (retval == -1) {
	    fprintf(stderr, "%s: Invalid resource specification at line %d.\n",
		    fname, line_num);
	    err = 1;
	}
	line_num++;
    } while (retval != 0);

    fclose(f);

    /* Update the values in the UI menus.  */
    UiUpdateMenus();
    return err ? -1 : 0;
}

/* Write the resource specification for resource number `num' to file
   descriptor `f'.  */
static void write_resource_item(FILE *f, int num)
{
    resource_value_t v;

    fprintf(f, "%s=", resources[num].name);
    v = *resources[num].value_ptr;
    switch (resources[num].type) {
      case RES_INTEGER:
        fprintf(f, "%d", (int)v);
        break;
      case RES_STRING:
        if ((char *)v != NULL)
            fprintf(f, "\"%s\"", (char *)v);
        break;
      default:
        fprintf(stderr, "Warning: Unknown value type for resource `%s'.\n",
                resources[num].name);
    }
    fputc('\n', f);
}

/* Save all the resources into file `fname'.  If `fname' is NULL, save them
   in the default resource file.  Writing the resources does not destroy the
   resources for the other emulators.  */
int resources_save(const char *fname)
{
    char *backup_name;
    FILE *in_file, *out_file;
    int have_old;
    int i;

    if (fname == NULL)
	fname = default_resource_file();

    /* Make a backup copy of the existing configuration file.  */
    backup_name = make_backup_file_name(fname);
    if (rename(fname, backup_name) == 0)
	have_old = 1;
    else
	have_old = 0;

    printf("Writing configuration file `%s'.\n", fname);

#ifdef __MSDOS__
    out_file = fopen(fname, "wt");
#else
    out_file = fopen(fname, "w");
#endif

    if (!out_file) {
	perror(fname);
	free (backup_name);
	return -1;
    }

    if (have_old) {
#ifdef __MSDOS__
	in_file = fopen(backup_name, "rt");
#else
	in_file = fopen(backup_name, "r");
#endif

	if (!in_file) {
	    fclose(out_file);
	    perror(backup_name);
	    free(backup_name);
	    return -1;
	}

	/* Copy the configuration for the other emulators.  */
	while (1) {
	    char buf[1024];

	    if (get_line(buf, 1024, in_file) < 0)
		break;

	    if (check_emu_id(buf))
		break;

	    fprintf(out_file, "%s\n", buf);
	}
    } else
        in_file = NULL;

    /* Write our current configuration.  */
    fprintf(out_file,"[%s]\n", machine_id);
    for (i = 0; resources[i].name != NULL; i++)
	write_resource_item(out_file, i);

    if (have_old) {
	char buf[1024];

	/* Skip the old configuration for this emulator.  */
	while (1) {
	    if (get_line(buf, 1024, in_file) < 0)
		break;

	    if (check_emu_id(buf)) {
		fprintf(out_file, "%s\n", buf);
		break;
	    }
	}

	if (!feof(in_file)) {
	    /* Copy the configuration for the other emulators.  */
	    while (get_line(buf, 1024, in_file) >= 0)
		fprintf(out_file, "%s\n", buf);
	}
    }

    if (in_file)
	fclose(in_file);

    fclose(out_file);
    free(backup_name);
    return 0;
}

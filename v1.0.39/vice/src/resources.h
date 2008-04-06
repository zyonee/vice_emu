/*
 * resources.h - Resource handling for VICE.
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

#ifndef _RESOURCES_H
#define _RESOURCES_H

#include <stdio.h>

typedef enum resource_type {RES_INTEGER, RES_STRING} resource_type_t;

typedef void *resource_value_t;

typedef int resource_set_func_t(resource_value_t v);

/* Warning: all the pointers should point to areas that are valid throughout
   the execution.  No reallocation is performed.  */
typedef struct resource {

    /* Resource name.  */
    const char *name;

    /* Type of resource.  */
    resource_type_t type;

    /* Factory default value.  */
    resource_value_t factory_value;

    /* Pointer to the value.  This is only used for *reading* it.  To change
       it, use `set_func'.  */
    resource_value_t *value_ptr;

    /* Function to call to set the value.  */
    resource_set_func_t *set_func;

} resource_t;

#define RESERR_FILE_NOT_FOUND       -1
#define RESERR_FILE_INVALID         -2
#define RESERR_READ_ERROR           -3
#define RESERR_CANNOT_CREATE_FILE   -4

/* ------------------------------------------------------------------------- */

int resources_init(const char *machine);
int resources_register(const resource_t *r);
int resources_set_value(const char *name, resource_value_t value);
int resources_set_value_string(const char *name, const char *value);
int resources_toggle(const char *name, resource_value_t *new_value_return);
int resources_get_value(const char *name, resource_value_t *value_return);
int resources_get_default_value(const char *name, 
				const resource_value_t *value_return);
resource_type_t resources_query_type(const char *name);
int resources_save(const char *fname);
int resources_load(const char *fname);

int resources_write_item_to_file(FILE *fp, const char *name);
int resources_read_item_from_file(FILE *fp);

void resources_set_defaults(void);

#endif /* _RESOURCES_H */

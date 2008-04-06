/*
 * attach.c - File system attach management.
 *
 * Written by
 *  Andreas Boose (boose@unixserv.rz.fh-hannover.de)
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
#include <stdlib.h>
#include <stdio.h>
#endif

#include "attach.h"
#include "fliplist.h"
#include "resources.h"
#include "serial.h"
#include "ui.h"
#include "vdrive.h"

static int file_system_device_enabled[4];

static int set_file_system_device8(resource_value_t v);
static int set_file_system_device9(resource_value_t v);
static int set_file_system_device10(resource_value_t v);
static int set_file_system_device11(resource_value_t v);

static resource_t resources[] = {
    { "FileSystemDevice8", RES_INTEGER, (resource_value_t) 1,
      (resource_value_t *) &file_system_device_enabled[0],
      set_file_system_device8 },
    { "FileSystemDevice9", RES_INTEGER, (resource_value_t) 1,
      (resource_value_t *) &file_system_device_enabled[1],
      set_file_system_device9 },
    { "FileSystemDevice10", RES_INTEGER, (resource_value_t) 1,
      (resource_value_t *) &file_system_device_enabled[2],
      set_file_system_device10 },
    { "FileSystemDevice11", RES_INTEGER, (resource_value_t) 1,
      (resource_value_t *) &file_system_device_enabled[3],
      set_file_system_device11 },
    { NULL }
};

/* ------------------------------------------------------------------------- */

static drive_attach_func_t attach_hooks[4];
static drive_detach_func_t detach_hooks[4];

int file_system_init_resources(void)
{
    return resources_register(resources);
}

/* Warning: this must be called /before/ `file_system_init()'.  */
int file_system_set_hooks(int unit,
                          int (*attach_func)(void *),
                          int (*detach_func)(void *))
{
    if (unit < 8 || unit > 11)
        return -1;

    attach_hooks[unit - 8] = attach_func;
    detach_hooks[unit - 8] = detach_func;

    return 0;
}

void file_system_init(void)
{
    int i;

    for (i = 0; i < 4; i++) {
        initialize_1541(i + 8, ((file_system_device_enabled[0] ? DT_FS : DT_DISK)
                                | DT_1541),
                        attach_hooks[i], detach_hooks[i], NULL);
    }
}

static int set_file_system_device8(resource_value_t v)
{
    serial_t *p;
    DRIVE *floppy;

    file_system_device_enabled[0] = (int) v;

    p = serial_get_device(8);
    floppy = (DRIVE *)p->info;
    if (floppy != NULL) {
	if (floppy->ActiveFd == NULL) {
	    p->inuse = 0;
	    initialize_1541(8, (file_system_device_enabled[0]
                                ? DT_FS : DT_DISK) | DT_1541,
                            attach_hooks[0], detach_hooks[0], floppy);
	}
    }
    return 0;
}

static int set_file_system_device9(resource_value_t v)
{
    serial_t *p;
    DRIVE *floppy;

    file_system_device_enabled[1] = (int) v;

    p = serial_get_device(9);
    floppy = (DRIVE *)p->info;
    if (floppy != NULL) {
	if (floppy->ActiveFd == NULL) {
	    p->inuse = 0;
	    initialize_1541(9, (file_system_device_enabled[1]
                                ? DT_FS : DT_DISK) | DT_1541,
                            attach_hooks[1], detach_hooks[1], floppy);
	}
    }
    return 0;
}

static int set_file_system_device10(resource_value_t v)
{
    serial_t *p;
    DRIVE *floppy;

    file_system_device_enabled[2] = (int) v;

    p = serial_get_device(10);
    floppy = (DRIVE *)p->info;
    if (floppy != NULL) {
	if (floppy->ActiveFd == NULL) {
	    p->inuse = 0;
	    initialize_1541(10, (file_system_device_enabled[2]
                                 ? DT_FS : DT_DISK) | DT_1541,
                            attach_hooks[2], detach_hooks[2], floppy);
	}
    }
    return 0;
}

static int set_file_system_device11(resource_value_t v)
{
    serial_t *p;
    DRIVE *floppy;

    file_system_device_enabled[3] = (int) v;

    p = serial_get_device(11);
    floppy = (DRIVE *)p->info;
    if (floppy != NULL) {
	if (floppy->ActiveFd == NULL) {
	    p->inuse = 0;
	    initialize_1541(11, (file_system_device_enabled[3]
                                 ? DT_FS : DT_DISK) | DT_1541,
                            attach_hooks[3], detach_hooks[3], floppy);
	}
    }
    return 0;
}

/* ------------------------------------------------------------------------- */

int file_system_attach_disk(int unit, const char *filename)
{
    serial_t *p;
    DRIVE *floppy;

    p = serial_get_device(unit);
    floppy = (DRIVE *)p->info;
    if ((floppy == NULL) || ((floppy->type & DT_FS) == DT_FS)) {
        p->inuse = 0;
        initialize_1541(unit, DT_DISK | DT_1541,
                        attach_hooks[unit - 8],
                        detach_hooks[unit - 8],
                        floppy);
    }

    if (serial_select_file(DT_DISK | DT_1541, unit, filename) < 0) {
        file_system_detach_disk(unit);
        return -1;
    } else {
        flip_set_current(unit, filename);
	ui_display_drive_current_image(unit - 8, filename);
        return 0;
    }
}

void file_system_detach_disk(int unit)
{
    int i;
    
    if (unit < 0) {
	serial_remove_file(8);
	set_file_system_device8((resource_value_t)
                                 file_system_device_enabled[0]);
	serial_remove_file(9);
	set_file_system_device9((resource_value_t)
                                 file_system_device_enabled[1]);
	serial_remove_file(10);
	set_file_system_device10((resource_value_t)
                                  file_system_device_enabled[2]);
	serial_remove_file(11);
	set_file_system_device11((resource_value_t)
                                  file_system_device_enabled[3]);
	/* XXX Fixme: Hardcoded 2 drives here! */
	for (i = 8; i<=9; i++)
	    ui_display_drive_current_image(i - 8, "");
	    
    } else {
	serial_remove_file(unit);
	switch(unit) {
	  case 8:
	    set_file_system_device8((resource_value_t)
                                     file_system_device_enabled[0]);
	    ui_display_drive_current_image(0, "");
	    break;
	  case 9:
	    set_file_system_device9((resource_value_t)
                                     file_system_device_enabled[1]);
	    ui_display_drive_current_image(1, "");
	    break;
	  case 10:
	    set_file_system_device10((resource_value_t)
                                      file_system_device_enabled[2]);
	    break;
	  case 11:
	    set_file_system_device11((resource_value_t)
                                      file_system_device_enabled[3]);
	    break;
	}
    }
}

/* ------------------------------------------------------------------------- */

#define SNAP_MAJOR 1
#define SNAP_MINOR 0

int vdrive_write_snapshot_module(snapshot_t *s, int start)
{
    int i;
    char snap_module_name[14];
    snapshot_module_t *m;
    serial_t *p;
    DRIVE *floppy;

    for (i = start; i <= 11; i++) {

        p = serial_get_device(i);
        floppy = (DRIVE *)p->info;
        if (floppy->ActiveFd > 0) {
            sprintf(snap_module_name, "VDRIVEIMAGE%i", i);
            m = snapshot_module_create(s, snap_module_name, SNAP_MAJOR,
                                       SNAP_MINOR);
            if (m == NULL)
                return -1;

            if (snapshot_module_write_byte_array(m,
                                                 (BYTE *)floppy->ActiveName,
                                                 sizeof(floppy->ActiveName)) < 0) {
                if (m != NULL)
                    snapshot_module_close(m);
                return -1;
            }
            snapshot_module_close(m);
        }
    }
    return 0;
}

int vdrive_read_snapshot_module(snapshot_t *s, int start)
{
    BYTE major_version, minor_version;
    int i;
    snapshot_module_t *m;
    char snap_module_name[14];

    for (i = start; i <= 11; i++) {

        sprintf(snap_module_name, "VDRIVEIMAGE%i", i);
        m = snapshot_module_open(s, snap_module_name,
                                 &major_version, &minor_version);
        if (m == NULL)
            return 0;

        if (major_version > SNAP_MAJOR || minor_version > SNAP_MINOR) {
            log_message(vdrive_log,
                        "Snapshot module version (%d.%d) newer than %d.%d.",
                        major_version, minor_version, SNAP_MAJOR, SNAP_MINOR);
        }
        snapshot_module_close(m);
    }
    return 0;
}


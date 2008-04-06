/*
 * prdevice.c - Printer device.
 *
 * Written by
 *  Andr� Fachat        (a.fachat@physik.tu-chemnitz.de)
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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "resources.h"
#include "cmdline.h"
#include "drive.h"
#include "file.h"
#include "charsets.h"
#include "utils.h"
#include "prdevice.h"
#include "print.h"

/*
 * At this time only one printer is supported and it is attached to
 * device 4.
 * Secondary address simply switched conversion modes (none of them are
 * implemented so far, so secondary address is simply ignored.
 *
 * The actual printing is done via print services in the arch/x/print.c
 * file; prototypes are in print.h
 */

static int prdevice_attach(int);
static int prdevice_detach(int);

/***********************************************************************
 * resource handling
 */

static int pr4_device;
static int pr4_enabled;

static int set_pr4_device(resource_value_t v) {
    pr4_device = (int) v;
    return 0;
}

static int set_pr4_enabled(resource_value_t v) {
    int flag = ((int) v) ? 1 : 0;
    if(pr4_enabled && !flag) {
	prdevice_detach(4);
    }
    if(flag && !pr4_enabled) {
        prdevice_attach(4);
    }
    pr4_enabled = flag;

    return 0;
}

static resource_t resources[] = {
    { "Printer4", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) &pr4_enabled, set_pr4_enabled },
    { "Printer4Dev", RES_INTEGER, (resource_value_t) 0,
      (resource_value_t *) &pr4_device, set_pr4_device },
    { NULL }
};

int prdevice_init_resources(void) {
    return resources_register(resources);
}

static cmdline_option_t cmdline_options[] = {
    { "-printer4", SET_RESOURCE, 0, NULL, NULL, "Printer4",
	(resource_value_t) 1, NULL,
	"Enable the IEC device #4 printer emulation" },
    { "+printer4", SET_RESOURCE, 0, NULL, NULL, "Printer4",
	(resource_value_t) 0, NULL,
	"Disable the IEC device #4 printer emulation" },
    { "-pr4dev", SET_RESOURCE, 1, NULL, NULL, "Printer4Device",
      "<0-2>", "Specify VICE printer device for IEC printer #4" },
    { NULL }
};

int prdevice_init_cmdline_options(void) {
    return cmdline_register_options(cmdline_options);
}

/***********************************************************************/

static int currfd;
static int inuse;

static int write_pr(void *var, BYTE byte, int secondary) {

    /* FIXME: switch(secondary) for code conversion */

    if(!inuse) {
	fprintf(stderr,"prdevice: printing while printer not open!\n");
	return -1;
    }

    return print_putc(currfd, byte);
}

static int open_pr(void *var, char *name, int length, int secondary) {

    if(inuse) {
	fprintf(stderr, "prdevice: open printer while still open - ignoring\n");
	return 0;
    }

    currfd = print_open(pr4_device);
    if(currfd < 0) {
	fprintf(stderr, "prdevice: Couldn't open device %d\n", pr4_device);
	return -1;
    }

    inuse = 1;

    return 0;
}


static int close_pr(void *var, int secondary) {

    if(!inuse) {
	fprintf(stderr, "prdevice: close printer while being closed - ignoring\n");
	return 0;
    }

    print_close(currfd);
    inuse = 0;

    return 0;
}


static int flush_pr(void *var, int secondary) {

    if(!inuse) {
	fprintf(stderr, "prdevice: flush printer while being closed - ignoring\n");
	return 0;
    }

    print_flush(currfd);

    return 0;
}

static int  fn()
{
    return 0x80;
}


static int prdevice_attach(int device)
{
    inuse = 0;

    if (serial_attach_device(device, (char *) NULL, (char *) "Printer device",
	    (int (*)(void *, BYTE *, int))fn,
	    write_pr,
	    open_pr,
	    close_pr,
	    flush_pr)) {
	return 1;
    }
    return 0;
}

static int prdevice_detach(int device)
{
    fprintf(stderr,"Printer device #4: Don't know how to detach (yet)\n");
    return 0;
}


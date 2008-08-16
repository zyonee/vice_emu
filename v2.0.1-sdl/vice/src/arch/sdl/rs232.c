/*
 * rs232.c - RS232 emulation.
 *
 * Written by
 *  André Fachat <a.fachat@physik.tu-chemnitz.de>
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

/*
 * The RS232 emulation captures the bytes sent to the RS232 interfaces
 * available (currently ACIA 6551, std C64 and Daniel Dallmanns fast RS232
 * with 9600 Baud).
 * The characters captured are sent to a file or an attached process.
 * Characters sent from a process are sent back to the
 * chip emulations.
 *
 */

#include "vice.h"

#include <errno.h>
#include <stdio.h>

#include "cmdline.h"
#include "log.h"
#include "resources.h"
#include "types.h"


/* ------------------------------------------------------------------------- */

/* resource handling */

#define NUM_DEVICES 4

extern char *devfile[NUM_DEVICES];
static int devbaud[NUM_DEVICES];

static int set_devbaud(int val, void *param)
{
    devbaud[(int)param] = val;
    return 0;
}

/* ------------------------------------------------------------------------- */

static const resource_int_t resources_int[] = {
    { "RsDevice1Baud", 9600, RES_EVENT_NO, NULL,
      &devbaud[0], set_devbaud, (void *)0 },
    { "RsDevice2Baud", 9600, RES_EVENT_NO, NULL,
      &devbaud[1], set_devbaud, (void *)1 },
    { "RsDevice3Baud", 9600, RES_EVENT_NO, NULL,
      &devbaud[2], set_devbaud, (void *)2 },
    { "RsDevice4Baud", 9600, RES_EVENT_NO, NULL,
      &devbaud[3], set_devbaud, (void *)3 },
    { NULL }
};

int rs232_resources_init(void)
{
    return resources_register_int(resources_int);
}

void rs232_resources_shutdown(void)
{
}

static const cmdline_option_t cmdline_options[] = {
    { "-rsdev1baud", SET_RESOURCE, 1, NULL, NULL, "RsDevice1Baud", NULL,
      "<baudrate>", N_("Specify baudrate of first RS232 device") },
    { "-rsdev2baud", SET_RESOURCE, 1, NULL, NULL, "RsDevice2Baud", NULL,
      "<baudrate>", N_("Specify baudrate of second RS232 device") },
    { "-rsdev3baud", SET_RESOURCE, 1, NULL, NULL, "RsDevice3Baud", NULL,
      "<baudrate>", N_("Specify baudrate of third RS232 device") },
    { "-rsdev4baud", SET_RESOURCE, 1, NULL, NULL, "RsDevice4Baud", NULL,
      "<baudrate>", N_("Specify baudrate of 4th RS232 device") },
    { NULL }
};

int rs232_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------- */

void rs232_close(int fd);

/* initializes all RS232 stuff */
void rs232_init(void)
{
}

/* reset RS232 stuff */
void rs232_reset(void)
{
}

/* opens a rs232 window, returns handle to give to functions below. */
int rs232_open(int device)
{
    return 0;
}

/* closes the rs232 window again */
void rs232_close(int fd)
{
    return 0;
}

/* sends a byte to the RS232 line */
int rs232_putc(int fd, BYTE b)
{
    return 0;
}

/* gets a byte to the RS232 line, returns !=0 if byte received, byte in *b. */
int rs232_getc(int fd, BYTE * b)
{
    return 0;
}


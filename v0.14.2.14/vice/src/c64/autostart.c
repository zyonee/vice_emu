/*
 * autostart.c - automatic image loading and starting
 *
 * Written by
 *  Teemu Rantanen      (tvr@cs.hut.fi)
 *  Ettore Perazzoli	(ettore@comm2000.it)
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

#include "vmachine.h"
#include "c64mem.h"
#include "serial.h"
#include "warn.h"
#include "resources.h"
#include "drive.h"
#include "tape.h"
#include "interrupt.h"
#include "true1541.h"
#include "ui.h"

#include "autostart.h"
#include "utils.h"
#include "kbdbuf.h"

/* Current state of the routine autostart.  */
static int autostartmode = AUTOSTART_NONE;

/* Warnings.  */
static warn_t *pwarn = NULL;

/* Flag: was true 1541 emulation turned on when we started booting the disk
   image?  */
static int orig_true1541_state = 0;

/* Program name to load. NULL if default */
static char *autostart_program_name = NULL;

/* ------------------------------------------------------------------------- */

/* Deallocate program name if we have one */
static void deallocate_program_name(void)
{
    if (autostart_program_name) {
	free(autostart_program_name);
	autostart_program_name = NULL;
    }
}


/* Big kludge: check whether the string `str' is in RAM at the address
   `addr'.  */
static int check(const char *str, int addr)
{
    int				i;
    for (i = 0; str[i]; i++)
    {
	if (ram[addr + i] != str[i] % 64)
	{
	    if (ram[addr + i] != 32
		&& ram[addr + i] != 0
		&& ram[addr + i] != 255)
	    {
		autostartmode = AUTOSTART_ERROR;
		deallocate_program_name();
		warn(pwarn, -1, "disabling autostart");
		return 0;
	    }
	    return 0;
	}
    }
    return 1;
}

static void settrue1541mode(int on)
{
    if (on)
        true1541_enable();
    else
        true1541_disable();

    UiUpdateMenus();
}

/* Initialize autostart.  */
void autostart_init(void)
{
    if (!pwarn)
	pwarn = warn_init("AUTOSTART", 32);
}

/* Execute the actions for the current `autostartmode', advancing to the next
   mode if necessary.  */
void autostart_advance(void)
{
    char		*tmp;

    switch (autostartmode)
    {
    case AUTOSTART_HASTAPE:
	if (check("READY.", 1024+5*40))
	{
	    warn(pwarn, -1, "loading tape");
	    if (autostart_program_name) {
		tmp = xmalloc(strlen(autostart_program_name) + 10);
		sprintf(tmp, "load\"%s\"\r", autostart_program_name);
		kbd_buf_feed(tmp);
		free(tmp);
	    }
	    else
		kbd_buf_feed("load\r");
	    autostartmode = AUTOSTART_LOADINGTAPE;
	    deallocate_program_name();
	}
	break;
    case AUTOSTART_LOADINGTAPE:
	if (check("READY.", 1024+11*40))
	{
	    warn(pwarn, -1, "starting program");
	    kbd_buf_feed("run\r");
	    autostartmode = AUTOSTART_DONE;
	}
	break;
    case AUTOSTART_HASDISK:
	if (clk > CYCLES_PER_RFSH * RFSH_PER_SEC && check("READY.", 1024+5*40))
	{
	    warn(pwarn, -1, "loading disk");
            /* FIXME */
	    orig_true1541_state = true1541_enabled;
	    if (1 /* || !app_resources.noTraps */) /* FIXME */
	    {
		if (orig_true1541_state)
		    warn(pwarn, -1, "switching true 1541 emulation off");
		settrue1541mode(0);
	    }
	    if (autostart_program_name) {
		tmp = xmalloc(strlen(autostart_program_name) + 20);
		sprintf(tmp, "load\"%s\",8,1\r", autostart_program_name);
		kbd_buf_feed(tmp);
		free(tmp);
	    }
	    else
		kbd_buf_feed("load\"*\",8,1\r");
	    autostartmode = AUTOSTART_LOADINGDISK;
	    deallocate_program_name();
	}
	break;
    case AUTOSTART_LOADINGDISK:
	if (check("LOADING", 1024+9*40) && check("READY.", 1024+10*40))
	{
            /* FIXME */
	    if (/* !app_resources.noTraps && */ orig_true1541_state)
		warn(pwarn, -1, "switching true 1541 on and starting program");
	    else
		warn(pwarn, -1, "starting program");
	    settrue1541mode(orig_true1541_state);
	    kbd_buf_feed("run\r");
	    autostartmode = AUTOSTART_DONE;
	}
	break;
      default:
        return;
    }

    if (autostartmode == AUTOSTART_ERROR)
    {
	warn(pwarn, -1, "now turning true 1541 emulation %s",
	     orig_true1541_state ? "on" : "off");
	settrue1541mode(orig_true1541_state);
    }
}

static int autostart_ignore_reset = 0;

/* Clean memory and reboot for autostart.  */
static void reboot_for_autostart(const char *program_name)
{
    warn(pwarn, -1, "rebooting...");
    mem_powerup();
    autostart_ignore_reset = 1;
    maincpu_trigger_reset();
    deallocate_program_name();
    if (program_name)
	autostart_program_name = stralloc(program_name);
}

/* ------------------------------------------------------------------------- */

/* Autostart tape image `file_name'.  */
int autostart_tape(const char *file_name, const char *program_name)
{
    if (file_name == NULL)
	return -1;

    if (serial_select_file(DT_TAPE, 1, file_name) < 0)
    {
	warn(pwarn, -1, "cannot attach file '%s'", file_name);
	autostartmode = AUTOSTART_ERROR;
	deallocate_program_name();
	return -1;
    }

    warn(pwarn, -1, "attached file `%s' as a tape image", file_name);
    autostartmode = AUTOSTART_HASTAPE;
    reboot_for_autostart(program_name);

    return 0;
}

/* Autostart disk image `file_name'.  */
int autostart_disk(const char *file_name, const char *program_name)
{
    if (file_name == NULL)
	return -1;

    if (serial_select_file(DT_DISK | DT_1541, 8, file_name) < 0)
    {
	warn(pwarn, -1, "cannot attach file `%s' as a disk image", file_name);
	autostartmode = AUTOSTART_ERROR;
	deallocate_program_name();
	return -1;
    }

    warn(pwarn, -1, "attached file `%s' as a disk image to device #8",
	 file_name);
    autostartmode = AUTOSTART_HASDISK;
    reboot_for_autostart(program_name);

    return 0;
}

/* Autostart `file_name', trying to auto-detect its type.  */
int autostart_autodetect(const char *file_name, const char *program_name)
{
    if (file_name == NULL)
	return -1;

    if (autostart_disk(file_name, program_name) == 0)
	warn(pwarn, -1, "`%s' detected as a disk image", file_name);
    else if (autostart_tape(file_name, program_name) == 0)
	warn(pwarn, -1, "`%s' detected as a tape image", file_name);
    else
    {
	warn(pwarn, -1, "type of file `%s' unrecognized", file_name);
	return -1;
    }

    return 0;
}

/* Autostart the image attached to device `num'.  */
int autostart_device(int num)
{
    switch (num) {
      case 8:
	autostartmode = AUTOSTART_HASDISK;
	break;
      case 1:
	autostartmode = AUTOSTART_HASTAPE;
	break;
      default:
	return -1;
    }

    reboot_for_autostart(NULL);
    return 0;
}

/* Disable autostart on reset.  */
void autostart_reset(void)
{
    if (!autostart_ignore_reset && autostartmode != AUTOSTART_NONE &&
	autostartmode != AUTOSTART_ERROR)
    {
	warn(pwarn, -1, "disabling autostart");
	autostartmode = AUTOSTART_NONE;
	deallocate_program_name();
    }
    autostart_ignore_reset = 0;
}


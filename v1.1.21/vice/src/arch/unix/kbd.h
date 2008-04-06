/*
 * kbd.h - X11 keyboard driver.
 *
 * Written by
 *  Jouko Valta <jopi@stekt.oulu.fi>
 *  Andre Fachat <fachat@physik.tu-chemnitz.de>
 *  Ettore Perazzoli <ettore@comm2000.it>
 *
 * This file is part of VICE, the Versatile Commodore Emulator.
 * See README file for copyright notice.
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

/* X11 keyboard driver. */

#ifndef _KBD_X_H
#define _KBD_X_H

#include <X11/Intrinsic.h>
#include "types.h"

extern BYTE joystick_value[3];

extern void kbd_event_handler(Widget w, XtPointer client_data, XEvent *report,
			      Boolean *ctd);
extern int kbd_load_keymap(const char *filename);
extern int kbd_dump_keymap(const char *filename);

extern int kbd_init(void);
extern int do_kbd_init_cmdline_options(void);
extern int do_kbd_init_resources(void);

extern int kbd_init_cmdline_options(void);
extern int kbd_init_resources(void);

extern int pet_kbd_init_cmdline_options(void);
extern int pet_kbd_init_resources(void);

extern const char **keymap_res_name_list;

extern int c64_kbd_init(void);
extern int c128_kbd_init(void);
extern int vic20_kbd_init(void);
extern int pet_kbd_init(void);
extern int c610_kbd_init(void);

typedef void (*key_ctrl_column4080_func_t) (void);
extern void kbd_register_column4080_key(key_ctrl_column4080_func_t func);

#endif


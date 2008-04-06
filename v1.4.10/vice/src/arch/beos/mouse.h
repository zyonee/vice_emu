/*
 * mouse.h - Mouse handling for BeOS.
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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#ifndef _MOUSE_H
#define _MOUSE_H

#include "types.h"

extern int mouse_init_resources(void);
extern int mouse_init_cmdline_options(void);
extern void mouse_set_format(void);
extern void mouse_update_mouse(void);
extern void mouse_init(void);
extern void mouse_update_mouse_acquire(void);
extern void mouse_set_cooperative_level(void);

extern int _mouse_enabled;
extern int _mouse_x, _mouse_y;

/* ------------------------------------------------------------------------- */

inline static BYTE mouse_get_x(void)
{
    if (!_mouse_enabled)
        return 0xff;
    return (BYTE) (_mouse_x >> 1) & 0x7e;
}

inline static BYTE mouse_get_y(void)
{
    if (!_mouse_enabled)
        return 0xff;
    return (BYTE) (~_mouse_y >> 1) & 0x7e;
}

#endif


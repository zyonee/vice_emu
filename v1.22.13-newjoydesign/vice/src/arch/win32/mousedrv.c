/*
 * mousedrv.c - Mouse handling for Win32
 *
 * Written by
 *  Tibor Biczo <crown@mail.matav.hu>
 *  Ettore Perazzoli <ettore@comm2000.it>
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

#include "vice.h"

#include <windows.h>
#include <dinput.h>
#include <stdio.h>

#include "log.h"
#include "mouse.h"
#include "mousedrv.h"
#include "types.h"
#include "ui.h"


int _mouse_x, _mouse_y;

/* ------------------------------------------------------------------------- */

void mousedrv_mouse_changed(void)
{
}

int mousedrv_resources_init(void)
{
    return 0;
}

int mousedrv_cmdline_options_init(void)
{
    return 0;
}

/* ------------------------------------------------------------------------- */

void mousedrv_init(void)
{
}

BYTE mousedrv_get_x(void)
{
    if (!_mouse_enabled)
        return 0xff;
    return (BYTE)(_mouse_x >> 1) & 0x7e;
}

BYTE mousedrv_get_y(void)
{
    if (!_mouse_enabled)
        return 0xff;
    return (BYTE)(~_mouse_y >> 1) & 0x7e;
}


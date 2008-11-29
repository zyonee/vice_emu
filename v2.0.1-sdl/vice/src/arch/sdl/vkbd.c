/*
 * vkbd.c - SDL virtual keyboard.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Mike Dawson <mike@gp2x.org>
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
#include "types.h"

#include "keyboard.h"
#include "uimenu.h"
#include "videoarch.h"
#include "vkbd.h"

#include <stdio.h>

int sdl_vkbd_state = 0;

static int vkbd_pos_x = 0;
static int vkbd_pos_y = 0;

static vkbd_t *vkbd = NULL;
static int vkbd_w = 0;
static int vkbd_h = 0;

static int vkbd_x = 0;
static int vkbd_y = 0;

/* ------------------------------------------------------------------ */
/* static functions */

static void sdl_vkbd_key_press(int value, int shift)
{
    int mr, mc;
    BYTE b, sb;

    b = vkbd->keytable[vkbd_x + vkbd_y*vkbd_w];

    if (b == 0xff) {
        return;
    }

    if (shift) {
        sb = vkbd->shift;
        mc = (int)(sb & 0xf);
        mr = (int)((sb >> 4) & 0xf);
        keyboard_set_keyarr(mr, mc, value);
    }

    mc = (int)(b & 0xf);
    mr = (int)((b >> 4) & 0xf);

    keyboard_set_keyarr(mr, mc, value);
}

/* ------------------------------------------------------------------ */
/* External interface */

void sdl_vkbd_set_vkbd(const vkbd_t *machine_vkbd)
{
    vkbd = (vkbd_t *)machine_vkbd;

    if (vkbd == NULL) {
        return;
    }

    for (vkbd_h = 0; vkbd->keyb[vkbd_h] != NULL; ++vkbd_h);

    if (vkbd_h > 0) {
        for(vkbd_w = 0; vkbd->keyb[0][vkbd_w] != 0; ++vkbd_w);
    }
}

void sdl_vkbd_activate(void)
{
    if (vkbd == NULL) {
        return;
    }

    sdl_ui_init_draw_params();
    sdl_vkbd_state = 1;
}

void sdl_vkbd_close(void)
{
    sdl_vkbd_state = 0;
}

void sdl_vkbd_draw(void)
{
    int i;

    for (i = 0; i < vkbd_h; ++i) {
        sdl_ui_print(vkbd->keyb[i], vkbd_pos_x, vkbd_pos_y + i);
    }

    sdl_ui_invert_char(vkbd_pos_x + vkbd_x, vkbd_pos_y + vkbd_y);
}

int sdl_vkbd_process(ui_menu_action_t input)
{
    int retval = 1;

    switch(input) {
        case MENU_ACTION_UP:
            --vkbd_y;
            if (vkbd_y < 0) {
                vkbd_y = vkbd_h - 1;
            }
            break;
        case MENU_ACTION_DOWN:
            ++vkbd_y;
            if (vkbd_y >= vkbd_h) {
                vkbd_y = 0;
            }
            break;
        case MENU_ACTION_LEFT:
            --vkbd_x;
            if (vkbd_x < 0) {
                vkbd_x = vkbd_w - 1;
            }
            break;
        case MENU_ACTION_RIGHT:
            ++vkbd_x;
            if (vkbd_x >= vkbd_w) {
                vkbd_x = 0;
            }
            break;
        case MENU_ACTION_VKBD:
            sdl_vkbd_close();
            retval = 0;
            break;
        case MENU_ACTION_SELECT:
            sdl_vkbd_key_press(1, 0);
            break;
        case MENU_ACTION_SELECT_RELEASE:
            sdl_vkbd_key_press(0, 0);
            break;
        case MENU_ACTION_CANCEL:
            sdl_vkbd_key_press(1, 1);
            break;
        case MENU_ACTION_CANCEL_RELEASE:
            sdl_vkbd_key_press(0, 1);
            break;
        default:
            retval = 0;
            break;
    }

    return retval;
}

/* ------------------------------------------------------------------ */
/* virtual keyboards */

static const char *keyb_c64[] = {
    "  \x1f 1234567890+-\x1ch del  F1",
    "ctrl QWERTYUIOP@*\x1e rstr F3",
    "r/s   ASDFGHJKL:;= rtrn F5",
    "c=     ZXCVBNM,./v>     F7",
    "sh      space           sh",
    NULL
};

static const BYTE keytable_c64[] =
    "\xff\xff\x71\xff\x70\x73\x10\x13\x20\x23\x30\x33\x40\x43\x50\x53\x60\x63\xff\x00\x00\x00\xff\xff\x04\x04"
    "\x72\x72\x72\x72\xff\x76\x11\x16\x21\x26\x31\x36\x41\x46\x51\x56\x61\x66\xff\xee\xee\xee\xee\xff\x05\x05"
    "\x77\x77\x77\xff\xff\xff\x12\x15\x22\x25\x32\x35\x42\x45\x52\x55\x62\x65\xff\x01\x01\x01\x01\xff\x06\x06"
    "\x75\x75\xff\xff\xff\xff\xff\x14\x27\x24\x37\x34\x47\x44\x57\x54\x67\x07\x02\xff\xff\xff\xff\xff\x03\x03"
    "\x17\x17\xff\xff\xff\xff\xff\xff\x74\x74\x74\x74\x74\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x64\x64";

vkbd_t vkbd_c64 = {
    keyb_c64,
    keytable_c64,
    '\x17'
};


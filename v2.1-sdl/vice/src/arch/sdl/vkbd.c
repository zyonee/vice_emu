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

#include <stdio.h>

#include "videoarch.h"
#include "joy.h"
#include "keyboard.h"
#include "menu_common.h"
#include "types.h"
#include "ui.h"
#include "uimenu.h"
#include "uipoll.h"
#include "vkbd.h"

int sdl_vkbd_state = 0;

/* ------------------------------------------------------------------ */
/* static functions/variables */

static int vkbd_pos_x = 0;
static int vkbd_pos_y = 0;
static int vkbd_pos_max_x = 0;
static int vkbd_pos_max_y = 0;

static vkbd_t *vkbd = NULL;
static int vkbd_w = 0;
static int vkbd_h = 0;

static int vkbd_x = 0;
static int vkbd_y = 0;

static int vkbd_move = 0;

static int vkbd_shiftflags;

static UI_MENU_CALLBACK(custom_shift_callback)
{
    int flag = (1 << ((int)param));

    if (activated) {
        vkbd_shiftflags ^= flag;
    } else {
        if (vkbd_shiftflags & flag) {
            return sdl_menu_text_tick;
        }
    }
    return NULL;
}

static const ui_menu_entry_t define_shift_options_menu[] = {
    { "Virtual shift",
      MENU_ENTRY_OTHER,
      custom_shift_callback,
      (ui_callback_data_t)0 },
    { "Left shift",
      MENU_ENTRY_OTHER,
      custom_shift_callback,
      (ui_callback_data_t)1 },
    { "Right shift",
      MENU_ENTRY_OTHER,
      custom_shift_callback,
      (ui_callback_data_t)2 },
    { "Allow shift",
      MENU_ENTRY_OTHER,
      custom_shift_callback,
      (ui_callback_data_t)3 },
    { "Deshift shift",
      MENU_ENTRY_OTHER,
      custom_shift_callback,
      (ui_callback_data_t)4 },
    { "Allow other",
      MENU_ENTRY_OTHER,
      custom_shift_callback,
      (ui_callback_data_t)5 },
    { "Alt map",
      MENU_ENTRY_OTHER,
      custom_shift_callback,
      (ui_callback_data_t)8 },
    { NULL }
};

static const ui_menu_entry_t shift_menu[] = {
    { "Define shift options",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)define_shift_options_menu },
};

#define VKBD_COMMAND_MOVE  0xff
#define VKBD_COMMAND_CLOSE 0xfe

static void sdl_vkbd_key_press(int value, int shift)
{
    int mr, mc, neg;
    BYTE b, sb;

    b = vkbd->keytable[vkbd_x + vkbd_y*vkbd_w];

    if ((b == VKBD_COMMAND_CLOSE)&&(value)) {
        sdl_vkbd_close();
        return;
    }

    if (b == VKBD_COMMAND_MOVE) {
        if (value && shift) {
            sdl_vkbd_close();
        } else {
            vkbd_move = value;
        }
        return;
    }

    neg = b & 0x80;

    if (shift && !neg) {
        sb = vkbd->shift;
        mc = (int)(sb & 0xf);
        mr = (int)((sb >> 4) & 0xf);
        keyboard_set_keyarr(mr, mc, value);
    }

    mc = (int)(b & 0xf);
    mr = (int)((b >> 4) & 0x7);

    if (neg) {
        mr = -mr;
    }

    keyboard_set_keyarr_any(mr, mc, value);
}

static void sdl_vkbd_key_map(void)
{
    int mr, mc, neg, i, j;
    BYTE b;
    SDL_Event e;
    int unmap = 0;
    char keyname[10];

    b = vkbd->keytable[vkbd_x + vkbd_y*vkbd_w];

    if (b == VKBD_COMMAND_CLOSE) {
        return;
    }

    sdl_ui_activate_pre_action();

    /* Use blank for unmapping */
    if (b == VKBD_COMMAND_MOVE) {
        unmap = 1;
    } else {
        /* get the key name for displaying */
        for(j = vkbd_x; (j > -1) && (vkbd->keytable[j + vkbd_y*vkbd_w] == b); --j);
        ++j;

        for(i = 0; ((i + j) < vkbd_w) && (vkbd->keytable[i + j + vkbd_y*vkbd_w] == b); ++i) {
            keyname[i] = vkbd->keyb[vkbd_y][i + j];
        }

        keyname[i] = 0;
    }

    neg = b & 0x80;
    mc = (int)(b & 0xf);
    mr = (int)((b >> 4) & 0x7);

    if (neg) {
        mr = -mr;
    }

    e = sdl_ui_poll_event("key or joystick event", unmap?"(unmap)":keyname, SDL_POLL_KEYBOARD | SDL_POLL_MODIFIER | SDL_POLL_JOYSTICK, 5);

    /* TODO check if key/event is suitable */
    switch (e.type) {
        case SDL_KEYDOWN:
            /* TODO unmap */
            if (!unmap) {
                vkbd_shiftflags = 0;
                sdl_ui_external_menu_activate((ui_menu_entry_t *)shift_menu);
                keyboard_set_map_any((signed long)e.key.keysym.sym, mr, mc, vkbd_shiftflags);
            }
            break;
        case SDL_JOYAXISMOTION:
        case SDL_JOYBUTTONDOWN:
        case SDL_JOYHATMOTION:
            if(unmap) {
                sdljoy_unset(e);
            } else {
                sdljoy_set_keypress(e, mr, mc);
            }
            break;
        default:
            break;
    }

    sdl_ui_activate_post_action();
    sdl_vkbd_state = SDL_VKBD_ACTIVE;
}

static inline void sdl_vkbd_move(int *var, int amount, int min, int max)
{
    *var += amount;

    if (*var < min) {
        *var = max - 1;
    } else if (*var >= max) {
        *var = min;
    }

    sdl_vkbd_state |= SDL_VKBD_REPAINT;
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
        for (vkbd_w = 0; vkbd->keyb[0][vkbd_w] != 0; ++vkbd_w);
    }
}

void sdl_vkbd_activate(void)
{
    menu_draw_t *limits = NULL;

    if (vkbd == NULL) {
        return;
    }

    sdl_ui_init_draw_params();
    limits = sdl_ui_get_menu_param();
    vkbd_pos_max_x = limits->max_text_x - vkbd_w + 1;
    vkbd_pos_max_y = limits->max_text_y - vkbd_h + 1;

    sdl_vkbd_state = SDL_VKBD_ACTIVE | SDL_VKBD_REPAINT;
    vkbd_move = 0;
}

void sdl_vkbd_close(void)
{
    keyboard_key_clear();
    sdl_vkbd_state = SDL_VKBD_REPAINT;
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
            if (vkbd_move) {
                sdl_vkbd_move(&vkbd_pos_y, -1, 0, vkbd_pos_max_y);
            } else {
                sdl_vkbd_move(&vkbd_y, -1, 0, vkbd_h);
            }
            break;
        case MENU_ACTION_DOWN:
            if (vkbd_move) {
                sdl_vkbd_move(&vkbd_pos_y, 1, 0, vkbd_pos_max_y);
            } else {
                sdl_vkbd_move(&vkbd_y, 1, 0, vkbd_h);
            }
            break;
        case MENU_ACTION_LEFT:
            if (vkbd_move) {
                sdl_vkbd_move(&vkbd_pos_x, -1, 0, vkbd_pos_max_x);
            } else {
                sdl_vkbd_move(&vkbd_x, -1, 0, vkbd_w);
            }
            break;
        case MENU_ACTION_RIGHT:
            if (vkbd_move) {
                sdl_vkbd_move(&vkbd_pos_x, 1, 0, vkbd_pos_max_x);
            } else {
                sdl_vkbd_move(&vkbd_x, 1, 0, vkbd_w);
            }
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
        case MENU_ACTION_MAP:
            sdl_vkbd_key_map();
            retval = 0;
            break;
        case MENU_ACTION_EXIT:
            sdl_vkbd_close();
            retval = 0;
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
    "X \x1f 1234567890+-\x1ch del  F1",
    "ctrl QWERTYUIOP@*\x1e rstr F3",
    "r/s   ASDFGHJKL:;= rtrn F5",
    "c=     ZXCVBNM,./v>     F7",
    "sh      space           sh",
    NULL
};

static const BYTE keytable_c64[] =
    "\xfe\xff\x71\xff\x70\x73\x10\x13\x20\x23\x30\x33\x40\x43\x50\x53\x60\x63\xff\x00\x00\x00\xff\xff\x04\x04"
    "\x72\x72\x72\x72\xff\x76\x11\x16\x21\x26\x31\x36\x41\x46\x51\x56\x61\x66\xff\xb0\xb0\xb0\xb0\xff\x05\x05"
    "\x77\x77\x77\xff\xff\xff\x12\x15\x22\x25\x32\x35\x42\x45\x52\x55\x62\x65\xff\x01\x01\x01\x01\xff\x06\x06"
    "\x75\x75\xff\xff\xff\xff\xff\x14\x27\x24\x37\x34\x47\x44\x57\x54\x67\x07\x02\xff\xff\xff\xff\xff\x03\x03"
    "\x17\x17\xff\xff\xff\xff\xff\xff\x74\x74\x74\x74\x74\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x64\x64";

static const BYTE keytable_vic20[] =
    "\xfe\xff\x01\xff\x00\x07\x10\x17\x20\x27\x30\x37\x40\x47\x50\x57\x60\x67\xff\x70\x70\x70\xff\xff\x74\x74"
    "\x02\x02\x02\x02\xff\x06\x11\x16\x21\x26\x31\x36\x41\x46\x51\x56\x61\x66\xff\xb0\xb0\xb0\xb0\xff\x75\x75"
    "\x03\x03\x03\xff\xff\xff\x12\x15\x22\x25\x32\x35\x42\x45\x52\x55\x62\x65\xff\x71\x71\x71\x71\xff\x76\x76"
    "\x05\x05\xff\xff\xff\xff\xff\x14\x23\x24\x33\x34\x43\x44\x53\x54\x63\x73\x72\xff\xff\xff\xff\xff\x77\x77"
    "\x13\x13\xff\xff\xff\xff\xff\xff\x04\x04\x04\x04\x04\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\xff\x64\x64";

vkbd_t vkbd_c64 = {
    keyb_c64,
    keytable_c64,
    '\x17'
};

vkbd_t vkbd_vic20 = {
    keyb_c64,
    keytable_vic20,
    '\x13'
};


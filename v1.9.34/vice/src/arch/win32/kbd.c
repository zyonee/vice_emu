/*
 * kbd.c - Keyboard emulation.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Tibor Biczo <crown@mail.matav.hu>
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

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include "kbd.h"
#include "cmdline.h"
#include "keyboard.h"
#include "resources.h"
#include "joy.h"
#include "machine.h"
#include "types.h"
#include "utils.h"

/* ------------------------------------------------------------------------ */

/* #define DEBUG_KBD */

/* Debugging stuff.  */
#ifdef DEBUG_KBD
static void kbd_debug(const char *format, ...)
{
    char tmp[1024];
    va_list args;

    va_start(args, format);
    vsprintf(tmp, format, args);
    va_end(args);
    OutputDebugString(tmp);
    printf(tmp);
}
#define KBD_DEBUG(x) kbd_debug x
#else
#define KBD_DEBUG(x)
#endif

/* ------------------------------------------------------------------------ */

BYTE _kbd_extended_key_tab[256] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, K_KPENTER, K_RIGHTCTRL, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, K_KPDIV, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, K_NUMLOCK, 0, K_HOME, K_UP, K_PGUP, 0, K_LEFT, 0, K_RIGHT, 0, K_END,
    K_DOWN, K_PGDOWN, K_INS, K_DEL, 0, 0, 0, 0, 0, 0, 0, K_LEFTW95, K_RIGHTW95, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

BYTE joystick_value[3];

/* 40/80 column key.  */
static key_ctrl_column4080_func_t key_ctrl_column4080_func = NULL;

/* CAPS key.  */
static key_ctrl_caps_func_t key_ctrl_caps_func = NULL;

struct _convmap {
    /* Conversion map.  */
    keyconv *map;
    /* Location of the virtual shift key in the keyboard matrix.  */
    int virtual_shift_row, virtual_shift_column;
};

static struct _convmap *keyconvmaps;
static struct _convmap *keyconv_base;
static int num_keyconvmaps;

static int keymap_index;

/* FIXME: Ugly hack.  */
//extern keyconv c64_keyboard[];
//keyconv *keyconvmap = c64_keyboard;

/* What is the location of the virtual shift key in the keyboard matrix?  */
//static int virtual_shift_column = 7;
//static int virtual_shift_row = 1;

/* ------------------------------------------------------------------------ */

int kbd_init(int num, ...)
{
    KBD_DEBUG(("Allocating keymaps"));
    keyconvmaps = (struct _convmap*)xmalloc(num * sizeof(struct _convmap));
    KBD_DEBUG(("Installing keymaps"));
    {
        va_list p;
        int i;

        num_keyconvmaps = num;

        va_start(p, num);
        for (i = 0; i < num_keyconvmaps; i++) {
            keyconv *map;
            unsigned int sizeof_map;
            int shift_row, shift_column;

            shift_row = va_arg(p, int);
            shift_column = va_arg(p, int);
            map = va_arg(p, keyconv *);
            sizeof_map=va_arg(p,unsigned int);

            keyconvmaps[i].map = map;
            keyconvmaps[i].virtual_shift_row = shift_row;
            keyconvmaps[i].virtual_shift_column = shift_column;
        }
    }
    keyconv_base=&keyconvmaps[keymap_index>>1];
    return 0;
}

static int set_keymap_index(resource_value_t v, void *param)
{
int real_index;

    keymap_index = (int)v;
    real_index = keymap_index >> 1;
    keyconv_base = &keyconvmaps[real_index];

    return 0;
}

static resource_t resources[] = {
    { "KeymapIndex", RES_INTEGER, (resource_value_t)0,
      (resource_value_t *)&keymap_index, set_keymap_index, NULL },
    { NULL }
};

int kbd_resources_init(void)
{
    return resources_register(resources);
}

static cmdline_option_t cmdline_options[] = {
    { "-keymap", SET_RESOURCE, 1, NULL, NULL, "KeymapIndex", NULL,
      "<number>", "Specify index of used keymap" },
    { NULL },
};

int kbd_cmdline_options_init(void)
{
    return cmdline_register_options(cmdline_options);
}

/* ------------------------------------------------------------------------ */

/* Windows would not want us to handle raw scancodes like this...  But we
   need it nevertheless.  */

int kbd_handle_keydown(DWORD virtual_key, DWORD key_data)
{
    int kcode = (key_data >> 16) & 0xff;

    /*  Translate Extended scancodes */
    if (key_data & (1 << 24)) {
        kcode=_kbd_extended_key_tab[kcode];
    }

#ifndef COMMON_KBD
    /* FIXME: We should read F4, F7 and PGUP from the the *.vkm.  */
    if (kcode == K_F7) {
        if (key_ctrl_column4080_func != NULL) {
            key_ctrl_column4080_func();
        }
    }

    if (kcode == K_F4) {
        if (key_ctrl_caps_func != NULL) {
            key_ctrl_caps_func();
        }
    }

    if (kcode == K_PGUP) {
        machine_set_restore_key(1);
    }

    KBD_DEBUG(("Keydown, code %d (0x%02x)\n", kcode, kcode));
    if (!joystick_handle_key(kcode, 1)) {
        keyboard_set_keyarr(keyconv_base->map[kcode].row,
                   keyconv_base->map[kcode].column, 1);
        if (keyconv_base->map[kcode].vshift)
            keyboard_set_keyarr(keyconv_base->virtual_shift_row,
                                keyconv_base->virtual_shift_column, 1);
    }
#else
    if (!joystick_handle_key(kcode, 1))
        keyboard_key_pressed((signed long)kcode);
#endif

    return 0;
}

int kbd_handle_keyup(DWORD virtual_key, DWORD key_data)
{
    int kcode = (key_data >> 16) & 0xff;

    /*  Translate Extended scancodes */
    if (key_data & (1 << 24)) {
        kcode=_kbd_extended_key_tab[kcode];
    }

#ifndef COMMON_KBD
    if (kcode==K_PGUP) {
        machine_set_restore_key(0);
    }

    if (!joystick_handle_key(kcode, 0)) {
        keyboard_set_keyarr(keyconv_base->map[kcode].row,
                            keyconv_base->map[kcode].column, 0);
        if (keyconv_base->map[kcode].vshift)
            keyboard_set_keyarr(keyconv_base->virtual_shift_row,
                                keyconv_base->virtual_shift_column, 0);
    }
#else
    if (!joystick_handle_key(kcode, 0))
        keyboard_key_released((signed long)kcode);
#endif

    return 0;
}

const char *kbd_code_to_string(kbd_code_t kcode)
{
    static char *tab[256] = {
        "None", "Esc", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-",
        "=", "Backspace", "Tab", "Q", "W", "E", "R", "T", "Y", "U", "I", "O",
        "P", "{", "}", "Enter", "Left Ctrl", "A", "S", "D", "F", "G", "H", "J",
        "K", "L", ";", "'", "`", "Left Shift", "\\", "Z", "X", "C", "V", "B",
        "N", "M", ",", ".", "/", "Right Shift", "Numpad *", "Left Alt",
        "Space", "Caps Lock", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8",
        "F9", "F10", "Num Lock", "Scroll Lock", "Numpad 7", "Numpad 8",
        "Numpad 9", "Numpad -", "Numpad 4", "Numpad 5", "Numpad 6",
        "Numpad +", "Numpad 1", "Numpad 2", "Numpad 3", "Numpad 0",
        "Numpad .", "SysReq", "85", "86", "F11", "F12", "Home",
        "Up", "PgUp", "Left", "Right", "End", "Down", "PgDown", "Ins", "Del",
        "Numpad Enter", "Right Ctrl", "Pause", "PrtScr", "Numpad /",
        "Right Alt", "Break", "Left Win95", "Right Win95"
    };

    return tab[(int) kcode];
}

/* ------------------------------------------------------------------------ */
#ifndef COMMON_KBD
void keyboard_register_column4080_key(key_ctrl_column4080_func_t func)
{
    key_ctrl_column4080_func = func;
}

void keyboard_register_caps_key(key_ctrl_caps_func_t func)
{
    key_ctrl_caps_func = func;
}
#endif

#ifdef COMMON_KBD
void kbd_arch_init(void)
{
}

signed long kbd_arch_keyname_to_keynum(char *keyname)
{
    return (signed long)atoi(keyname);
}

const char *kbd_arch_keynum_to_keyname(signed long keynum)
{
    static char keyname[20];

    memset(keyname, 0, 20);

    sprintf(keyname, "%li", keynum);

    return keyname;
}
#endif


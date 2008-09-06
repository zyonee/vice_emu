/*
 * kbd.c - SDL keyboard driver.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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

#include <SDL/SDL.h>

#include "kbd.h"
#include "fullscreenarch.h"
#include "keyboard.h"
#include "log.h"
#include "machine.h"
#include "monitor.h"
#include "resources.h"
#include "ui.h"
#include "uimenu.h"

int sdl_ui_menukeys[MENU_ACTION_NUM];

/* UI hotkeys: index is the key(combo), value is a pointer to the menu item.
   5 is the number of the supported modifiers: none, shift, alt, control, meta. */
#define SDLKBD_UI_HOTKEYS_MAX (SDLK_LAST * 5)
ui_menu_entry_t *sdlkbd_ui_hotkeys[SDLKBD_UI_HOTKEYS_MAX];

/* ------------------------------------------------------------------------ */

static inline int sdlkbd_key_mod_to_index(SDLKey key, SDLMod mod)
{
#if 0
    int i;
    Uint16 mask;

    /* Combine left&right shift/alt/meta/ctrl */
    mod = (mod & 0x541) | ((mod >> 1) & 0x541);

    /* shift = 0x01, alt = 0x02, ctrl = 0x40, meta = 0x08 */
    mod = (mod & 0x041) | ((mod & 0x500)>>7);

    /* shift = 0x01, alt = 0x02, ctrl = 0x04, meta = 0x08 */
    mod = (mod & 0xb) | ((mod & 0x40)>>4);

    /*  shift = 1, alt = 2, ctrl = 3, meta = 4 */
    for(i = 4, mask = 0x8; mask > 0; mask >>= 1, --i) {
        if(mod & mask) {
            break;
        }
    }
#else
    int i = 0;

    mod &= (KMOD_CTRL|KMOD_SHIFT|KMOD_ALT|KMOD_META);

    if(mod) {
        if (mod & KMOD_SHIFT) {
            i = 1;
        } else if (mod & KMOD_ALT) {
            i = 2;
        } else if (mod & KMOD_CTRL) {
            i = 3;
        } else if (mod & KMOD_META) {
            i = 4;
        }
    }
#endif
    return i * SDLK_LAST + key;
}

ui_menu_entry_t *sdlkbd_get_hotkey(SDLKey key, SDLMod mod)
{
    return sdlkbd_ui_hotkeys[sdlkbd_key_mod_to_index(key, mod)];
}

void sdlkbd_set_hotkey(SDLKey key, SDLMod mod, ui_menu_entry_t *value)
{
    sdlkbd_ui_hotkeys[sdlkbd_key_mod_to_index(key, mod)] = value;
}

/* ------------------------------------------------------------------------ */

ui_menu_action_t sdlkbd_press(SDLKey key, SDLMod mod)
{
    ui_menu_action_t i, retval = MENU_ACTION_NONE;
    ui_menu_entry_t *hotkey_action = NULL;

/*fprintf(stderr,"%s: %i (%s),%i\n",__func__,key,SDL_GetKeyName(key),mod);*/
    if(sdl_menu_state) {
        if(key != SDLK_UNKNOWN) {
            for(i = MENU_ACTION_UP; i < MENU_ACTION_NUM; ++i) {
                if(sdl_ui_menukeys[i] == (int)key) {
                    retval = i;
                    break;
                }
            }
        }
        return retval;
    }

    if((int)(key) == sdl_ui_menukeys[0]) {
        sdl_ui_activate();
        return retval;
    }

    if((hotkey_action = sdlkbd_get_hotkey(key, mod)) != NULL) {
        sdl_ui_menu_item_activate(hotkey_action);
        return retval;
    }

    keyboard_key_pressed((unsigned long)key);
    return retval;
}

void sdlkbd_release(SDLKey key, SDLMod mod)
{
/*fprintf(stderr,"%s: %i (%s),%i\n",__func__,key,SDL_GetKeyName(key),mod);*/
    keyboard_key_released((unsigned long)key);
}

/* ------------------------------------------------------------------------ */

void kbd_arch_init(void)
{
    int i;
fprintf(stderr,"%s\n",__func__);
    for(i = 0; i < SDLKBD_UI_HOTKEYS_MAX; ++i) {
        sdlkbd_ui_hotkeys[i] = NULL;
    }
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

void kbd_initialize_numpad_joykeys(int* joykeys)
{
    joykeys[0] = SDLK_KP0;
    joykeys[1] = SDLK_KP1;
    joykeys[2] = SDLK_KP2;
    joykeys[3] = SDLK_KP3;
    joykeys[4] = SDLK_KP4;
    joykeys[5] = SDLK_KP6;
    joykeys[6] = SDLK_KP7;
    joykeys[7] = SDLK_KP8;
    joykeys[8] = SDLK_KP9;
}


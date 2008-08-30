/*
 * kbd.c - MS-DOS keyboard driver.
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

/* ------------------------------------------------------------------------ */

ui_menu_action_t sdlkbd_press(SDLKey key, SDLMod mod)
{
    ui_menu_action_t retval = MENU_ACTION_NONE;
/*fprintf(stderr,"%s: %i (%s),%i\n",__func__,key,SDL_GetKeyName(key),mod);*/
    if(sdl_menu_state) {
        switch(key) {
            case SDLK_w:
                retval = MENU_ACTION_UP;
                break;
            case SDLK_s:
                retval = MENU_ACTION_DOWN;
                break;
            case SDLK_a:
                retval = MENU_ACTION_CANCEL;
                break;
            case SDLK_d:
                retval = MENU_ACTION_SELECT;
                break;
            case SDLK_q:
                retval = MENU_ACTION_EXIT;
                break;
            default:
                break;
        }
        return retval;
    }

    if(mod & KMOD_LALT) {
        switch(key) {
            case SDLK_q:
                exit(0);
                break;
            case SDLK_d:
                resources_set_int("viciifullscreen",!fullscreen_is_enabled);
                break;
            case SDLK_h:
                if (!ui_emulation_is_paused()) {
                    monitor_startup_trap();
                } else {
                    monitor_startup();
                }
                return;
                break;
            case SDLK_z:
                sdl_ui_activate();
                break;
            default:
                break;
        }
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
fprintf(stderr,"%s\n",__func__);
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


/*
 * kbd.c - MS-DOS keyboard driver.
 *
 * Written by
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

#include "kbd.h"
#include "keyboard.h"
#include "log.h"
#include "machine.h"
#include "types.h"

#include <SDL/SDL.h>

/* ------------------------------------------------------------------------ */

void sdlkbd_press(SDLKey key, SDLMod mod)
{
/*fprintf(stderr,"%s: %i (%s),%i\n",__func__,key,SDL_GetKeyName(key),mod);*/
    keyboard_key_pressed((unsigned long)key);
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


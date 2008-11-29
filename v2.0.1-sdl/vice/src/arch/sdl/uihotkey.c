/*
 * uihotkey.c - UI hotkey functions.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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

#include "joy.h"
#include "lib.h"
#include "kbd.h"
#include "uihotkey.h"
#include "uimenu.h"
#include "util.h"

#include <SDL/SDL.h>

/* ------------------------------------------------------------------ */
/* static functions */

static char* sdl_ui_hotkey_path_find(ui_menu_entry_t *action, ui_menu_entry_t *menu)
{
    char *p = NULL;
    char *q = NULL;

    while(menu->string) {
        if(menu == action) {
            return util_concat(menu->string, NULL);
        }
        if((menu->type) == MENU_ENTRY_SUBMENU) {
            p = sdl_ui_hotkey_path_find(action, (ui_menu_entry_t *)menu->data);
            if(p) {
                q = util_concat(menu->string, SDL_UI_HOTKEY_DELIM, p, NULL);
                lib_free(p);
                return q;
            }
        }
        ++menu;
    }
    return NULL;
}

static inline int is_not_modifier(SDLKey k)
{
    return ((k != SDLK_RSHIFT) &&
            (k != SDLK_LSHIFT) &&
            (k != SDLK_RCTRL) &&
            (k != SDLK_LCTRL) &&
            (k != SDLK_RALT) &&
            (k != SDLK_LALT))
            ?1:0;
}

/* ------------------------------------------------------------------ */
/* External interface */

char *sdl_ui_hotkey_path(ui_menu_entry_t *action)
{
    return sdl_ui_hotkey_path_find(action, sdl_ui_get_main_menu());
}

ui_menu_entry_t *sdl_ui_hotkey_action(char *path)
{
    ui_menu_entry_t *menupos = sdl_ui_get_main_menu();
    char *p;

    p = strtok(path, SDL_UI_HOTKEY_DELIM);

    if(p == NULL) {
        return NULL;
    }

    while(menupos->string) {
        if(strcmp(p, menupos->string) == 0) {
            p = strtok(NULL, SDL_UI_HOTKEY_DELIM);
            if(p == NULL) {
                return menupos;
            } else {
                if(menupos->type == MENU_ENTRY_SUBMENU) {
                    menupos = (ui_menu_entry_t *)menupos->data;
                } else {
                    return NULL;
                }
            }
        } else {
            ++menupos;
        }
    }
    return NULL;
}

int sdl_ui_hotkey_map(ui_menu_entry_t *item)
{
    SDL_Event e;
    int polling = 1;

    if(item == NULL) {
        return -1;
    }

    /* Use text item for unsetting hotkeys */
    if(item->type == MENU_ENTRY_TEXT) {
        item = NULL;
    }

    sdl_ui_clear();
    sdl_ui_print("Polling hotkey for:", 0, 0);
    sdl_ui_print(item?item->string:"(unmap hotkey)", 0, 1);
    sdl_ui_refresh();

    /* TODO check if key/event is suitable */
    while(polling) {
        while(SDL_PollEvent(&e)) {
            switch(e.type) {
                case SDL_QUIT:
                    exit(0);
                    break;
                case SDL_KEYDOWN:
                    if(is_not_modifier(e.key.keysym.sym)) {
                        sdlkbd_set_hotkey(e.key.keysym.sym, e.key.keysym.mod, item);
                        polling = 0;
                    }
                    break;
                case SDL_JOYBUTTONDOWN:
                    sdljoy_set_hotkey(e, item);
                    polling = 0;
                    break;
                case SDL_JOYAXISMOTION:
                    if(sdljoy_check_axis_movement(e) != 0) {
                        sdljoy_set_hotkey(e, item);
                        polling = 0;
                    }
                    break;
                case SDL_JOYHATMOTION:
                    break;
                default:
                    break;
            }
        }
        SDL_Delay(20);
    }
    return 1;
}


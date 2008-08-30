/*
 * uimenu.c - Common SDL menu functions.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *
 * Based on code by
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
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "vice.h"
#include "types.h"

#include "interrupt.h"
#include "resources.h"
#include "ui.h"
#include "uimenu.h"
#include "vsync.h"

#include <SDL/SDL.h>
#include <stdio.h>

int sdl_menu_state = 0;

static ui_menu_entry_t *main_menu = NULL;
static int max_onscreen_items = 10;

/* ------------------------------------------------------------------ */

static void sdl_ui_clear(void)
{
    fprintf(stderr,"\n\n");
}


static void sdl_ui_display_title(const char *title)
{
    fprintf(stderr,"%s\n",title);
}


static void sdl_ui_display_item(ui_menu_entry_t *item, int y_pos)
{

    if(((item->string == NULL) || (item->type == MENU_ENTRY_SEPARATOR))) {
        fprintf(stderr,"null or separator %i\n", y_pos);
        return;
    }

    fprintf(stderr,"%i: %s", y_pos, item->string);

    switch(item->type) {
        case MENU_ENTRY_RESOURCE_TOGGLE:
        case MENU_ENTRY_RESOURCE_RADIO:
            fprintf(stderr,"\t%s", item->callback(0, item->callback_data));
            break;
        default:
            break;
    }
    fprintf(stderr,"\n");
}

static void sdl_ui_display_cursor(int pos)
{
    fprintf(stderr,"cursor: %i\n",pos);
}

static ui_menu_action_t sdl_ui_menu_poll_input(void)
{
    ui_menu_action_t retval;
    do {
        SDL_Delay(20);
        retval = ui_dispatch_events();
    } while (retval == MENU_ACTION_NONE);
    return retval;
}


static void sdl_ui_menu_redraw(ui_menu_entry_t *menu, const char* title, int num_items)
{
    int i;

    sdl_ui_clear();
    sdl_ui_display_title(title);

    for(i=0; i<num_items; ++i) {
        if(num_items == max_onscreen_items) {
            break;
        }
        sdl_ui_display_item(&(menu[i]), i);
    }
}

static int sdl_ui_menu_display(ui_menu_entry_t *menu, const char* title)
{
    int num_items = 0, cur = 0, cur_offset = 0, in_menu = 1;


    while(menu[num_items].string != NULL) {
        ++num_items;
    }

    sdl_ui_menu_redraw(menu, title, num_items);

    while(in_menu) {
        sdl_ui_display_cursor(cur - cur_offset);

        switch(sdl_ui_menu_poll_input()) {
            case MENU_ACTION_UP:
                if(cur > 0) {
                    --cur;
                }
                break;
            case MENU_ACTION_DOWN:
                if(cur < (num_items-1)) {
                    ++cur;
                }
                break;
            case MENU_ACTION_SELECT:
                if(sdl_ui_menu_item_activate(&(menu[cur]))) {
                    sdl_ui_menu_redraw(menu, title, num_items);
                }
                break;
            case MENU_ACTION_CANCEL:
                return 0;
                break;
            case MENU_ACTION_EXIT:
                in_menu = 0;
                break;
            default:
                SDL_Delay(10);
                break;
        }
    }

    return 0;
}

static void sdl_ui_trap(WORD addr, void *data)
{
    vsync_suspend_speed_eval();
    sdl_menu_state = 1;
    sdl_ui_menu_display(main_menu, "VICE main menu");
    sdl_menu_state = 0;
}

/* ------------------------------------------------------------------ */

int sdl_ui_menu_item_activate(ui_menu_entry_t *item)
{
    switch(item->type) {
        case MENU_ENTRY_OTHER:
        case MENU_ENTRY_DIALOG:
        case MENU_ENTRY_RESOURCE_TOGGLE:
        case MENU_ENTRY_RESOURCE_RADIO:
        case MENU_ENTRY_RESOURCE_INT:
        case MENU_ENTRY_RESOURCE_STRING:
            item->callback(1, item->callback_data);
            return 1;
            break;
        case MENU_ENTRY_SUBMENU:
            sdl_ui_menu_display(item->sub_menu, item->string);
            return 1;
            break;
        default:
            break;
    }
    return 0;
}

void sdl_ui_set_main_menu(ui_menu_entry_t *menu)
{
    main_menu = menu;
}

void sdl_ui_activate(void)
{
    interrupt_maincpu_trigger_trap(sdl_ui_trap, 0);
}

const char *sdl_ui_menu_toggle_helper(int activated, const char *resource_name)
{
    int value, r;

    if (activated) {
        r = resources_toggle(resource_name, &value);
        if (r < 0)
            r = resources_get_int(resource_name, &value);
    } else
        r = resources_get_int(resource_name, &value);

    if (r < 0)
        return "Unknown";
    else
        return value ? "On " : "Off";
}

const char *sdl_ui_menu_radio_helper(int activated, ui_callback_data_t param, const char *resource_name)
{
    if (activated) {
        resources_set_value(resource_name, (resource_value_t)param);
    } else {
        resource_value_t v;
        resources_get_value(resource_name, (void *)&v);
        if (v == (resource_value_t)param)
            return "M";
    }
    return NULL;
}


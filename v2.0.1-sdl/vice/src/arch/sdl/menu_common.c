/*
 * menu_common.c - Common SDL menu functions.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#include <stdlib.h>

#include "autostart.h"
#include "lib.h"
#include "menu_common.h"
#include "monitor.h"
#include "resources.h"
#include "uifilereq.h"
#include "uimenu.h"


/* ------------------------------------------------------------------ */
/* Common strings */

const char* sdl_menu_text_tick = "*";
const char* sdl_menu_text_unknown = "?";
const char* sdl_menu_text_exit_ui = "\1";

/* ------------------------------------------------------------------ */
/* Common callbacks */

UI_MENU_CALLBACK(autostart_callback)
{
    char *name = NULL;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Choose autostart image", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            if (autostart_autodetect(name, NULL, 0, AUTOSTART_MODE_RUN) < 0) {
                /* TODO:
                   to be replaced with ui_error */
                fprintf(stderr, "could not start auto-image\n");
            }
            lib_free(name);
            return sdl_menu_text_exit_ui;
        }
    }
    return NULL;
}

UI_MENU_CALLBACK(pause_callback)
{
    return NULL;
}

UI_MENU_CALLBACK(monitor_callback)
{
    if(activated) {
        monitor_startup();
    }
    return NULL;
}

UI_MENU_CALLBACK(quit_callback)
{
    if(activated) {
        exit(0);
    }
    return NULL;
}

UI_MENU_CALLBACK(save_settings_callback)
{
    if(activated) {
        if (resources_save(NULL) < 0) {
            ui_error("Cannot save current settings.");
        } else { 
            ui_message("Settings saved.");
        }

        /* TODO:
           to be uncommented after the fliplist menus have been made
        uifliplist_save_settings(); */

        return sdl_menu_text_exit_ui;
    }
    return NULL;
}

UI_MENU_CALLBACK(load_settings_callback)
{
    if(activated) {
        if (resources_load(NULL) < 0) {
            ui_error("Cannot load settings.");
        } else {
            ui_message("Settings loaded.");
        }
        return sdl_menu_text_exit_ui;
    }
    return NULL;
}

UI_MENU_CALLBACK(default_settings_callback)
{
    if(activated) {
        resources_set_defaults();
        ui_message("Default settings restored.");
        return sdl_menu_text_exit_ui;
    }
    return NULL;
}

/* ------------------------------------------------------------------ */
/* Menu helpers */

const char *sdl_ui_menu_toggle_helper(int activated, const char *resource_name)
{
    int value, r;

    if(activated) {
        r = resources_toggle(resource_name, &value);
        if (r < 0)
            r = resources_get_int(resource_name, &value);
    } else
        r = resources_get_int(resource_name, &value);

    if (r < 0)
        return sdl_menu_text_unknown;
    else
        return value ? sdl_menu_text_tick : NULL;
}

const char *sdl_ui_menu_radio_helper(int activated, ui_callback_data_t param, const char *resource_name)
{
    if(activated) {
        resources_set_value(resource_name, (resource_value_t)param);
    } else {
        resource_value_t v;
        resources_get_value(resource_name, (void *)&v);
        if (v == (resource_value_t)param)
            return sdl_menu_text_tick;
    }
    return NULL;
}

const char *sdl_ui_menu_string_helper(int activated, ui_callback_data_t param, const char *resource_name)
{
    char *value = NULL;
    static const char *previous = NULL;

    if(resources_get_string(resource_name, &previous)) {
        return sdl_menu_text_unknown;
    }

    if (activated) {
        value = sdl_ui_text_input_dialog((const char*)param, previous);
        if(value) {
            resources_set_value_string(resource_name, value);
            lib_free(value);
        }
    } else {
        return previous;
    }
    return NULL;
}

const char *sdl_ui_menu_int_helper(int activated, ui_callback_data_t param, const char *resource_name)
{
    static char buf[20];
    char *value = NULL;
    int previous, new_value;

    if(resources_get_int(resource_name, &previous)) {
        return sdl_menu_text_unknown;
    }

    sprintf(buf, "%i", previous);

    if (activated) {
        value = sdl_ui_text_input_dialog((const char*)param, buf);
        if(value) {
            new_value = strtol(value, NULL, 0);
            resources_set_int(resource_name, new_value);
            lib_free(value);
        }
    } else {
        return buf;       
    }
    return NULL;
}


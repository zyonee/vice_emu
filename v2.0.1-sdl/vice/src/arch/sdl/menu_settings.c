/*
 * menu_settings.c - Common SDL settings related functions.
 *
 * Written by
 *  Marco van den Heuvel <blackystardust68@yahoo.com>
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

#include "menu_common.h"
#include "menu_settings.h"
#include "resources.h"
#include "ui.h"
#include "uimenu.h"


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

const ui_menu_entry_t settings_manager_menu[] = {
    { "Save current settings",
      MENU_ENTRY_OTHER,
      save_settings_callback,
      NULL },
    { "Load settings",
      MENU_ENTRY_OTHER,
      load_settings_callback,
      NULL },
    { "Restore default settings",
      MENU_ENTRY_OTHER,
      default_settings_callback,
      NULL },
    { NULL }
};

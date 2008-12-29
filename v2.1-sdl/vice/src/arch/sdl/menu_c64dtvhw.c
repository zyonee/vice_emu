/*
 * menu_c64dtvhw.c - C64DTV HW menu for SDL UI.
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

#include "lib.h"
#include "menu_common.h"
#include "resources.h"
#include "uifilereq.h"
#include "uimenu.h"

static UI_MENU_CALLBACK(c64dtvromimage_callback)
{
    char *name = NULL;

    if (activated) {
        name = sdl_ui_file_selection_dialog("C64DTV ROM image", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            resources_set_string("c64dtvromfilename", name);
            lib_free(name);
        }
    }
    return NULL;
}

UI_MENU_DEFINE_TOGGLE(c64dtvromrw)
UI_MENU_DEFINE_TOGGLE(FlashTrueFS)
UI_MENU_DEFINE_RADIO(DtvRevision)
UI_MENU_DEFINE_RADIO(HummerUserportDevice)
UI_MENU_DEFINE_RADIO(HummerUserportJoyPort)
UI_MENU_DEFINE_TOGGLE(ps2mouse)
UI_MENU_DEFINE_TOGGLE(Mouse)

const ui_menu_entry_t c64dtv_hardware_menu[] = {
    SDL_MENU_ITEM_TITLE("C64DTV ROM image"),
    { "C64DTV ROM image name...",
      MENU_ENTRY_DIALOG,
      c64dtvromimage_callback,
      NULL },
    { "Enable writes to C64DTV ROM image",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_c64dtvromrw_callback,
      NULL },
    { "Enable true flash filesystem",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_FlashTrueFS_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("DTV revision"),
    { "DTV2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DtvRevision_callback,
      (ui_callback_data_t)2 },
    { "DTV3",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_DtvRevision_callback,
      (ui_callback_data_t)3 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Hummer userport device"),
    { "None",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_HummerUserportDevice_callback,
      (ui_callback_data_t)0 },
    { "ADC",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_HummerUserportDevice_callback,
      (ui_callback_data_t)1 },
    { "Joystick",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_HummerUserportDevice_callback,
      (ui_callback_data_t)2 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Hummer joystick port mapped to userport"),
    { "Joy1",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_HummerUserportJoyPort_callback,
      (ui_callback_data_t)1 },
    { "Joy2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_HummerUserportJoyPort_callback,
      (ui_callback_data_t)2 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("PS/2 mouse on userport"),
    { "Enable PS/2 mouse",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_ps2mouse_callback,
      NULL },
    { "Grab mouse events",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Mouse_callback,
      NULL },
    { NULL }
};

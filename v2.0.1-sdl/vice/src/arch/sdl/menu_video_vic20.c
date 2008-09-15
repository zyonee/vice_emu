/*
 * menu_video_vic20.c - VIC20 SDL video settings functions.
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

#include "lib.h"
#include "machine.h"
#include "menu_common.h"
#include "menu_video.h"
#include "menu_video_vic20.h"
#include "resources.h"
#include "ui.h"
#include "uifilereq.h"
#include "uimenu.h"

static UI_MENU_CALLBACK(radio_MachineVideoStandard_callback)
{
    if (activated) {
        int value = (int)param;
        sdl_ui_set_menu_borders(0, (value == MACHINE_SYNC_PAL) ? 28: 8);
    }
    return sdl_ui_menu_radio_helper(activated, param, "MachineVideoStandard");
}

static UI_MENU_CALLBACK(external_palette_callback)
{
    char *name = NULL;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Choose palette file", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            resources_set_string("VICPaletteFile", name);
            lib_free(name);
        }
    }
    return NULL;
}

UI_MENU_DEFINE_TOGGLE(VICFullscreen)
UI_MENU_DEFINE_TOGGLE(VICDoubleSize)
UI_MENU_DEFINE_TOGGLE(VICDoubleScan)
UI_MENU_DEFINE_TOGGLE(VICVideoCache)
UI_MENU_DEFINE_TOGGLE(VICExternalPalette)

const ui_menu_entry_t vic20_video_menu[] = {
    { "Fullscreen",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICFullscreen_callback,
      NULL },
    { "Double size",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICDoubleSize_callback,
      NULL },
    { "Double scan",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICDoubleScan_callback,
      NULL },
    { "Video cache",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICVideoCache_callback,
      NULL },
    { "Color controls",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)color_controls_menu },
    { "PAL emulation controls",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)pal_controls_menu },
    { "External palette",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICExternalPalette_callback,
      NULL },
    { "Select external palette file",
      MENU_ENTRY_DIALOG,
      external_palette_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    { "Video Standard", MENU_ENTRY_TEXT, NULL, (ui_callback_data_t)1 },
    { "PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_callback,
      (ui_callback_data_t)MACHINE_SYNC_PAL },
    { "NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_callback,
      (ui_callback_data_t)MACHINE_SYNC_NTSC },
    { NULL }
};

/*
 * menu_video_c64.c - C64 SDL video settings functions.
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
#include "menu_video_c64.h"
#include "resources.h"
#include "ui.h"
#include "uifilereq.h"
#include "uimenu.h"

static UI_MENU_CALLBACK(fullscreen_callback)
{
    return NULL;
}

UI_MENU_DEFINE_TOGGLE(VICIIDoubleSize)
UI_MENU_DEFINE_TOGGLE(VICIIDoubleScan)
UI_MENU_DEFINE_TOGGLE(VICIIVideoCache)
UI_MENU_DEFINE_TOGGLE(VICIIExternalPalette)
UI_MENU_DEFINE_RADIO(MachineVideoStandard)

static UI_MENU_CALLBACK(external_palette_callback)
{
    char *name = NULL;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Choose palette file", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            resources_set_string("VICIIPaletteFile", name);
            lib_free(name);
        }
    }
    return NULL;
}

const ui_menu_entry_t c64_video_menu[] = {
    { "Fullscreen (todo)",
      MENU_ENTRY_OTHER,
      fullscreen_callback,
      NULL },
    { "Double size",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIDoubleSize_callback,
      NULL },
    { "Double scan",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIDoubleScan_callback,
      NULL },
    { "Video cache",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIVideoCache_callback,
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
      toggle_VICIIExternalPalette_callback,
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
    { "NTSC old",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_callback,
      (ui_callback_data_t)MACHINE_SYNC_NTSCOLD },
    { NULL }
};

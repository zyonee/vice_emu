/*
 * menu_video_c128.c - C128 SDL video settings functions.
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
#include "menu_video_c128.h"
#include "resources.h"
#include "ui.h"
#include "uifilereq.h"
#include "uimenu.h"

UI_MENU_DEFINE_TOGGLE(VICIIFullscreen)
UI_MENU_DEFINE_TOGGLE(VICIIDoubleSize)
UI_MENU_DEFINE_TOGGLE(VICIIDoubleScan)
UI_MENU_DEFINE_TOGGLE(VICIIVideoCache)
UI_MENU_DEFINE_TOGGLE(VDCDoubleSize)
UI_MENU_DEFINE_TOGGLE(VDCDoubleScan)
UI_MENU_DEFINE_TOGGLE(VDCVideoCache)
UI_MENU_DEFINE_TOGGLE(VICIIExternalPalette)
UI_MENU_DEFINE_RADIO(MachineVideoStandard)

static UI_MENU_CALLBACK(external_vicii_palette_callback)
{
    char *name = NULL;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Choose VICII palette file", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            resources_set_string("VICIIPaletteFile", name);
            lib_free(name);
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(external_vdc_palette_callback)
{
    char *name = NULL;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Choose VDC palette file", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            resources_set_string("VDCPaletteFile", name);
            lib_free(name);
        }
    }
    return NULL;
}

const ui_menu_entry_t c128_video_menu[] = {
    { "Fullscreen",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIFullscreen_callback,
      NULL },
    { "VICII Double size",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIDoubleSize_callback,
      NULL },
    { "VICII Double scan",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIDoubleScan_callback,
      NULL },
    { "VICII Video cache",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIVideoCache_callback,
      NULL },
    { "VDC Double size",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VDCDoubleSize_callback,
      NULL },
    { "VDC Double scan",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VDCDoubleScan_callback,
      NULL },
    { "VDC Video cache",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VDCVideoCache_callback,
      NULL },
    { "Color controls",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)color_controls_menu },
    { "PAL emulation controls",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)pal_controls_menu },
    { "External VICII palette",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIExternalPalette_callback,
      NULL },
    { "Select VICII external palette file",
      MENU_ENTRY_DIALOG,
      external_vicii_palette_callback,
      NULL },
    { "Select VDC external palette file",
      MENU_ENTRY_DIALOG,
      external_vdc_palette_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Video Standard"),
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

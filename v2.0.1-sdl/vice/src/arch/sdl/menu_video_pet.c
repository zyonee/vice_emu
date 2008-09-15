/*
 * menu_video_pet.c - PET SDL video settings functions.
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
#include "menu_video_pet.h"
#include "resources.h"
#include "ui.h"
#include "uifilereq.h"
#include "uimenu.h"

UI_MENU_DEFINE_TOGGLE(CrtcFullscreen)
UI_MENU_DEFINE_TOGGLE(CrtcDoubleSize)
UI_MENU_DEFINE_TOGGLE(CrtcDoubleScan)
UI_MENU_DEFINE_TOGGLE(CrtcVideoCache)

static UI_MENU_CALLBACK(external_palette_callback)
{
    char *name = NULL;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Choose palette file", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            resources_set_string("CrtcPaletteFile", name);
            lib_free(name);
        }
    }
    return NULL;
}

const ui_menu_entry_t pet_video_menu[] = {
    { "Fullscreen",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CrtcFullscreen_callback,
      NULL },
    { "Double size",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CrtcDoubleSize_callback,
      NULL },
    { "Double scan",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CrtcDoubleScan_callback,
      NULL },
    { "Video cache",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CrtcVideoCache_callback,
      NULL },
    { "Select external palette file",
      MENU_ENTRY_DIALOG,
      external_palette_callback,
      NULL },
    { NULL }
};

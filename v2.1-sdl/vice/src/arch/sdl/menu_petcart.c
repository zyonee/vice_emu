/*
 * menu_petcart.c - Implementation of the pet cartridge settings menu for the SDL UI.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "menu_petcart.h"
#include "menu_common.h"
#include "ui.h"
#include "uimenu.h"

UI_MENU_DEFINE_FILE_STRING(RomModule9Name)
UI_MENU_DEFINE_FILE_STRING(RomModuleAName)
UI_MENU_DEFINE_FILE_STRING(RomModuleBName)

static UI_MENU_CALLBACK(detach_cart_callback)
{
    if (activated) {
        resources_set_string((char *)param, "");
    }
    return NULL;
}

const ui_menu_entry_t petcart_menu[] = {
    SDL_MENU_ITEM_TITLE("ROM 9"),
    { "attach",
      MENU_ENTRY_DIALOG,
      file_string_RomModule9Name_callback,
      (ui_callback_data_t)"Select ROM 9 image" },
    { "Detach",
      MENU_ENTRY_OTHER,
      detach_cart_callback,
      (ui_callback_data_t)"RomModule9Name" },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("ROM A"),
    { "attach",
      MENU_ENTRY_DIALOG,
      file_string_RomModuleAName_callback,
      (ui_callback_data_t)"Select ROM A image" },
    { "Detach",
      MENU_ENTRY_OTHER,
      detach_cart_callback,
      (ui_callback_data_t)"RomModuleAName" },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("ROM B"),
    { "attach",
      MENU_ENTRY_DIALOG,
      file_string_RomModuleBName_callback,
      (ui_callback_data_t)"Select ROM B image" },
    { "Detach",
      MENU_ENTRY_OTHER,
      detach_cart_callback,
      (ui_callback_data_t)"RomModuleBName" },
    { NULL }
};

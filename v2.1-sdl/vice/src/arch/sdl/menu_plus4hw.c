/*
 * menu_plus4hw.c - PLUS4 HW menu for SDL UI.
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

#include "types.h"

#include "menu_common.h"
#include "menu_joystick.h"
#include "menu_ram.h"
#include "menu_sid.h"
#include "uimenu.h"

UI_MENU_DEFINE_RADIO(RamSize)
UI_MENU_DEFINE_TOGGLE(CS256K)
UI_MENU_DEFINE_RADIO(H256K)

const ui_menu_entry_t plus4_hardware_menu[] = {
    { "Joystick settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joystick_menu },
    { "SID cart settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_plus4_menu },
    { "RAM pattern settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ram_menu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Memory"),
    { "16K",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)16 },
    { "32K",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)32 },
    { "64K",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)64 },
    { "256K (CSORY)",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CS256K_callback,
      NULL },
    { "256K (HANNES)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_H256K_callback,
      (ui_callback_data_t)1 },
    { "1024K (HANNES)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_H256K_callback,
      (ui_callback_data_t)2 },
    { "4096K (HANNES)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_H256K_callback,
      (ui_callback_data_t)3 },
    { NULL }
};

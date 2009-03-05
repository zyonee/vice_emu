/*
 * menu_cbm2hw.c - CBM2 HW menu for SDL UI.
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
#include "menu_sid.h"
#include "uimenu.h"

UI_MENU_DEFINE_RADIO(ModelLine)
UI_MENU_DEFINE_RADIO(RamSize)
UI_MENU_DEFINE_TOGGLE(Ram08)
UI_MENU_DEFINE_TOGGLE(Ram1)
UI_MENU_DEFINE_TOGGLE(Ram2)
UI_MENU_DEFINE_TOGGLE(Ram4)
UI_MENU_DEFINE_TOGGLE(Ram6)
UI_MENU_DEFINE_TOGGLE(RamC)

static const ui_menu_entry_t cbm2_model_menu[] = {
    SDL_MENU_ITEM_TITLE("Model line"),
    { "7x0 (50 Hz)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_ModelLine_callback,
      (ui_callback_data_t)0 },
    { "6x0 60 Hz",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_ModelLine_callback,
      (ui_callback_data_t)1 },
    { "6x0 50 Hz",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_ModelLine_callback,
      (ui_callback_data_t)2 },
    { NULL }
};

static const ui_menu_entry_t cbm2_memory_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("CBM2 memory size"),
    { "128K",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)128 },
    { "256K",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)256 },
    { "512K",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)512 },
    { "1024K",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_RamSize_callback,
      (ui_callback_data_t)1024 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("CBM2 memory blocks"),
    { "RAM at $0800-$0FFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Ram08_callback,
      NULL },
    { "RAM at $1000-$1FFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Ram1_callback,
      NULL },
    { "RAM at $2000-$3FFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Ram2_callback,
      NULL },
    { "RAM at $4000-$5FFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Ram4_callback,
      NULL },
    { "RAM at $6000-$7FFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_Ram6_callback,
      NULL },
    { "RAM at $C000-$CFFF",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_RamC_callback,
      NULL },
    { NULL }
};

const ui_menu_entry_t cbm5x0_hardware_menu[] = {
    { "Joystick settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joystick_menu },
    { "SID settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_cbm2_menu },
    { "CBM2 memory setting",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)cbm2_memory_menu },
    { NULL }
};

const ui_menu_entry_t cbm6x0_7x0_hardware_menu[] = {
    { "Joystick settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joystick_menu },
    { "SID settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_cbm2_menu },
    { "CBM2 model settings",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)cbm2_model_menu },
    { "CBM2 memory setting",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)cbm2_memory_menu },
    { NULL }
};

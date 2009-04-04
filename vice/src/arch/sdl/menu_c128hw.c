/*
 * menu_c128hw.c - C128 HW menu for SDL UI.
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

#include "menu_c64_common_expansions.h"
#include "menu_common.h"
#include "menu_joystick.h"
#ifdef HAVE_MOUSE
#include "menu_mouse.h"
#endif
#include "menu_ram.h"
#include "menu_rom.h"
#include "menu_sid.h"
#include "uimenu.h"

UI_MENU_DEFINE_RADIO(VDC64KB)
UI_MENU_DEFINE_RADIO(VDCRevision)

static const ui_menu_entry_t vdc_menu[] = {
    SDL_MENU_ITEM_TITLE("VDC revision"),
    { "Rev 0",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VDCRevision_callback,
      (ui_callback_data_t)0 },
    { "Rev 1",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VDCRevision_callback,
      (ui_callback_data_t)1 },
    { "Rev 2",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VDCRevision_callback,
      (ui_callback_data_t)2 },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("VDC memory size"),
    { "16kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VDC64KB_callback,
      (ui_callback_data_t)0 },
    { "64kB",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VDC64KB_callback,
      (ui_callback_data_t)1 },
    { NULL }
};

UI_MENU_DEFINE_TOGGLE(EmuID)
UI_MENU_DEFINE_TOGGLE(IEEE488)

const ui_menu_entry_t c128_hardware_menu[] = {
    { "Joystick settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joystick_menu },
    { "SID settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_c128_menu },
    { "VDC settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)vdc_menu },
#ifdef HAVE_MOUSE
    { "Mouse emulation",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mouse_menu },
#endif
    { "RAM pattern settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ram_menu },
    { "ROM settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)c128_rom_menu },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Hardware expansions"),
#if 0 /* TODO */
    { "ACIA settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)c64_acia_menu },
#endif
    { "Digimax settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)digimax_menu },
    { "GEORAM settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)georam_menu },
    { "IDE64 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ide64_menu },
    { "IEEE488 interface",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_IEEE488_callback,
      NULL },
#if 0 /* TODO */
    { "MIDI settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)c64_midi_menu },
#endif
    { "MMC64 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mmc64_menu },
    { "RAMCART settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ramcart_menu },
    { "REU settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)reu_menu },
#if 0 /* TODO */
    { "RS232 userport settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)rs232user_menu },
    { "The Final Ethernet settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)tfe_menu },
#endif
    { "Emulator ID",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_EmuID_callback,
      NULL },
    { NULL }
};

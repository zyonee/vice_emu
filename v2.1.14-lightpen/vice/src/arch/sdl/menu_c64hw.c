/*
 * menu_c64hw.c - C64 HW menu for SDL UI.
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

#include <stdio.h>

#include "types.h"

#include "menu_c64_common_expansions.h"
#include "menu_c64_expansions.h"
#include "menu_common.h"
#include "menu_joystick.h"
#ifdef HAVE_MIDI
#include "menu_midi.h"
#endif
#ifdef HAVE_MOUSE
#include "menu_lightpen.h"
#include "menu_mouse.h"
#endif
#include "menu_ram.h"
#include "menu_rom.h"
#ifdef HAVE_RS232
#include "menu_rs232.h"
#endif
#include "menu_sid.h"
#ifdef HAVE_TFE
#include "menu_tfe.h"
#endif
#include "uimenu.h"

UI_MENU_DEFINE_TOGGLE(EmuID)
UI_MENU_DEFINE_TOGGLE(SFXSoundExpander)
UI_MENU_DEFINE_TOGGLE(SFXSoundSampler)

const ui_menu_entry_t c64_hardware_menu[] = {
    { "Joystick settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)joystick_four_menu },
    { "SID settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sid_c64_menu },
#ifdef HAVE_MOUSE
    { "Mouse emulation",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mouse_menu },
    { "Lightpen emulation",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)lightpen_menu },
#endif
    { "RAM pattern settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ram_menu },
    { "ROM settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)c64_vic20_rom_menu },
    SDL_MENU_ITEM_TITLE("Hardware expansions"),
    { "256K settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)c64_256k_menu },
#ifdef HAVE_RS232
    { "RS232 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)rs232_menu },
#endif
    { "Digimax settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)digimax_menu },
    { "Double Quick Brown Box settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)dqbb_menu },
    { "GEORAM settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)georam_menu },
    { "IDE64 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ide64_menu },
    { "Isepic settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)isepic_menu },
#ifdef HAVE_MIDI
    { "MIDI settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)midi_c64_menu },
#endif
    { "MMC64 settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)mmc64_menu },
    { "PLUS60K settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)plus60k_menu },
    { "PLUS256K settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)plus256k_menu },
    { "RAMCART settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)ramcart_menu },
    { "REU settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)reu_menu },
#ifdef HAVE_TFE
    { "The Final Ethernet settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)tfe_menu },
#endif
    { "SFX Sound Expander",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SFXSoundExpander_callback,
      NULL },
    { "SFX Sound Sampler",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_SFXSoundSampler_callback,
      NULL },
    { "Emulator ID",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_EmuID_callback,
      NULL },
    { NULL }
};

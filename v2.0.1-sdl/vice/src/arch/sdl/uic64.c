/*
 * uic64.c - Implementation of the C64-specific part of the UI.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  André Fachat <fachat@physik.tu-chemnitz.de>
 *  Andreas Boose <viceteam@t-online.de>
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

#include "c64mem.h"
#include "uimenu.h"

#include <stdio.h>

UI_MENU_DEFINE_RADIO(SidModel)

static ui_menu_entry_t sid_model_menu[] = {
    { "6581 (old)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidModel_callback,
      (ui_callback_data_t)0,
      NULL },
    { "8580 (new)",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidModel_callback,
      (ui_callback_data_t)1,
      NULL },
    { "8580 + digiboost",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidModel_callback,
      (ui_callback_data_t)2,
      NULL },
    { "DTVSID",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_SidModel_callback,
      (ui_callback_data_t)4,
      NULL },
    { NULL }
};

UI_MENU_DEFINE_TOGGLE(VICIIDoubleSize)

static UI_MENU_CALLBACK(quit_callback)
{
    exit(0);
    return 0;
}

static ui_menu_entry_t x64_main_menu[] = {
    { "Attach disk",
      MENU_ENTRY_DIALOG,
      NULL, /* disk_attach_dialog */
      NULL,
      NULL },
    { "Doublesize",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIDoubleSize_callback,
      NULL,
      NULL },
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { "SID model",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      sid_model_menu },
    { "Quit",
      MENU_ENTRY_OTHER,
      quit_callback,
      NULL,
      NULL },
    { NULL }
};

int c64ui_init(void)
{
fprintf(stderr,"%s\n",__func__);
    sdl_register_vcachename("VICIIVideoCache");
    sdl_ui_set_main_menu(x64_main_menu);
    sdl_ui_set_menu_font(mem_chargen_rom + 0x800, 8, 8);
    return 0;
}

void c64ui_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);
}


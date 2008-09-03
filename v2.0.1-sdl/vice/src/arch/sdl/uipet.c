/*
 * uipet.c - Implementation of the PET-specific part of the UI.
 *
 * Written by
 *  Hannu Nuotio <hannu.nuotio@tut.fi>
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

#include "petmem.h"
#include "resources.h"
#include "ui.h"
#include "uimenu.h"

UI_MENU_DEFINE_TOGGLE(CrtcDoubleSize)

static UI_MENU_CALLBACK(quit_callback)
{
    exit(0);
    return 0;
}

static ui_menu_entry_t xpet_main_menu[] = {
    { "Attach disk",
      MENU_ENTRY_SUBMENU,
      NULL, /* disk_attach_dialog */
      NULL,
      NULL },
    { "Doublesize",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CrtcDoubleSize_callback,
      NULL,
      NULL },
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { "Quit",
      MENU_ENTRY_OTHER,
      quit_callback,
      NULL,
      NULL },
    { NULL }
};

BYTE pet_font[14*256];

int petui_init(void)
{
    int i, j;

fprintf(stderr,"%s\n",__func__);

    sdl_ui_set_main_menu(xpet_main_menu);

    for (i=0; i<128; i++)
    {
        for (j=0; j<8; j++)
        {
            pet_font[(i*8)+j]=mem_chargen_rom[(i*16)+(256*16)+j];
            pet_font[(i*8)+(128*8)+j]=mem_chargen_rom[(i*16)+j];
        }
    }
    sdl_ui_set_menu_font(pet_font, NULL, 0, 8, 8);
    sdl_register_vcachename("CrtcVideoCache");
    return 0;
}

void petui_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);
}

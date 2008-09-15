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

#include "debug.h"
#include "lib.h"
#include "menu_common.h"
#include "menu_help.h"
#include "menu_reset.h"
#include "menu_screenshot.h"
#include "menu_settings.h"
#include "menu_speed.h"
#include "menu_video_pet.h"
#include "petmem.h"
#include "resources.h"
#include "ui.h"
#include "uimenu.h"

/* temporary empty drive menu, this one will be moved out to menu_drive.c */
static ui_menu_entry_t drive_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};

/* temporary empty tape menu, this one will be moved out to menu_tape.c */
static ui_menu_entry_t tape_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};

/* temporary empty pet hardware menu, this one will be moved out to menu_pethw.c */
static ui_menu_entry_t pet_hardware_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};

/* temporary empty pet rom menu, this one will be moved out to menu_petrom.c */
static ui_menu_entry_t pet_rom_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};

/* temporary empty sound menu, this one will be moved out to menu_sound.c */
static ui_menu_entry_t sound_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};

/* temporary empty snapshot menu, this one will be moved out to menu_snapshot.c */
static ui_menu_entry_t snapshot_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};

#ifdef DEBUG
/* temporary empty debug menu, this one will be moved out to menu_debug.c */
static ui_menu_entry_t debug_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};
#endif

static const ui_menu_entry_t xpet_main_menu[] = {
    { "Autostart image",
      MENU_ENTRY_DIALOG,
      autostart_callback,
      NULL },
    { "Drive (todo)",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)drive_menu },
    { "Tape (todo)",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)tape_menu },
    { "Machine settings (todo)",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)pet_hardware_menu },
    { "ROM settings (todo)",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)pet_rom_menu },
    { "Video settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)pet_video_menu },
    { "Sound settings (todo)",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)sound_menu },
    { "Snapshot (todo)",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)snapshot_menu },
    { "Screenshot (to be fixed)",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)screenshot_menu },
    { "Speed settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)speed_menu },
    { "Reset",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)reset_menu },
    { "Pause (todo)",
      MENU_ENTRY_OTHER,
      pause_callback,
      NULL },
    { "Monitor (todo)",
      MENU_ENTRY_OTHER,
      monitor_callback,
      NULL },
#ifdef DEBUG
    { "Debug (todo)",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)debug_menu },
#endif
    { "Help",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)help_menu },
    { "Settings management",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)settings_manager_menu },
    { "Quit emulator",
      MENU_ENTRY_OTHER,
      quit_callback,
      NULL },
    { NULL }
};

static BYTE *pet_font;

int petui_init(void)
{
    int i, j, cols;

fprintf(stderr,"%s\n",__func__);

    sdl_ui_set_main_menu(xpet_main_menu);

    resources_get_int("VideoSize", &cols);

    pet_font=lib_malloc(8*256);
    for (i=0; i<128; i++)
    {
        for (j=0; j<8; j++)
        {
            pet_font[(i*8)+j]=mem_chargen_rom[(i*16)+(256*16)+j];
            pet_font[(i*8)+(128*8)+j]=mem_chargen_rom[(i*16)+j];
        }
    }
    sdl_ui_set_menu_font(pet_font, 8, 8);
    sdl_ui_set_menu_borders(32, (cols == 40) ? 40 : 28);
    sdl_ui_set_double_x((cols == 40) ? 0 : 1);
    sdl_ui_set_menu_colors(1, 0);
    sdl_ui_set_vcachename("CrtcVideoCache");
    return 0;
}

void petui_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);

    lib_free(pet_font);
}


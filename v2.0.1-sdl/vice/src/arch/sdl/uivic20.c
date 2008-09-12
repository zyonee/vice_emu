/*
 * uivic20.c - Implementation of the VIC20-specific part of the UI.
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
#include "machine.h"
#include "menu_common.h"
#include "menu_reset.h"
#include "menu_speed.h"
#include "resources.h"
#include "ui.h"
#include "uimenu.h"
#include "vic20memrom.h"

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

/* temporary empty vic20 cart menu, this one will be moved out to menu_vic20_cart.c */
static ui_menu_entry_t vic20_cart_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};

/* temporary empty vic20 hardware menu, this one will be moved out to menu_vic20hw.c */
static ui_menu_entry_t vic20_hardware_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};

/* temporary empty vic20 rom menu, this one will be moved out to menu_vic20rom.c */
static ui_menu_entry_t vic20_rom_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};

/* temporary empty vic20 video menu, this one will be moved out to menu_vic20video.c */
static ui_menu_entry_t vic20_video_menu[] = {
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

/* temporary empty help menu, this one will be moved out to menu_help.c */
static ui_menu_entry_t help_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};

static const ui_menu_entry_t xvic_main_menu[] = {
    { "Autostart image",
      MENU_ENTRY_DIALOG,
      autostart_callback,
      NULL },
    { "Drive",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)drive_menu },
    { "Tape",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)tape_menu },
    { "Cartridge",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)vic20_cart_menu },
    { "Machine settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)vic20_hardware_menu },
    { "ROM settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)vic20_rom_menu },
    { "Video settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)vic20_video_menu },
    { "Sound settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)sound_menu },
    { "Snapshot",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)snapshot_menu },
    { "Speed settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)speed_menu },
    { "Reset",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)reset_menu },
    { "Pause",
      MENU_ENTRY_OTHER,
      pause_callback,
      NULL },
    { "Monitor",
      MENU_ENTRY_OTHER,
      monitor_callback,
      NULL },
#ifdef DEBUG
    { "Debug",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)debug_menu },
#endif
    { "Help",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)help_menu },
    { "Save current settings",
      MENU_ENTRY_OTHER,
      save_settings_callback,
      NULL },
    { "Load settings",
      MENU_ENTRY_OTHER,
      load_settings_callback,
      NULL },
    { "Restore default settings",
      MENU_ENTRY_OTHER,
      default_settings_callback,
      NULL },
    { "Quit emulator",
      MENU_ENTRY_OTHER,
      quit_callback,
      NULL },
    { NULL }
};

static BYTE *vic20_font;

int vic20ui_init(void)
{
    int i, j, videostandard;
fprintf(stderr,"%s\n",__func__);

    sdl_ui_set_vcachename("VICVideoCache");
    sdl_ui_set_main_menu(xvic_main_menu);

    resources_get_int("MachineVideoStandard", &videostandard);

    vic20_font=lib_malloc(8*256);
    for (i=0; i<128; i++)
    {
        for (j=0; j<8; j++)
        {
            vic20_font[(i*8)+j]=vic20memrom_chargen_rom[(i*8)+(128*8)+j+0x800];
            vic20_font[(i*8)+(128*8)+j]=vic20memrom_chargen_rom[(i*8)+j+0x800];
        }
    }

    sdl_ui_set_menu_font(vic20_font, 8, 8);
    sdl_ui_set_menu_borders(0, (videostandard == MACHINE_SYNC_PAL) ? 28: 8);
    sdl_ui_set_double_x(1);
    sdl_ui_set_menu_colors(1, 0);
    return 0;
}

void vic20ui_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);

    lib_free(vic20_font);
}


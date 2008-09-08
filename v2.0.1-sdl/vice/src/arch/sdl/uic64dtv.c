/*
 * uic64dtv.c - Implementation of the C64DTV-specific part of the UI.
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
#include "c64mem.h"
#include "menu_common.h"
#include "menu_reset.h"
#include "menu_speed.h"
#include "ui.h"
#include "uimenu.h"

/* temporary empty drive menu, this one will be moved out to menu_drive.c */
static ui_menu_entry_t drive_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};

/* temporary empty c64dtv hardware menu, this one will be moved out to menu_c64dtvhw.c */
static ui_menu_entry_t c64dtv_hardware_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};

/* temporary empty c64dtv rom menu, this one will be moved out to menu_c64dtvrom.c */
static ui_menu_entry_t c64dtv_rom_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};

/* temporary empty c64dtv video menu, this one will be moved out to menu_c64dtvvideo.c */
static ui_menu_entry_t c64dtv_video_menu[] = {
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

static const ui_menu_entry_t x64dtv_main_menu[] = {
    { "Autostart image",
      MENU_ENTRY_OTHER,
      autostart_callback,
      NULL },
    { "Drive",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)drive_menu },
    { "Machine settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)c64dtv_hardware_menu },
    { "ROM settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)c64dtv_rom_menu },
    { "Video settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      (ui_callback_data_t)c64dtv_video_menu },
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
    { "Quit emulator",
      MENU_ENTRY_OTHER,
      quit_callback,
      NULL },
    { NULL }
};

int c64dtvui_init(void)
{
fprintf(stderr,"%s\n",__func__);

    sdl_ui_set_vcachename("VICIIVideoCache");
    sdl_ui_set_main_menu(x64dtv_main_menu);
    sdl_ui_set_menu_font(mem_chargen_rom + 0x800, 8, 8);
    sdl_ui_set_menu_colors(15, 0);
    return 0;
}

void c64dtvui_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);
}


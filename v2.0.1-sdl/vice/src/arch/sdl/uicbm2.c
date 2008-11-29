/*
 * uicmb2.c - Implementation of the CBM-II-specific part of the UI.
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
#include "cbm2.h"
#include "cbm2mem.h"
#include "lib.h"
#include "menu_common.h"
#include "menu_drive.h"
#include "menu_help.h"
#include "menu_reset.h"
#include "menu_screenshot.h"
#include "menu_settings.h"
#include "menu_snapshot.h"
#include "menu_sound.h"
#include "menu_speed.h"
#include "menu_video_cbm2.h"
#include "resources.h"
#include "ui.h"
#include "uimenu.h"

/* temporary empty tape menu, this one will be moved out to menu_tape.c */
static ui_menu_entry_t tape_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};

/* temporary empty cbm2 hardware menu, this one will be moved out to menu_cbm2hw.c */
static ui_menu_entry_t cbm2_hardware_menu[] = {
    SDL_MENU_ITEM_SEPARATOR,
    { NULL }
};

/* temporary empty cbm2 rom menu, this one will be moved out to menu_cbm2rom.c */
static ui_menu_entry_t cbm2_rom_menu[] = {
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

static const ui_menu_entry_t xcbm6x0_7x0_main_menu[] = {
    { "Autostart image",
      MENU_ENTRY_DIALOG,
      autostart_callback,
      NULL },
    { "Drive",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)drive_menu },
    { "Tape (todo)",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)tape_menu },
    { "Machine settings (todo)",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)cbm2_hardware_menu },
    { "ROM settings (todo)",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)cbm2_rom_menu },
    { "Video settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)cbm6x0_7x0_video_menu },
    { "Sound output settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sound_output_menu },
    { "Sound record settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sound_record_menu },
    { "Snapshot",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)snapshot_menu },
    { "Screenshot",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)screenshot_menu },
    { "Speed settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)speed_menu },
    { "Reset",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
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
      submenu_callback,
      (ui_callback_data_t)debug_menu },
#endif
    { "Help",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)help_menu },
    { "Settings management",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)settings_manager_menu },
    { "Quit emulator",
      MENU_ENTRY_OTHER,
      quit_callback,
      NULL },
    { NULL }
};

static const ui_menu_entry_t xcbm5x0_main_menu[] = {
    { "Autostart image",
      MENU_ENTRY_DIALOG,
      autostart_callback,
      NULL },
    { "Drive",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)drive_menu },
    { "Tape (todo)",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)tape_menu },
    { "Machine settings (todo)",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)cbm2_hardware_menu },
    { "ROM settings (todo)",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)cbm2_rom_menu },
    { "Video settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)cbm5x0_video_menu },
    { "Sound output settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sound_output_menu },
    { "Sound record settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)sound_record_menu },
    { "Snapshot",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)snapshot_menu },
    { "Speed settings",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)speed_menu },
    { "Reset",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
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
      submenu_callback,
      (ui_callback_data_t)debug_menu },
#endif
    { "Help",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)help_menu },
    { "Settings management",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)settings_manager_menu },
    { "Quit emulator",
      MENU_ENTRY_OTHER,
      quit_callback,
      NULL },
    { NULL }
};

static BYTE *cbm2_font;

int cbm2ui_init(void)
{
    int i, j, model;

fprintf(stderr,"%s\n",__func__);

    if (cbm2_is_c500()) {
        sdl_ui_set_menu_borders(0, 0);
        sdl_ui_set_menu_font(mem_chargen_rom + 0x800, 8, 8);
        sdl_ui_set_main_menu(xcbm5x0_main_menu);
    } else {
        resources_get_int("ModelLine", &model);
        if (model == 0)
        {
            cbm2_font=lib_malloc(14*256);
            for (i=0; i<256; i++)
            {
                for (j=0; j<14; j++)
                {
                    cbm2_font[(i*14)+j]=mem_chargen_rom[(i*16)+j+1];
                }
            }
        }
        else
        {
            cbm2_font=lib_malloc(8*256);
            for (i=0; i<256; i++)
            {
                for (j=0; j<8; j++)
                {
                    cbm2_font[(i*8)+j]=mem_chargen_rom[(i*16)+j];
                }
            }
        }
        sdl_ui_set_menu_font(cbm2_font, 8, (model == 0) ? 14 : 8);
        sdl_ui_set_menu_borders(32, (model == 0) ? 16 : 40);
        sdl_ui_set_main_menu(xcbm6x0_7x0_main_menu);
    }
    sdl_ui_set_menu_colors(1, 0);
    sdl_ui_set_double_x(1);
    return 0;
}

void cbm2ui_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);

    lib_free(cbm2_font);
}

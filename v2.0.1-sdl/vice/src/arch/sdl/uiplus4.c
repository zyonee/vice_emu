/*
 * uiplus4.c - Implementation of the Plus4-specific part of the UI.
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
#include "menu_reset.h"
#include "menu_speed.h"
#include "plus4memrom.h"
#include "resources.h"
#include "ui.h"
#include "uimenu.h"

/* temporary place holder for the autostart callback till we get file selectors. */
static UI_MENU_CALLBACK(autostart_callback)
{
    return NULL;
}

/* temporary empty drive menu, this one will be moved out to menu_drive.c */
static ui_menu_entry_t drive_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty tape menu, this one will be moved out to menu_tape.c */
static ui_menu_entry_t tape_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty plus4 cart menu, this one will be moved out to menu_plus4_cart.c */
static ui_menu_entry_t plus4_cart_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty plus4 hardware menu, this one will be moved out to menu_plus4hw.c */
static ui_menu_entry_t plus4_hardware_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty plus4 rom menu, this one will be moved out to menu_plus4rom.c */
static ui_menu_entry_t plus4_rom_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty plus4 video menu, this one will be moved out to menu_plus4video.c */
static ui_menu_entry_t plus4_video_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty sound menu, this one will be moved out to menu_sound.c */
static ui_menu_entry_t sound_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty snapshot menu, this one will be moved out to menu_snapshot.c */
static ui_menu_entry_t snapshot_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary place holder for the pause callback till we can get it working. */
static UI_MENU_CALLBACK(pause_callback)
{
    return NULL;
}

/* temporary place holder for the monitor callback till we can get it working. */
static UI_MENU_CALLBACK(monitor_callback)
{
    return NULL;
}

#ifdef DEBUG
/* temporary empty debug menu, this one will be moved out to menu_debug.c */
static ui_menu_entry_t debug_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};
#endif

/* temporary empty help menu, this one will be moved out to menu_help.c */
static ui_menu_entry_t help_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* this callback will be moved out to menu_common.c */
static UI_MENU_CALLBACK(quit_callback)
{
    if(activated) {
        exit(0);
    }
    return NULL;
}

static ui_menu_entry_t xplus4_main_menu[] = {
    { "Autostart image",
      MENU_ENTRY_OTHER,
      autostart_callback,
      NULL,
      NULL },
    { "Drive",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      drive_menu },
    { "Tape",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      tape_menu },
    { "Cartridge",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      plus4_cart_menu },
    { "Machine settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      plus4_hardware_menu },
    { "ROM settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      plus4_rom_menu },
    { "Video settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      plus4_video_menu },
    { "Sound settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      sound_menu },
    { "Snapshot",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      snapshot_menu },
    { "Speed settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      speed_menu },
    { "Reset",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      reset_menu },
    { "Pause",
      MENU_ENTRY_OTHER,
      pause_callback,
      NULL,
      NULL },
    { "Monitor",
      MENU_ENTRY_OTHER,
      monitor_callback,
      NULL,
      NULL },
#ifdef DEBUG
    { "Debug",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      debug_menu },
#endif
    { "Help",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      help_menu },
    { "Quit emulator",
      MENU_ENTRY_OTHER,
      quit_callback,
      NULL,
      NULL },
    { NULL }
};

static BYTE *plus4_font;

int plus4ui_init(void)
{
    int i, j;

fprintf(stderr,"%s\n",__func__);

    sdl_ui_set_vcachename("TEDVideoCache");
    sdl_ui_set_main_menu(xplus4_main_menu);

    plus4_font=lib_malloc(8*256);
    for (i=0; i<128; i++)
    {
        for (j=0; j<8; j++)
        {
            plus4_font[(i*8)+j]=plus4memrom_kernal_rom[(i*8)+(128*8)+j+0x1000];
            plus4_font[(i*8)+(128*8)+j]=plus4memrom_kernal_rom[(i*8)+j+0x1000];
        }
    }

    sdl_ui_set_menu_font(plus4_font, 8, 8);
    sdl_ui_set_menu_colors(120, 0);
    sdl_ui_set_menu_borders(0, 55);
    return 0;
}

void plus4ui_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);

    lib_free(plus4_font);
}

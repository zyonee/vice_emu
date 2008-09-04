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
#include "ui.h"
#include "uimenu.h"
#include "vic20memrom.h"

/* temporary place holder for the autostart callback till we get file selectors. */
static UI_MENU_CALLBACK(autostart_callback)
{
    return 0;
}

/* temporary empty drive menu, this one will be moved out to uimenu_drive.c */
static ui_menu_entry_t drive_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty tape menu, this one will be moved out to uimenu_tape.c */
static ui_menu_entry_t tape_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty vic20 cart menu, this one will be moved out to uimenu_vic20_cart.c */
static ui_menu_entry_t vic20_cart_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty vic20 hardware menu, this one will be moved out to uimenu_vic20hw.c */
static ui_menu_entry_t vic20_hardware_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty vic20 rom menu, this one will be moved out to uimenu_vic20rom.c */
static ui_menu_entry_t vic20_rom_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty vic20 video menu, this one will be moved out to uimenu_vic20video.c */
static ui_menu_entry_t vic20_video_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty sound menu, this one will be moved out to uimenu_sound.c */
static ui_menu_entry_t sound_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty snapshot menu, this one will be moved out to uimenu_snapshot.c */
static ui_menu_entry_t snapshot_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty speed menu, this one will be moved out to uimenu_speed.c */
static ui_menu_entry_t speed_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty reset menu, this one will be moved out to uimenu_reset.c */
static ui_menu_entry_t reset_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary place holder for the pause callback till we can get it working. */
static UI_MENU_CALLBACK(pause_callback)
{
    return 0;
}

/* temporary place holder for the monitor callback till we can get it working. */
static UI_MENU_CALLBACK(monitor_callback)
{
    return 0;
}

#ifdef DEBUG
/* temporary empty debug menu, this one will be moved out to uimenu_debug.c */
static ui_menu_entry_t debug_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};
#endif

/* temporary empty help menu, this one will be moved out to uimenu_help.c */
static ui_menu_entry_t help_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* this callback will be moved out to uimenu_common.c */
static UI_MENU_CALLBACK(quit_callback)
{
    exit(0);
    return 0;
}

static ui_menu_entry_t xvic_main_menu[] = {
    { "Autostart disk, tape or program",
      MENU_ENTRY_OTHER,
      autostart_callback,
      NULL,
      NULL },
    { "Drive menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      drive_menu },
    { "Tape menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      tape_menu },
    { "Cartridge menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      vic20_cart_menu },
    { "Machine settings menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      vic20_hardware_menu },
    { "ROM settings menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      vic20_rom_menu },
    { "Video settings menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      vic20_video_menu },
    { "Sound settings menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      sound_menu },
    { "Snapshot menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      snapshot_menu },
    { "Speed settings menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      speed_menu },
    { "Reset menu",
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
    { "Debug menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      debug_menu },
#endif
    { "Help menu",
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

BYTE vic20_font[8*256];

int vic20ui_init(void)
{
    int i, j;
fprintf(stderr,"%s\n",__func__);

    sdl_register_vcachename("VICVideoCache");
    sdl_ui_set_main_menu(xvic_main_menu);

    for (i=0; i<128; i++)
    {
        for (j=0; j<8; j++)
        {
            vic20_font[(i*8)+j]=vic20memrom_chargen_rom[(i*8)+(128*8)+j+0x800];
            vic20_font[(i*8)+(128*8)+j]=vic20memrom_chargen_rom[(i*8)+j+0x800];
        }
    }

    sdl_ui_set_menu_font(vic20_font, 8, 8);
    return 0;
}

void vic20ui_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);
}

/*
 * uic64.c - Implementation of the C64-specific part of the UI.
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
#include "menu_reset.h"
#include "menu_speed.h"
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

/* temporary empty c128/c64 cart menu, this one will be moved out to menu_c64_c128_cart.c */
static ui_menu_entry_t c64_c128_cart_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty c64 hardware menu, this one will be moved out to menu_c64hw.c */
static ui_menu_entry_t c64_hardware_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty c64 rom menu, this one will be moved out to menu_c64rom.c */
static ui_menu_entry_t c64_rom_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty c64 video menu, this one will be moved out to menu_c64video.c */
static ui_menu_entry_t c64_video_menu[] = {
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

static ui_menu_entry_t x64_main_menu[] = {
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
      c64_c128_cart_menu },
    { "Machine settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      c64_hardware_menu },
    { "ROM settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      c64_rom_menu },
    { "Video settings",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      c64_video_menu },
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

int c64ui_init(void)
{
fprintf(stderr,"%s\n",__func__);

    sdl_ui_set_vcachename("VICIIVideoCache");
    sdl_ui_set_main_menu(x64_main_menu);
    sdl_ui_set_menu_font(mem_chargen_rom + 0x800, 8, 8);
    return 0;
}

void c64ui_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);
}

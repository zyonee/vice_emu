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
#include "menu_reset.h"
#include "menu_speed.h"
#include "petmem.h"
#include "resources.h"
#include "ui.h"
#include "uimenu.h"

/* temporary place holder for the autostart callback till we get file selectors. */
static UI_MENU_CALLBACK(autostart_callback)
{
    return NULL;
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

/* temporary empty pet hardware menu, this one will be moved out to uimenu_pethw.c */
static ui_menu_entry_t pet_hardware_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty pet rom menu, this one will be moved out to uimenu_petrom.c */
static ui_menu_entry_t pet_rom_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty pet video menu, this one will be moved out to uimenu_petvideo.c */
static ui_menu_entry_t pet_video_menu[] = {
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
    if(activated) {
        exit(0);
    }
    return NULL;
}

static ui_menu_entry_t xpet_main_menu[] = {
    { "Autostart image",
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
    { "Machine settings menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      pet_hardware_menu },
    { "ROM settings menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      pet_rom_menu },
    { "Video settings menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      pet_video_menu },
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
    sdl_ui_set_vcachename("CrtcVideoCache");
    return 0;
}

void petui_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);

    lib_free(pet_font);
}

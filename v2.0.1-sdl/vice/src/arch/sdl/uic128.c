/*
 * uic128.c - Implementation of the C128-specific part of the UI.
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
#include "c128mem.h"
#include "ui.h"
#include "uimenu.h"

/* the following is only kept as an example for now */
#if 0
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

static ui_menu_entry_t x128_main_menu[] = {
    { "Attach disk",
      MENU_ENTRY_SUBMENU,
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
#endif

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

/* temporary empty c128/c64 cart menu, this one will be moved out to uimenu_c64_c128_cart.c */
static ui_menu_entry_t c64_c128_cart_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty c128 hardware menu, this one will be moved out to uimenu_c128hw.c */
static ui_menu_entry_t c128_hardware_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty c128 rom menu, this one will be moved out to uimenu_c128rom.c */
static ui_menu_entry_t c128_rom_menu[] = {
    { "-", MENU_ENTRY_SEPARATOR, NULL, NULL, NULL },
    { NULL }
};

/* temporary empty c128 video menu, this one will be moved out to uimenu_c128video.c */
static ui_menu_entry_t c128_video_menu[] = {
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

static ui_menu_entry_t x128_main_menu[] = {
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
      c64_c128_cart_menu },
    { "Emulation hardware settings menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      c128_hardware_menu },
    { "ROM settings menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      c128_rom_menu },
    { "Video settings menu",
      MENU_ENTRY_SUBMENU,
      NULL,
      NULL,
      c128_video_menu },
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

int c128ui_init(void)
{
fprintf(stderr,"%s\n",__func__);

    sdl_register_vcachename("VICIIVideoCache");
    sdl_ui_set_main_menu(x128_main_menu);
    sdl_ui_set_menu_font(mem_chargen_rom + 0x800, 8, 8);
    return 0;
}

void c128ui_shutdown(void)
{
fprintf(stderr,"%s\n",__func__);
}

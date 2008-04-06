/*
 * c128ui.c - Implementation of the C128-specific part of the UI.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <boose@linux.rz.fh-hannover.de>
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

#include "drive.h"
#include "joystick.h"
#include "resources.h"
#include "uicommands.h"
#include "uimenu.h"
#include "uisettings.h"
#include "vsync.h"

#ifdef XPM
#include <X11/xpm.h>
#include "x11/xaw/c128icon.xpm"
#endif

/* ------------------------------------------------------------------------- */

UI_MENU_DEFINE_RADIO(VideoStandard)

static ui_menu_entry_t set_video_standard_submenu[] = {
    { "*PAL-G", (ui_callback_t) radio_VideoStandard,
      (ui_callback_data_t) DRIVE_SYNC_PAL, NULL },
    { "*NTSC-M", (ui_callback_t) radio_VideoStandard,
      (ui_callback_data_t) DRIVE_SYNC_NTSC, NULL },
    { NULL }
};

UI_MENU_DEFINE_STRING_RADIO(PaletteFile)

static ui_menu_entry_t palette_submenu[] = {
    { "*Default",
      (ui_callback_t) radio_PaletteFile, (ui_callback_data_t) "default", NULL },
    { "*C64S",
      (ui_callback_t) radio_PaletteFile, (ui_callback_data_t) "c64s", NULL },
    { "*CCS64",
      (ui_callback_t) radio_PaletteFile, (ui_callback_data_t) "ccs64", NULL },
    { "*Frodo",
      (ui_callback_t) radio_PaletteFile, (ui_callback_data_t) "frodo", NULL },
    { "*GoDot",
      (ui_callback_t) radio_PaletteFile, (ui_callback_data_t) "godot", NULL },
    { "*PC64",
      (ui_callback_t) radio_PaletteFile, (ui_callback_data_t) "pc64", NULL },
    { "--" },
    { "Load custom",
      (ui_callback_t) ui_load_palette, NULL, NULL },
    { NULL }
};

UI_MENU_DEFINE_TOGGLE(CheckSsColl)
UI_MENU_DEFINE_TOGGLE(CheckSbColl)

static ui_menu_entry_t vic_submenu[] = {
    { "Video standard",
      NULL, NULL, set_video_standard_submenu },
    { "--",
      NULL, NULL, NULL },
    { "*Sprite-sprite collisions",
      (ui_callback_t) toggle_CheckSsColl, NULL, NULL },
    { "*Sprite-background collisions",
      (ui_callback_t) toggle_CheckSbColl, NULL, NULL },
    { "--",
      NULL, NULL, NULL },
    { "Color set",
      NULL, NULL, palette_submenu },
    { NULL }
};

UI_MENU_DEFINE_TOGGLE(VDC_DoubleSize)
UI_MENU_DEFINE_TOGGLE(VDC_DoubleScan)
UI_MENU_DEFINE_TOGGLE(VDC_64KB)

static ui_menu_entry_t vdc_submenu[] = {
    { "*Double size",
      (ui_callback_t) toggle_VDC_DoubleSize, NULL, NULL },
    { "*Double scan",
      (ui_callback_t) toggle_VDC_DoubleScan, NULL, NULL },
    { "--" },
    { "*64KB display memory",
      (ui_callback_t) toggle_VDC_64KB, NULL, NULL },
    { NULL }
};

/* ------------------------------------------------------------------------- */

UI_MENU_DEFINE_RADIO(SidModel)

static ui_menu_entry_t sid_model_submenu[] = {
    { "*6581 (old)",
      (ui_callback_t) radio_SidModel, (ui_callback_data_t) 0, NULL },
    { "*8580 (new)",
      (ui_callback_t) radio_SidModel, (ui_callback_data_t) 1, NULL },
    { NULL }
};

UI_MENU_DEFINE_TOGGLE(SidFilters)
#ifdef HAVE_RESID
UI_MENU_DEFINE_TOGGLE(SidUseResid)
#endif

static ui_menu_entry_t sid_submenu[] = {
    { "*Emulate filters",
      (ui_callback_t) toggle_SidFilters, NULL, NULL },
    { "Chip model",
      NULL, NULL, sid_model_submenu },
#ifdef HAVE_RESID
    { "--" },
    { "*Use reSID emulation",
      (ui_callback_t) toggle_SidUseResid, NULL, NULL },
#endif
    { NULL },
};

/* ------------------------------------------------------------------------- */

static UI_CALLBACK(set_joystick_device_1)
{
    int tmp;

    suspend_speed_eval();
    if (!CHECK_MENUS) {
        resources_set_value("JoyDevice1", (resource_value_t) UI_MENU_CB_PARAM);
	ui_update_menus();
    } else {
        resources_get_value("JoyDevice1", (resource_value_t *) &tmp);
	ui_menu_set_tick(w, tmp == (int) UI_MENU_CB_PARAM);
    }
}

static UI_CALLBACK(set_joystick_device_2)
{
    int tmp;

    suspend_speed_eval();
    if (!CHECK_MENUS) {
        resources_set_value("JoyDevice2", (resource_value_t) UI_MENU_CB_PARAM);
	ui_update_menus();
    } else {
        resources_get_value("JoyDevice2", (resource_value_t *) &tmp);
	ui_menu_set_tick(w, tmp == (int) UI_MENU_CB_PARAM);
    }
}

static UI_CALLBACK(swap_joystick_ports)
{
    int tmp1, tmp2;

    if (w != NULL)
	suspend_speed_eval();
    resources_get_value("JoyDevice1", (resource_value_t *) &tmp1);
    resources_get_value("JoyDevice2", (resource_value_t *) &tmp2);
    resources_set_value("JoyDevice1", (resource_value_t) tmp2);
    resources_set_value("JoyDevice2", (resource_value_t) tmp1);
    ui_update_menus();
}

static ui_menu_entry_t set_joystick_device_1_submenu[] = {
    { "*None",
      (ui_callback_t) set_joystick_device_1,
      (ui_callback_data_t) JOYDEV_NONE, NULL },
    { "*Numpad",
      (ui_callback_t) set_joystick_device_1,
      (ui_callback_data_t) JOYDEV_NUMPAD, NULL },
    { "*Custom Keys",
      (ui_callback_t) set_joystick_device_1,
      (ui_callback_data_t) JOYDEV_CUSTOM_KEYS, NULL },
#ifdef HAS_JOYSTICK
    { "*Analog Joystick 0",
      (ui_callback_t) set_joystick_device_1,
      (ui_callback_data_t) JOYDEV_ANALOG_0, NULL },
    { "*Analog Joystick 1",
      (ui_callback_t) set_joystick_device_1,
      (ui_callback_data_t) JOYDEV_ANALOG_1, NULL },
#ifdef HAS_DIGITAL_JOYSTICK
    { "*Digital Joystick 0",
      (ui_callback_t) set_joystick_device_1,
      (ui_callback_data_t) JOYDEV_DIGITAL_0, NULL },
    { "*Digital Joystick 1",
      (ui_callback_t) set_joystick_device_1,
      (ui_callback_data_t) JOYDEV_DIGITAL_1, NULL },
#endif
#endif
    { NULL }
};

static ui_menu_entry_t set_joystick_device_2_submenu[] = {
    { "*None",
      (ui_callback_t) set_joystick_device_2,
      (ui_callback_data_t) JOYDEV_NONE, NULL },
    { "*Numpad",
      (ui_callback_t) set_joystick_device_2,
      (ui_callback_data_t) JOYDEV_NUMPAD, NULL },
    { "*Custom Keys",
      (ui_callback_t) set_joystick_device_2,
      (ui_callback_data_t) JOYDEV_CUSTOM_KEYS, NULL },
#ifdef HAS_JOYSTICK
    { "*Analog Joystick 0",
      (ui_callback_t) set_joystick_device_2,
      (ui_callback_data_t) JOYDEV_ANALOG_0, NULL },
    { "*Analog Joystick 1",
      (ui_callback_t) set_joystick_device_2,
      (ui_callback_data_t) JOYDEV_ANALOG_1, NULL },
#ifdef HAS_DIGITAL_JOYSTICK
    { "*Digital Joystick 0",
      (ui_callback_t) set_joystick_device_2,
      (ui_callback_data_t) JOYDEV_DIGITAL_0, NULL },
    { "*Digital Joystick 1",
      (ui_callback_t) set_joystick_device_2,
      (ui_callback_data_t) JOYDEV_DIGITAL_1, NULL },
#endif
#endif /* HAS_JOYSTICK */
    { NULL }
};

static ui_menu_entry_t joystick_settings_submenu[] = {
    { "Joystick device in port 1",
      NULL, NULL, set_joystick_device_1_submenu },
    { "Joystick device in port 2",
      NULL, NULL, set_joystick_device_2_submenu },
    { "--" },
    { "Swap joystick ports",
      (ui_callback_t) swap_joystick_ports, NULL, NULL },
    { NULL }
};

static ui_menu_entry_t joystick_settings_menu[] = {
    { "Joystick settings",
      NULL, NULL, joystick_settings_submenu },
    { NULL }
};

/* ------------------------------------------------------------------------- */

UI_MENU_DEFINE_TOGGLE(IEEE488)
UI_MENU_DEFINE_TOGGLE(EmuID)
UI_MENU_DEFINE_TOGGLE(REU)
UI_MENU_DEFINE_TOGGLE(Mouse)

static ui_menu_entry_t io_extensions_submenu[] = {
    { "*Emulator identification",
      (ui_callback_t) toggle_EmuID, NULL, NULL },
    { "*IEEE488 interface emulation",
      (ui_callback_t) toggle_IEEE488, NULL, NULL },
    { "*512K RAM Expansion Unit",
      (ui_callback_t) toggle_REU, NULL, NULL },
    { "*1351 Mouse Emulation",
      (ui_callback_t) toggle_Mouse, NULL, NULL, XK_m, UI_HOTMOD_META },
    { NULL }
};

/* ------------------------------------------------------------------------- */

static ui_menu_entry_t c128_romset_submenu[] = {
    { "Load default ROMs",
      (ui_callback_t) ui_set_romset,
      (ui_callback_data_t)"default.vrs", NULL },
    { "--" },
    { "Load new Kernal ROM",
      (ui_callback_t) ui_load_rom_file,
      (ui_callback_data_t) "KernalName", NULL },
    { "Load new Basic ROM",
      (ui_callback_t) ui_load_rom_file,
      (ui_callback_data_t) "BasicName", NULL },
    { "Load new Chargen ROM",
      (ui_callback_t) ui_load_rom_file,
      (ui_callback_data_t) "ChargenName", NULL },
    { "--" },
    { "Load new 1541 ROM",
      (ui_callback_t) ui_load_rom_file,
      (ui_callback_data_t)"DosName1541", NULL },
    { "Load new 1541-II ROM",
      (ui_callback_t) ui_load_rom_file,
      (ui_callback_data_t)"DosName1541ii", NULL },
    { "Load new 1571 ROM",
      (ui_callback_t) ui_load_rom_file,
      (ui_callback_data_t)"DosName1571", NULL },
    { "Load new 1581 ROM",
      (ui_callback_t) ui_load_rom_file,
      (ui_callback_data_t)"DosName1581", NULL },
    { "Load new 2031 ROM",
      (ui_callback_t) ui_load_rom_file,
      (ui_callback_data_t)"DosName2031", NULL },
    { "--" },
    { "Load custom ROM set from file",
      (ui_callback_t) ui_load_romset, NULL, NULL },
    { "Dump ROM set definition to file",
      (ui_callback_t) ui_dump_romset, NULL, NULL },
    { NULL }
};

/* ------------------------------------------------------------------------- */

static ui_menu_entry_t c128_menu[] = {
    { "VIC-II settings",
      NULL, NULL, vic_submenu },
    { "VDC settings",
      NULL, NULL, vdc_submenu },
    { "SID settings",
      NULL, NULL, sid_submenu },
    { "I/O extensions at $DFxx",
      NULL, NULL, io_extensions_submenu },
    { "RS232 settings",
      NULL, NULL, rs232_submenu },
    { "Memory settings",
      NULL, NULL, c128_romset_submenu },
    { NULL }
};

int c128_ui_init(void)
{
#ifdef XPM
    {
        Pixmap icon_pixmap;

        /* Create the icon pixmap. */
        XpmCreatePixmapFromData(display, DefaultRootWindow(display),
                                (char **) icon_data, &icon_pixmap, NULL, NULL);
        ui_set_application_icon(icon_pixmap);
    }
#endif

    ui_set_left_menu(ui_menu_create("LeftMenu",
                                    ui_disk_commands_menu,
                                    ui_menu_separator,
                                    ui_tape_commands_menu,
                                    ui_menu_separator,
                                    ui_smart_attach_commands_menu,
                                    ui_menu_separator,
                                    ui_directory_commands_menu,
                                    ui_menu_separator,
                                    ui_snapshot_commands_menu,
                                    ui_menu_separator,
                                    ui_tool_commands_menu,
                                    ui_menu_separator,
                                    ui_help_commands_menu,
                                    ui_menu_separator,
                                    ui_run_commands_menu,
                                    ui_menu_separator,
                                    ui_exit_commands_menu,
                                    NULL));

    ui_set_right_menu(ui_menu_create("RightMenu",
                                     ui_performance_settings_menu,
                                     ui_menu_separator,
                                     ui_video_settings_menu,
#ifdef USE_VIDMODE_EXTENSION
				     ui_fullscreen_settings_menu,
#endif
                                     ui_keyboard_settings_menu,
                                     ui_sound_settings_menu,
                                     ui_drive_settings_menu,
                                     ui_peripheral_settings_menu,
                                     joystick_settings_menu,
                                     ui_menu_separator,
                                     c128_menu,
                                     ui_menu_separator,
                                     ui_settings_settings_menu,
                                     NULL));

    ui_update_menus();

    return 0;
}

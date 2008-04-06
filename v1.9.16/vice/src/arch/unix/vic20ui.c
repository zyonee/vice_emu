/*
 * vic20ui.c - Implementation of the VIC20-specific part of the UI.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
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
#include <unistd.h>

#include "cartridge.h"
#include "debug.h"
#include "icon.h"
#include "joystick.h"
#include "machine.h"
#include "resources.h"
#include "uicommands.h"
#include "uidatasette.h"
#include "uidrive.h"
#include "uipalemu.h"
#include "uipalette.h"
#include "uiperipheral.h"
#include "uimenu.h"
#include "uirs232.h"
#include "uiscreenshot.h"
#include "uisettings.h"
#include "uisound.h"
#include "utils.h"
#include "vsync.h"


enum {
    MEM_NONE,
    MEM_ALL,
    MEM_3K,
    MEM_8K,
    MEM_16K,
    MEM_24K
};

enum {
    BLOCK_0 = 1,
    BLOCK_1 = 1 << 1,
    BLOCK_2 = 1 << 2,
    BLOCK_3 = 1 << 3,
    BLOCK_5 = 1 << 5
};

static UI_CALLBACK(set_common_memory_configuration)
{
    int blocks;

    switch ((int) UI_MENU_CB_PARAM) {
      case MEM_NONE:
        blocks = 0;
        break;
      case MEM_ALL:
        blocks = BLOCK_0 | BLOCK_1 | BLOCK_2 | BLOCK_3 | BLOCK_5;
        break;
      case MEM_3K:
        blocks = BLOCK_0;
        break;
      case MEM_8K:
        blocks = BLOCK_1;
        break;
      case MEM_16K:
        blocks = BLOCK_1 | BLOCK_2;
        break;
      case MEM_24K:
        blocks = BLOCK_1 | BLOCK_2 | BLOCK_3;
        break;
      default:
        /* Shouldn't happen.  */
        fprintf(stderr, _("What?!\n"));
        blocks = 0;         /* Make compiler happy.  */
    }
    resources_set_value("RamBlock0",
                        (resource_value_t)(blocks & BLOCK_0 ? 1 : 0));
    resources_set_value("RamBlock1",
                        (resource_value_t)(blocks & BLOCK_1 ? 1 : 0));
    resources_set_value("RamBlock2",
                        (resource_value_t)(blocks & BLOCK_2 ? 1 : 0));
    resources_set_value("RamBlock3",
                        (resource_value_t)(blocks & BLOCK_3 ? 1 : 0));
    resources_set_value("RamBlock5",
                        (resource_value_t)(blocks & BLOCK_5 ? 1 : 0));
    ui_menu_update_all();
    vsync_suspend_speed_eval();
}

static ui_menu_entry_t vic20_romset_submenu[] = {
    { N_("Load default ROMs"),
      (ui_callback_t)ui_set_romset, (ui_callback_data_t)"default.vrs", NULL },
    { "--" },
    { N_("Load new Kernal ROM"),
      (ui_callback_t)ui_load_rom_file,
      (ui_callback_data_t)"KernalName", NULL },
    { N_("Load new Basic ROM"),
      (ui_callback_t)ui_load_rom_file,
      (ui_callback_data_t)"BasicName", NULL },
    { N_("Load new Character ROM"),
      (ui_callback_t)ui_load_rom_file,
      (ui_callback_data_t)"ChargenName", NULL },
    { "--" },
    { N_("Load custom ROM set from file"),
      (ui_callback_t)ui_load_romset, NULL, NULL },
    { N_("Dump ROM set definition to file"),
      (ui_callback_t)ui_dump_romset, NULL, NULL },
    { NULL }
};

static ui_menu_entry_t common_memory_configurations_submenu[] = {
    { N_("No expansion memory"),
      set_common_memory_configuration, (ui_callback_data_t)MEM_NONE, NULL },
    { "--" },
    { N_("3K (block 0)"),
      set_common_memory_configuration, (ui_callback_data_t)MEM_3K, NULL },
    { N_("8K (block 1)"),
      set_common_memory_configuration, (ui_callback_data_t)MEM_8K, NULL },
    { N_("16K (blocks 1/2)"),
      set_common_memory_configuration, (ui_callback_data_t)MEM_16K, NULL },
    { N_("24K (blocks 1/2/3)"),
      set_common_memory_configuration, (ui_callback_data_t)MEM_24K, NULL },
    { "--" },
    { N_("All (blocks 0/1/2/3/5)"),
      set_common_memory_configuration, (ui_callback_data_t)MEM_ALL, NULL },
    { NULL }
};

UI_MENU_DEFINE_TOGGLE(RAMBlock0)
UI_MENU_DEFINE_TOGGLE(RAMBlock1)
UI_MENU_DEFINE_TOGGLE(RAMBlock2)
UI_MENU_DEFINE_TOGGLE(RAMBlock3)
UI_MENU_DEFINE_TOGGLE(RAMBlock5)

UI_MENU_DEFINE_TOGGLE(EmuID)

static ui_menu_entry_t memory_settings_submenu[] = {
    { N_("Common configurations"),
      NULL, NULL, common_memory_configurations_submenu },
    { "--" },
    { N_("ROM sets"),
      NULL, NULL, vic20_romset_submenu },
    { "--" },
    { N_("*Block 0 (3K at $0400-$0FFF)"),
      (ui_callback_t)toggle_RAMBlock0, NULL, NULL },
    { N_("*Block 1 (8K at $2000-$3FFF)"),
      (ui_callback_t)toggle_RAMBlock1, NULL, NULL },
    { N_("*Block 2 (8K at $4000-$5FFF)"),
      (ui_callback_t)toggle_RAMBlock2, NULL, NULL },
    { N_("*Block 3 (8K at $6000-$7FFF)"),
      (ui_callback_t)toggle_RAMBlock3, NULL, NULL },
    { N_("*Block 5 (8K at $A000-$BFFF)"),
      (ui_callback_t)toggle_RAMBlock5, NULL, NULL },
    { "--" },
    { N_("*Emulator identification"),
      (ui_callback_t)toggle_EmuID, NULL, NULL },
    { NULL }
};

static ui_menu_entry_t memory_settings_menu[] = {
    { N_("Memory expansions"),
      NULL, NULL, memory_settings_submenu },
    { NULL }
};


/* ------------------------------------------------------------------------- */

static UI_CALLBACK(attach_cartridge)
{
    int type = (int)UI_MENU_CB_PARAM;
    char *filename;
    ui_button_t button;
    static char *last_dir;

    vsync_suspend_speed_eval();
    filename = ui_select_file(_("Attach cartridge image"),
                              NULL, False, last_dir, "*.prg", &button, False,
                              NULL);
    switch (button) {
      case UI_BUTTON_OK:
        if (cartridge_attach_image(type, filename) < 0)
            ui_error(_("Invalid cartridge image"));
        if (last_dir)
            free(last_dir);
        util_fname_split(filename, &last_dir, NULL);
        ui_update_menus();
        break;
      default:
        /* Do nothing special.  */
        break;
    }
    if (filename != NULL)
        free(filename);
}

static UI_CALLBACK(detach_cartridge)
{
    cartridge_detach_image();
}

static UI_CALLBACK(default_cartridge)
{
    cartridge_set_default();
}

static ui_menu_entry_t attach_cartridge_image_submenu[] = {
    { N_("Smart-attach cartridge image..."),
      (ui_callback_t)attach_cartridge,
      (ui_callback_data_t)CARTRIDGE_VIC20_DETECT, NULL,
      XK_c, UI_HOTMOD_META },
    { "--" },
    { N_("Attach 4/8/16KB image at $2000..."),
      (ui_callback_t)attach_cartridge,
      (ui_callback_data_t)CARTRIDGE_VIC20_16KB_2000, NULL },
    { N_("Attach 4/8/16KB image at $4000..."),
      (ui_callback_t)attach_cartridge,
      (ui_callback_data_t)CARTRIDGE_VIC20_16KB_4000, NULL },
    { N_("Attach 4/8/16KB image at $6000..."),
      (ui_callback_t)attach_cartridge,
      (ui_callback_data_t)CARTRIDGE_VIC20_16KB_6000, NULL },
    { N_("Attach 4/8KB image at $A000..."),
      (ui_callback_t)attach_cartridge,
      (ui_callback_data_t)CARTRIDGE_VIC20_8KB_A000, NULL },
    { N_("Attach 4KB image at $B000..."),
      (ui_callback_t)attach_cartridge,
      (ui_callback_data_t)CARTRIDGE_VIC20_4KB_B000, NULL },
    { "--" },
    { N_("Set cartridge as default"),
      (ui_callback_t)default_cartridge, NULL, NULL },
    { NULL }
};

static ui_menu_entry_t vic20_cartridge_commands_menu[] = {
    { N_("Attach a cartridge image"),
      NULL, NULL, attach_cartridge_image_submenu },
    { N_("Detach cartridge image(s)"),
      (ui_callback_t)detach_cartridge, NULL, NULL },
    { NULL }
};


/* ------------------------------------------------------------------------- */

static UI_CALLBACK(set_joystick_device)
{
    vsync_suspend_speed_eval();
    if (!CHECK_MENUS) {
        resources_set_value("JoyDevice1", (resource_value_t)UI_MENU_CB_PARAM);
        ui_update_menus();
    } else {
        int tmp;

        resources_get_value("JoyDevice1", (resource_value_t *)&tmp);
        ui_menu_set_tick(w, tmp == (int)UI_MENU_CB_PARAM);
    }
}

static ui_menu_entry_t set_joystick_device_1_submenu[] = {
    { N_("*None"),
      (ui_callback_t)set_joystick_device,
      (ui_callback_data_t)JOYDEV_NONE, NULL },
    { N_("*Numpad"),
      (ui_callback_t)set_joystick_device,
      (ui_callback_data_t)JOYDEV_NUMPAD, NULL },
    { N_("*Custom Keys"),
      (ui_callback_t)set_joystick_device,
      (ui_callback_data_t)JOYDEV_CUSTOM_KEYS, NULL },
#ifdef HAS_JOYSTICK
    { N_("*Analog Joystick 0"),
      (ui_callback_t)set_joystick_device,
      (ui_callback_data_t)JOYDEV_ANALOG_0, NULL },
    { N_("*Analog Joystick 1"),
      (ui_callback_t)set_joystick_device,
      (ui_callback_data_t)JOYDEV_ANALOG_1, NULL },
#ifdef HAS_DIGITAL_JOYSTICK
    { N_("*Digital Joystick 0"),
      (ui_callback_t)set_joystick_device,
      (ui_callback_data_t)JOYDEV_DIGITAL_0, NULL },
    { N_("*Digital Joystick 1"),
      (ui_callback_t)set_joystick_device,
      (ui_callback_data_t)JOYDEV_DIGITAL_1, NULL },
#endif
#endif
    { NULL }
};

static ui_menu_entry_t joystick_settings_menu[] = {
    { N_("Joystick settings"),
      NULL, NULL, set_joystick_device_1_submenu },
    { NULL }
};

/*------------------------------------------------------------*/

UI_MENU_DEFINE_RADIO(RsUser)

static ui_menu_entry_t vic20_rs232_submenu[] = {
    { N_("*No Userport RS232 emulation"),
      (ui_callback_t)radio_RsUser, (ui_callback_data_t)0, NULL },
    { N_("*Userport 300 baud RS232 emulation"),
      (ui_callback_t)radio_RsUser, (ui_callback_data_t)300, NULL },
    { N_("*Userport 1200 baud RS232 emulation"),
      (ui_callback_t)radio_RsUser, (ui_callback_data_t)1200, NULL },
    { N_("Userport RS232 device"),
      NULL, NULL, rsuser_device_submenu },
    { "--" },
    { N_("Serial 1 device..."), (ui_callback_t)set_rs232_device_file,
      (ui_callback_data_t)"RsDevice1", NULL },
    { N_("Serial 1 baudrate"),
      NULL, NULL, ser1_baud_submenu },
    { "--" },
    { N_("Serial 2 device..."), (ui_callback_t)set_rs232_device_file,
      (ui_callback_data_t)"RsDevice2", NULL },
    { N_("Serial 2 baudrate"),
      NULL, NULL, ser2_baud_submenu },
    { "--" },
    { N_("Dump filename..."), (ui_callback_t)set_rs232_dump_file,
      (ui_callback_data_t)"RsDevice3", NULL },
    { "--" },
    { N_("Program name to exec..."), (ui_callback_t)set_rs232_exec_file,
      (ui_callback_data_t)"RsDevice4", NULL },
    { NULL }
};

static ui_menu_entry_t rs232_settings_menu[] = {
    { N_("RS232 settings"),
      NULL, NULL, vic20_rs232_submenu },
    { NULL }
};

/*------------------------------------------------------------*/

UI_MENU_DEFINE_TOGGLE(IEEE488)

/*
static ui_menu_entry_t vic20_io_submenu[] = {
    { "*VIC-1112 IEEE 488 module",
      (ui_callback_t)toggle_IEEE488, NULL, NULL },
    { NULL }
};

static ui_menu_entry_t vic20_io_settings_menu[] = {
    { "I/O settings",
      NULL, NULL, vic20_io_submenu },
    { NULL }
};
*/

/*------------------------------------------------------------*/

static UI_CALLBACK(save_screenshot)
{
    /* Where does the 1024 come from?  */
    char filename[1024];
    int wid = (int)UI_MENU_CB_PARAM;

    vsync_suspend_speed_eval();

    /* The following code depends on a zeroed filename.  */
    memset(filename, 0, 1024);

    if (ui_screenshot_dialog(filename, wid) < 0)
        return;
}

static ui_menu_entry_t ui_screenshot_commands_menu[] = {
    { N_("Screenshot..."),
      (ui_callback_t)save_screenshot, (ui_callback_data_t)0, NULL },
    { NULL }
};

/* ------------------------------------------------------------------------- */

UI_MENU_DEFINE_RADIO(MachineVideoStandard)

static ui_menu_entry_t set_video_standard_submenu[] = {
    { N_("*PAL-G"), (ui_callback_t)radio_MachineVideoStandard,
      (ui_callback_data_t)MACHINE_SYNC_PAL, NULL },
    { N_("*NTSC-M"), (ui_callback_t)radio_MachineVideoStandard,
      (ui_callback_data_t)MACHINE_SYNC_NTSC, NULL },
    { NULL }
};

UI_MENU_DEFINE_STRING_RADIO(PaletteFile)

static ui_menu_entry_t palette_submenu[] = {
    { N_("*Default"),
      (ui_callback_t)radio_PaletteFile, (ui_callback_data_t)"default", NULL },
    { N_("Load custom"),
      (ui_callback_t)ui_load_palette,
      (ui_callback_data_t)"PaletteFile", NULL },
    { NULL }
};

static ui_menu_entry_t vic_submenu[] = {
    { N_("Video standard"),
      NULL, NULL, set_video_standard_submenu },
    { "--" },
    { N_("Color set"),
      NULL, NULL, palette_submenu },
    { "--" },
    { N_("PAL Emulation Settings"),
      NULL, NULL, PALMode_submenu },
    { NULL }
};

static ui_menu_entry_t vic20_menu[] = {
    { N_("VIC settings"),
      NULL, NULL, vic_submenu },
    { N_("Memory expansions"),
      NULL, NULL, memory_settings_submenu },
    { NULL }
};

/* ------------------------------------------------------------------------- */

int vic20_ui_init(void)
{
    ui_set_application_icon(vic20_icon_data);
    ui_set_left_menu(ui_menu_create("LeftMenu",
                                    ui_disk_commands_menu,
                                    ui_menu_separator,
                                    ui_tape_commands_menu,
                                    ui_datasette_commands_menu,
                                    ui_menu_separator,
                                    ui_smart_attach_commands_menu,
                                    ui_menu_separator,
                                    vic20_cartridge_commands_menu,
                                    ui_menu_separator,
                                    ui_directory_commands_menu,
                                    ui_menu_separator,
                                    ui_snapshot_commands_menu,
                                    ui_screenshot_commands_menu,
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
                                     ui_vic_video_settings_menu,
#ifdef USE_XF86_EXTENSIONS
                                     ui_fullscreen_settings_menu,
#endif
                                     ui_keyboard_settings_menu,
                                     ui_sound_settings_menu,
                                     ui_drive_settings_menu,
                                     ui_peripheral_settings_menu,
/*
                                     vic20_io_settings_menu,
*/
                                     joystick_settings_menu,
                                     rs232_settings_menu,
                                     ui_menu_separator,
                                     vic20_menu,
                                     ui_menu_separator,
                                     ui_settings_settings_menu,
#ifdef DEBUG
                                     ui_menu_separator,
                                     ui_debug_settings_menu,
#endif
                                     NULL));

    ui_set_tape_menu(ui_menu_create("TapeMenu",
                                    ui_tape_commands_menu,
                                    ui_menu_separator,
                                    datasette_control_submenu,
                                    NULL));
    ui_set_topmenu("TopLevelMenu",
                   _("File"),
                   ui_menu_create("File",
                                  ui_smart_attach_commands_menu,
                                  ui_menu_separator,
                                  ui_disk_commands_menu,
                                  ui_menu_separator,
                                  ui_tape_commands_menu,
                                  ui_datasette_commands_menu,
                                  ui_menu_separator,
                                  vic20_cartridge_commands_menu,
                                  ui_menu_separator,
                                  ui_directory_commands_menu,
                                  ui_menu_separator,
                                  ui_tool_commands_menu,
                                  ui_menu_separator,
                                  ui_run_commands_menu,
                                  ui_menu_separator,
                                  ui_exit_commands_menu,
                                  NULL),
                   _("Snapshot"),
                   ui_menu_create("Snapshot",
                                  ui_snapshot_commands_submenu,
                                  ui_menu_separator,
                                  ui_screenshot_commands_menu,
                                  NULL),
                   _("Options"),
                   ui_menu_create("Options",
                                  ui_performance_settings_menu,
                                  ui_menu_separator,
#ifdef USE_XF86_EXTENSIONS
                                  ui_fullscreen_settings_menu,
                                  ui_menu_separator,
#endif
                                  ui_drive_options_submenu,
                                  NULL),
                   _("Settings"),
                   ui_menu_create("Settings",
                                  ui_vic_video_settings_menu,
                                  ui_peripheral_settings_menu,
                                  ui_drive_settings_menu,
                                  ui_keyboard_settings_menu,
                                  joystick_settings_menu,
                                  ui_sound_settings_menu,
                                  ui_menu_separator,
                                  rs232_settings_menu,
                                  ui_menu_separator,
                                  memory_settings_menu,
                                  ui_menu_separator,
                                  ui_settings_settings_menu,
                                  NULL),
                   /* Translators: RJ means right justify and should be
                      saved in your tranlation! e.g. german "RJHilfe" */
                   _("RJHelp"),
                   ui_menu_create("Help",
                                  ui_help_commands_menu,
                                  NULL),
                   NULL);
    ui_set_speedmenu(ui_menu_create("SpeedMenu",
                                    ui_performance_settings_menu,
                                    NULL));
    ui_update_menus();

    return 0;
}


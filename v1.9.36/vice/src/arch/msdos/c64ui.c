/*
 * c64ui.c - Definition of the C64-specific part of the UI.
 *
 * Written by
 *  Ettore Perazzoli <ettore@comm2000.it>
 *  Andreas Boose <viceteam@t-online.de>
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
#include <string.h>

#include "c64ui.h"
#include "cartridge.h"
#include "keyboard.h"
#include "machine.h"
#include "menudefs.h"
#include "resources.h"
#include "sidui.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "types.h"
#include "ui.h"
#include "utils.h"

/* ------------------------------------------------------------------------- */

static TUI_MENU_CALLBACK(attach_cartridge_callback)
{
    if (been_activated) {
        char *default_item, *directory;
        char *name;
        const char *s, *filter;
        int type = (int)param;

        s = cartridge_get_file_name((ADDRESS) 0);
        util_fname_split(s, &directory, &default_item);

        filter = (type == CARTRIDGE_CRT) ? "*.crt" : "*";

        name = tui_file_selector("Attach cartridge image",
                                 directory, filter, default_item, NULL, NULL,
                                 NULL);
        if (name != NULL
            && (s == NULL || strcasecmp(name, s) != 0)
            && cartridge_attach_image(type, name) < 0)
            tui_error("Invalid cartridge image.");
        ui_update_menus();
        free(name);
    }

    return NULL;
}

static TUI_MENU_CALLBACK(cartridge_set_default_callback)
{
    if (been_activated)
        cartridge_set_default();

    return NULL;
}


static TUI_MENU_CALLBACK(cartridge_callback)
{
    const char *s = cartridge_get_file_name((ADDRESS) 0);

    if (s == NULL || *s == '\0')
        return "(none)";
    else
        return s;
}

static tui_menu_item_def_t attach_cartridge_submenu_items[] = {
    { "Attach _CRT Image...",
      "Attach a CRT image, autodetecting its type",
      attach_cartridge_callback, (void *)CARTRIDGE_CRT, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach Generic _8KB Image...",
      "Attach a generic 8KB cartridge dump",
      attach_cartridge_callback, (void *)CARTRIDGE_GENERIC_8KB, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach Generic _16KB Image...",
      "Attach a generic 16KB cartridge dump",
      attach_cartridge_callback, (void *)CARTRIDGE_GENERIC_16KB, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach _Action Replay Image...",
      "Attach an Action Replay cartridge image",
      attach_cartridge_callback, (void *)CARTRIDGE_ACTION_REPLAY, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach Atomic _Power Image...",
      "Attach an Atomic Power cartridge image",
      attach_cartridge_callback, (void *)CARTRIDGE_ATOMIC_POWER, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach _Epyx fastload Image...",
      "Attach an Epyx fastload cartridge image",
      attach_cartridge_callback, (void *)CARTRIDGE_EPYX_FASTLOAD, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach _IEEE488 Interface Image...",
      "Attach an IEEE488 interface cartridge image",
      attach_cartridge_callback, (void *)CARTRIDGE_IEEE488, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach _Retro Replay Image...",
      "Attach a Retro Replay cartridge image",
      attach_cartridge_callback, (void *)CARTRIDGE_RETRO_REPLAY, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach IDE_64 Interface Image...",
      "Attach an IDE64 interface cartridge image",
      attach_cartridge_callback, (void *)CARTRIDGE_IDE64, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach Super Snapshot _4 Image...",
      "Attach an Super Snapshot 4 cartridge image",
      attach_cartridge_callback, (void *)CARTRIDGE_SUPER_SNAPSHOT, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Attach Super Snapshot _5 Image...",
      "Attach an Super Snapshot 5 cartridge image",
      attach_cartridge_callback, (void *)CARTRIDGE_SUPER_SNAPSHOT_V5, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "--" },
    { "Set cartridge as _default",
      "Save the current cartridge to the settings",
      cartridge_set_default_callback, NULL, 0,
      TUI_MENU_BEH_CLOSE, NULL, NULL },
    { NULL }
};

static tui_menu_item_def_t attach_cartridge_menu_items[] = {
    { "--" },
    { "_Cartridge:",
      "Attach a cartridge image",
      cartridge_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, attach_cartridge_submenu_items,
      "Attach cartridge" },
    { NULL }
};

static TUI_MENU_CALLBACK(detach_cartridge_callback)
{
    const char *s;

    if (been_activated)
        cartridge_detach_image();

    s = cartridge_get_file_name((ADDRESS) 0);

    if (s == NULL || *s == '\0')
        return "(none)";
    else
        return s;
}

static tui_menu_item_def_t detach_cartridge_menu_items[] = {
    { "--" },
    { "_Cartridge:",
      "Detach attached cartridge image",
      detach_cartridge_callback, NULL, 30,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { NULL }
};

static TUI_MENU_CALLBACK(freeze_cartridge_callback)
{
    if (been_activated) {
        keyboard_clear_keymatrix();
        cartridge_trigger_freeze();
    }
    /* This way, the "Not Really!" item is always the default one.  */
    *become_default = 0;

    return NULL;
}

static tui_menu_item_def_t freeze_cartridge_menu_items[] = {
    { "Cartridge _Freeze",
      "Activates the cartridge's freeze button",
      freeze_cartridge_callback, NULL, 0,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { NULL }
};

/* ------------------------------------------------------------------------- */

TUI_MENU_DEFINE_TOGGLE(VICIIVideoCache)
TUI_MENU_DEFINE_TOGGLE(CheckSsColl)
TUI_MENU_DEFINE_TOGGLE(CheckSbColl)
TUI_MENU_DEFINE_TOGGLE(PALEmulation)

static TUI_MENU_CALLBACK(toggle_MachineVideoStandard_callback)
{
    int value;

    resources_get_value("MachineVideoStandard", (resource_value_t *)&value);

    if (been_activated) {
            if (value == MACHINE_SYNC_PAL)
                value = MACHINE_SYNC_NTSC;
            else if (value == MACHINE_SYNC_NTSC)
                value = MACHINE_SYNC_NTSCOLD;
        else
                value = MACHINE_SYNC_PAL;

        resources_set_value("MachineVideoStandard", (resource_value_t)value);
    }

    switch (value) {
      case MACHINE_SYNC_PAL:
        return "PAL-G";
      case MACHINE_SYNC_NTSC:
        return "NTSC-M";
      case MACHINE_SYNC_NTSCOLD:
        return "old NTSC-M";
      default:
        return "(Custom)";
    }
}

static TUI_MENU_CALLBACK(toggle_PALMode_callback)
{
    int value;

    resources_get_value("PALMode", (resource_value_t *) &value);

    if (been_activated) {
        value = (value + 1) % 3;
        resources_set_value("PALMode", (resource_value_t) value);
    }

    switch (value) {
      case 0:
        return "Fast PAL";
      case 1:
        return "Y/C cable (sharp)";
      case 2:
        return "Composite (blurry)";
      default:
        return "unknown";
    }
}

static tui_menu_item_def_t vic_ii_menu_items[] = {
    { "Video _Cache:",
      "Enable screen cache (disabled when using triple buffering)",
      toggle_VICIIVideoCache_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_PAL Emulation:",
      "Enable PAL emulation",
      toggle_PALEmulation_callback, NULL, 3,
      TUI_MENU_BEH_RESUME, NULL, NULL },
    { "PAL _Mode:",
      "Change PAL Mode",
      toggle_PALMode_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "--" },
    { "Sprite-_Background Collisions:",
      "Emulate sprite-background collision register",
      toggle_CheckSbColl_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Sprite-_Sprite Collisions:",
      "Emulate sprite-sprite collision register",
      toggle_CheckSsColl_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "V_ideo Standard:",
      "Select machine clock ratio",
      toggle_MachineVideoStandard_callback, NULL, 11,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { NULL }
};

/* ------------------------------------------------------------------------- */

TUI_MENU_DEFINE_TOGGLE(Mouse)
TUI_MENU_DEFINE_TOGGLE(REU)
TUI_MENU_DEFINE_TOGGLE(EmuID)

static tui_menu_item_def_t special_menu_items[] = {
    { "--" },
    { "1351 _Mouse Emulation:",
      "Emulate a Commodore 1351 proportional mouse connected to joystick port #1",
      toggle_Mouse_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "512K _RAM Expansion Unit (C1750):",
      "Emulate auxiliary 512K RAM Expansion Unit",
      toggle_REU_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_Emulator Identification:",
      "Allow programs to identify the emulator they are running on",
      toggle_EmuID_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { NULL }
};

/* ------------------------------------------------------------------------- */

static struct {
    const char *name;
    const char *brief_description;
    const char *menu_item;
    const char *long_description;
} palette_items[] = {
    { "default", "Default", "_Default",
      "Default VICE C64 palette" },
    { "c64s", "C64S", "C64_S",
      "Palette from the C64S emulator by Miha Peternel" },
    { "ccs64", "CCS64", "_CCS64",
      "Palette from the CCS64 emulator by Per Hakan Sundell" },
    { "frodo", "Frodo", "_Frodo",
      "Palette from the Frodo emulator by Christian Bauer" },
    { "godot", "GoDot", "_GoDot",
      "Palette as suggested by the authors of the GoDot C64 graphics package" },    { "pc64", "PC64", "_PC64",
      "Palette from the PC64 emulator by Wolfgang Lorenz" },
    { NULL }
};

static TUI_MENU_CALLBACK(palette_callback)
{
    if (been_activated) {
        if (resources_set_value("PaletteFile", (resource_value_t *)param) < 0)
           tui_error("Invalid palette file");
        ui_update_menus();
    }
    return NULL;
}

static TUI_MENU_CALLBACK(custom_palette_callback)
{
    if (been_activated) {
        char *name;

        name = tui_file_selector("Load custom palette",
                                 NULL, "*.vpl", NULL, NULL, NULL, NULL);

        if (name != NULL) {
            if (resources_set_value("PaletteFile", (resource_value_t *)name)
                < 0)
                tui_error("Invalid palette file");
            ui_update_menus();
            free(name);
        }
    }
    return NULL;
}

static TUI_MENU_CALLBACK(palette_menu_callback)
{
    char *s;
    int i;

    resources_get_value("PaletteFile", (resource_value_t *)&s);
    for (i = 0; palette_items[i].name != NULL; i++) {
        if (strcmp(s, palette_items[i].name) == 0)
           return palette_items[i].brief_description;
    }
    return "Custom";
}

TUI_MENU_DEFINE_TOGGLE(ExternalPalette)

static void add_palette_submenu(tui_menu_t parent)
{
    int i;
    tui_menu_t palette_menu = tui_menu_create("Color Set", 1);

    for (i = 0; palette_items[i].name != NULL; i++)
        tui_menu_add_item(palette_menu,
                          palette_items[i].menu_item,
                          palette_items[i].long_description,
                          palette_callback,
                          (void *) palette_items[i].name, 0,
                          TUI_MENU_BEH_RESUME);

    tui_menu_add_item(palette_menu,
                      "C_ustom",
                      "Load a custom palette",
                      custom_palette_callback,
                      NULL, 0,
                      TUI_MENU_BEH_RESUME);

    tui_menu_add_item(parent, "Use external Palette",
                      "Use the palette file below",
                      toggle_ExternalPalette_callback,
                      NULL, 3, TUI_MENU_BEH_RESUME);

    tui_menu_add_submenu(parent, "Color _Palette:",
                         "Choose color palette",
                         palette_menu,
                         palette_menu_callback,
                         NULL,
                         10);
}

/* ------------------------------------------------------------------------- */

static TUI_MENU_CALLBACK(load_rom_file_callback)
{
    if (been_activated) {
        char *name;

        name = tui_file_selector("Load ROM file",
                                 NULL, "*", NULL, NULL, NULL, NULL);

        if (name != NULL) {
            if (resources_set_value(param, (resource_value_t)name) < 0)
                ui_error("Could not load ROM file '%s'", name);
            free(name);
        }
    }
    return NULL;
}

static tui_menu_item_def_t rom_menu_items[] = {
    { "--" },
    { "Load new _Kernal ROM...",
      "Load new Kernal ROM",
      load_rom_file_callback, "KernalName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new _BASIC ROM...",
      "Load new BASIC ROM",
      load_rom_file_callback, "BasicName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new _Character ROM...",
      "Load new Character ROM",
      load_rom_file_callback, "ChargenName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 15_41 ROM...",
      "Load new 1541 ROM",
      load_rom_file_callback, "DosName1541", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1541-_II ROM...",
      "Load new 1541-II ROM",
      load_rom_file_callback, "DosName1541ii", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 15_71 ROM...",
      "Load new 1571 ROM",
      load_rom_file_callback, "DosName1571", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 15_81 ROM...",
      "Load new 1581 ROM",
      load_rom_file_callback, "DosName1581", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new _2031 ROM...",
      "Load new 2031 ROM",
      load_rom_file_callback, "DosName2031", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new _1001 ROM...",
      "Load new 1001 ROM",
      load_rom_file_callback, "DosName1001", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { NULL }
};

/* ------------------------------------------------------------------------- */

int c64_ui_init(void)
{
    ui_create_main_menu(1, 1, 1, 2, 1);

    tui_menu_add(ui_attach_submenu, attach_cartridge_menu_items);
    tui_menu_add(ui_detach_submenu, detach_cartridge_menu_items);
    tui_menu_add(ui_reset_submenu, freeze_cartridge_menu_items);
    tui_menu_add_separator(ui_video_submenu);

    add_palette_submenu(ui_video_submenu);

    tui_menu_add(ui_video_submenu, vic_ii_menu_items);
    tui_menu_add(ui_sound_submenu, sid_ui_menu_items);
    tui_menu_add(ui_special_submenu, special_menu_items);
    tui_menu_add(ui_rom_submenu, rom_menu_items);

    return 0;
}


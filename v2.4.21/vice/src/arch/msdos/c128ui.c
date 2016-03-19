/*
 * c128ui.c - Definition of the C128-specific part of the UI.
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

#include "c128ui.h"
#include "cartridge.h"
#include "lib.h"
#include "machine.h"
#include "menudefs.h"
#include "mouse.h"
#include "resources.h"
#include "tui.h"
#include "tuifs.h"
#include "tuimenu.h"
#include "ui.h"
#include "uic64cart.h"
#include "uic128model.h"
#include "uiciamodel.h"
#include "uidigimax.h"
#include "uidrive.h"
#include "uids12c887rtc.h"
#include "uieasyflash.h"
#include "uiexpert.h"
#include "uigeoram.h"
#include "uiide64.h"
#include "uilightpen.h"
#include "uimagicvoice.h"
#include "uimmc64.h"
#include "uimmcreplay.h"
#include "uiramcart.h"
#include "uireu.h"
#include "uisidc128.h"
#include "uisoundexpander.h"
#ifdef HAVE_TFE
#include "uitfe.h"
#endif
#include "uivideo.h"

TUI_MENU_DEFINE_TOGGLE(Mouse)
TUI_MENU_DEFINE_TOGGLE(C128FullBanks)
TUI_MENU_DEFINE_TOGGLE(SmartMouseRTCSave)

TUI_MENU_DEFINE_TOGGLE(InternalFunctionROMRTCSave)
TUI_MENU_DEFINE_TOGGLE(ExternalFunctionROMRTCSave)

TUI_MENU_DEFINE_RADIO(Mousetype)

static TUI_MENU_CALLBACK(MouseType_callback)
{
    int value;

    resources_get_int("Mousetype", &value);

    switch (value) {
        case MOUSE_TYPE_1351:
            return "1351";
        case MOUSE_TYPE_NEOS:
            return "NEOS";
        case MOUSE_TYPE_AMIGA:
            return "AMIGA";
        case MOUSE_TYPE_PADDLE:
            return "PADDLE";
        case MOUSE_TYPE_CX22:
            return "Atari CX-22";
        case MOUSE_TYPE_ST:
            return "Atari ST";
        case MOUSE_TYPE_SMART:
            return "Smart";
        case MOUSE_TYPE_MICROMYS:
            return "MicroMys";
        case MOUSE_TYPE_KOALAPAD:
            return "Koalapad";
        default:
            return "unknown";
    }
}

static tui_menu_item_def_t mouse_type_submenu[] = {
    { "1351", NULL, radio_Mousetype_callback,
      (void *)MOUSE_TYPE_1351, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "NEOS", NULL, radio_Mousetype_callback,
      (void *)MOUSE_TYPE_NEOS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "AMIGA", NULL, radio_Mousetype_callback,
      (void *)MOUSE_TYPE_AMIGA, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "PADDLE", NULL, radio_Mousetype_callback,
      (void *)MOUSE_TYPE_PADDLE, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Atari CX-22", NULL, radio_Mousetype_callback,
      (void *)MOUSE_TYPE_CX22, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Atari ST", NULL, radio_Mousetype_callback,
      (void *)MOUSE_TYPE_ST, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Smart", NULL, radio_Mousetype_callback,
      (void *)MOUSE_TYPE_SMART, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "MicroMys", NULL, radio_Mousetype_callback,
      (void *)MOUSE_TYPE_MICROMYS, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Koalapad", NULL, radio_Mousetype_callback,
      (void *)MOUSE_TYPE_KOALAPAD, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { NULL }
};

TUI_MENU_DEFINE_RADIO(Mouseport)

static TUI_MENU_CALLBACK(MousePort_callback)
{
    int value;

    resources_get_int("Mouseport", &value);
    value--;

    if (been_activated) {
        value = (value + 1) % 2;
        resources_set_int("Mouseport", value + 1);
    }

    switch (value) {
        case 0:
            return "Joy1";
        case 1:
            return "Joy2";
        default:
            return "unknown";
    }
}

static tui_menu_item_def_t mouse_port_submenu[] = {
    { "Joy1", NULL, radio_Mouseport_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "Joy2", NULL, radio_Mouseport_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { NULL }
};

TUI_MENU_DEFINE_RADIO(InternalFunctionROM)

static TUI_MENU_CALLBACK(InternalFunctionROM_callback)
{
    int value;

    resources_get_int("InternalFunctionROM", &value);

    switch (value) {
        case 0:
            return "Off";
        case 1:
            return "ROM";
        case 2:
            return "RAM";
        case 3:
            return "RAM+RTC";
        default:
            return "unknown";
    }
}

static tui_menu_item_def_t int_function_rom_submenu[] = {
    { "Off", NULL, radio_InternalFunctionROM_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "ROM", NULL, radio_InternalFunctionROM_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "RAM", NULL, radio_InternalFunctionROM_callback,
      (void *)2, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "RAM+RTC", NULL, radio_InternalFunctionROM_callback,
      (void *)3, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { NULL }
};

TUI_MENU_DEFINE_RADIO(ExternalFunctionROM)

static TUI_MENU_CALLBACK(ExternalFunctionROM_callback)
{
    int value;

    resources_get_int("ExternalFunctionROM", &value);

    switch (value) {
        case 0:
            return "Off";
        case 1:
            return "ROM";
        case 2:
            return "RAM";
        case 3:
            return "RAM+RTC";
        default:
            return "unknown";
    }
}

static tui_menu_item_def_t ext_function_rom_submenu[] = {
    { "Off", NULL, radio_ExternalFunctionROM_callback,
      (void *)0, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "ROM", NULL, radio_ExternalFunctionROM_callback,
      (void *)1, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "RAM", NULL, radio_ExternalFunctionROM_callback,
      (void *)2, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { "RAM+RTC", NULL, radio_ExternalFunctionROM_callback,
      (void *)3, 7, TUI_MENU_BEH_CLOSE, NULL, NULL },
    { NULL }
};

static tui_menu_item_def_t ioextenstions_menu_items[] = {
    { "--" },
    { "Mouse Type:",
      "Change Mouse Type",
      MouseType_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, mouse_type_submenu,
      "Mouse Type" },
    { "Mouse Port:",
      "Change Mouse Port",
      MousePort_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, mouse_port_submenu,
      "Mouse Port" },
    { "Save Smart Mouse RTC data when changed",
      "Save Smart Mouse RTC data when changed",
      toggle_SmartMouseRTCSave_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Grab mouse events:",
      "Emulate a mouse",
      toggle_Mouse_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "_RAM banks 2 & 3:",
      "Enable RAM banks 2 & 3",
      toggle_C128FullBanks_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Internal function ROM:",
      "Change internal function ROM type",
      InternalFunctionROM_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, int_function_rom_submenu,
      "Internal function ROM" },
    { "Save Internal function RTC when changed:",
      "Enable saving the Internal function RTC when changed",
      toggle_InternalFunctionROMRTCSave_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "External function ROM:",
      "Change external function ROM type",
      ExternalFunctionROM_callback, NULL, 20,
      TUI_MENU_BEH_CONTINUE, ext_function_rom_submenu,
      "External function ROM" },
    { "Save External function RTC when changed:",
      "Enable saving the External function RTC when changed",
      toggle_ExternalFunctionROMRTCSave_callback, NULL, 3,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { NULL }
};

/* ------------------------------------------------------------------------- */

static TUI_MENU_CALLBACK(load_rom_file_callback)
{
    if (been_activated) {
        char *name;

        name = tui_file_selector("Load ROM file", NULL, "*", NULL, NULL, NULL, NULL);

        if (name != NULL) {
            if (resources_set_string(param, name) < 0) {
                ui_error("Could not load ROM file '%s'", name);
            }
            lib_free(name);
        }
    }
    return NULL;
}

static tui_menu_item_def_t rom_menu_items[] = {
    { "--" },
    { "Load new International Kernal ROM...",
      "Load new International Kernal ROM",
      load_rom_file_callback, "KernalIntName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new German Kernal ROM...",
      "Load new German Kernal ROM",
      load_rom_file_callback, "KernalDEName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new Finnish Kernal ROM...",
      "Load new Finnish Kernal ROM",
      load_rom_file_callback, "KernalFIName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new French Kernal ROM...",
      "Load new French Kernal ROM",
      load_rom_file_callback, "KernalFRName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new Italian Kernal ROM...",
      "Load new Italian Kernal ROM",
      load_rom_file_callback, "KernalITName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new Norwegian Kernal ROM...",
      "Load new Norwegian Kernal ROM",
      load_rom_file_callback, "KernalNOName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new Swedish Kernal ROM...",
      "Load new Swedish Kernal ROM",
      load_rom_file_callback, "KernalSEName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new Swiss Kernal ROM...",
      "Load new Swiss Kernal ROM",
      load_rom_file_callback, "KernalCHName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new C64 Kernal ROM...",
      "Load new C64 Kernal ROM",
      load_rom_file_callback, "Kernal64Name", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new BASIC LO ROM...",
      "Load new BASIC LO ROM",
      load_rom_file_callback, "BasicLoName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new BASIC HI ROM...",
      "Load new BASIC HI ROM",
      load_rom_file_callback, "BasicHiName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new C64 BASIC ROM...",
      "Load new C64 BASIC ROM",
      load_rom_file_callback, "Basic64Name", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new International Character ROM...",
      "Load new International Character ROM",
      load_rom_file_callback, "ChargenIntName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new German Character ROM...",
      "Load new German Character ROM",
      load_rom_file_callback, "ChargenDEName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new French Character ROM...",
      "Load new French Character ROM",
      load_rom_file_callback, "ChargenFRName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new Swedish Character ROM...",
      "Load new Swedish Character ROM",
      load_rom_file_callback, "ChargenSEName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new Swiss Character ROM...",
      "Load new Swiss Character ROM",
      load_rom_file_callback, "ChargenCHName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "--" },
    { "Load new 1540 ROM...",
      "Load new 1540 ROM",
      load_rom_file_callback, "DosName1540", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1541 ROM...",
      "Load new 1541 ROM",
      load_rom_file_callback, "DosName1541", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1541-II ROM...",
      "Load new 1541-II ROM",
      load_rom_file_callback, "DosName1541ii", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1570 ROM...",
      "Load new 1570 ROM",
      load_rom_file_callback, "DosName1570", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1571 ROM...",
      "Load new 1571 ROM",
      load_rom_file_callback, "DosName1571", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1571CR ROM...",
      "Load new 1571CR ROM",
      load_rom_file_callback, "DosName1571cr", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1581 ROM...",
      "Load new 1581 ROM",
      load_rom_file_callback, "DosName1581", 0,
       TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 2000 ROM...",
      "Load new 2000 ROM",
      load_rom_file_callback, "DosName2000", 0,
       TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 4000 ROM...",
      "Load new 4000 ROM",
      load_rom_file_callback, "DosName4000", 0,
       TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 2031 ROM...",
      "Load new 2031 ROM",
      load_rom_file_callback, "DosName2031", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 2040 ROM...",
      "Load new 2040 ROM",
      load_rom_file_callback, "DosName2040", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 3040 ROM...",
      "Load new 3040 ROM",
      load_rom_file_callback, "DosName3040", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 4040 ROM...",
      "Load new 4040 ROM",
      load_rom_file_callback, "DosName4040", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new 1001 ROM...",
      "Load new 1001 ROM",
      load_rom_file_callback, "DosName1001", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new Professional DOS ROM...",
      "Load new Professional DOS ROM",
      load_rom_file_callback, "DriveProfDOS1571Name", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { "Load new SuperCard+ ROM...",
      "Load new SuperCard+ ROM",
      load_rom_file_callback, "DriveSuperCardName", 0,
      TUI_MENU_BEH_CONTINUE, NULL, NULL },
    { NULL }
};

/* ------------------------------------------------------------------------- */

TUI_MENU_DEFINE_TOGGLE(SFXSoundSampler)
TUI_MENU_DEFINE_TOGGLE(UserportRTC)
TUI_MENU_DEFINE_TOGGLE(UserportRTCSave)

int c128ui_init(void)
{
    tui_menu_t ui_ioextensions_submenu;

    ui_create_main_menu(1, 1, 1, 2, 1, drivec128_settings_submenu);

    tui_menu_add_separator(ui_special_submenu);

    uic128model_init(ui_special_submenu);

    ui_ioextensions_submenu = tui_menu_create("I/O extensions", 1);
    tui_menu_add(ui_ioextensions_submenu, ioextenstions_menu_items);
    tui_menu_add_submenu(ui_special_submenu, "_I/O extensions...",
                         "Configure I/O extensions",
                         ui_ioextensions_submenu,
                         NULL, 0,
                         TUI_MENU_BEH_CONTINUE);

    uic64cart_init(NULL);
    tui_menu_add_separator(ui_video_submenu);

    uivideo_init(ui_video_submenu, VID_VICII, VID_VDC);

    tui_menu_add(ui_sound_submenu, sid_c128_ui_menu_items);
    tui_menu_add(ui_rom_submenu, rom_menu_items);

    uiciamodel_double_init(ui_ioextensions_submenu);

    uilightpen_init(ui_ioextensions_submenu);

    uireu_init(ui_ioextensions_submenu);

    uigeoram_c64_init(ui_ioextensions_submenu);

    uiramcart_init(ui_ioextensions_submenu);

    uiexpert_init(ui_ioextensions_submenu);

    uiide64_init(ui_ioextensions_submenu);

    uidigimax_c64_init(ui_ioextensions_submenu);

    uids12c887rtc_c128_init(ui_ioextensions_submenu);

    uimmc64_init(ui_ioextensions_submenu);

    uimmcreplay_init(ui_ioextensions_submenu);

    uimagicvoice_init(ui_ioextensions_submenu);

#ifdef HAVE_TFE
    uitfe_c64_init(ui_ioextensions_submenu);
#endif

    uieasyflash_init(ui_ioextensions_submenu);

    uisoundexpander_c64_init(ui_ioextensions_submenu);

    tui_menu_add_item(ui_ioextensions_submenu, "Enable SFX Sound Sampler",
                      "Enable SFX Sound Sampler",
                      toggle_SFXSoundSampler_callback,
                      NULL, 3,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_item(ui_ioextensions_submenu, "Enable Userport RTC",
                      "Enable Userport RTC",
                      toggle_UserportRTC_callback,
                      NULL, 3,
                      TUI_MENU_BEH_CONTINUE);

    tui_menu_add_item(ui_ioextensions_submenu, "Save Userport RTC data when changed",
                      "Save Userport RTC data when changed",
                      toggle_UserportRTCSave_callback,
                      NULL, 3,
                      TUI_MENU_BEH_CONTINUE);

    return 0;
}

void c128ui_shutdown(void)
{
}
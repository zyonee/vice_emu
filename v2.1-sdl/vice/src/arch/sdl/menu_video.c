/*
 * menu_video.c - SDL video settings menus.
 *
 * Written by
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
#include "types.h"

#include "lib.h"
#include "machine.h"
#include "menu_common.h"
#include "menu_video.h"
#include "resources.h"
#include "ui.h"
#include "uifilereq.h"
#include "uimenu.h"
#include "vicii.h"

UI_MENU_DEFINE_RADIO(VICIIBorderMode)

static const ui_menu_entry_t vicii_border_menu[] = {
    { "Normal",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VICIIBorderMode_callback,
      (ui_callback_data_t)VICII_NORMAL_BORDERS },
    { "Full",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VICIIBorderMode_callback,
      (ui_callback_data_t)VICII_FULL_BORDERS },
    { "Debug",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_VICIIBorderMode_callback,
      (ui_callback_data_t)VICII_DEBUG_BORDERS },
    { NULL }
};

UI_MENU_DEFINE_TOGGLE(PALEmulation)
UI_MENU_DEFINE_INT(PALScanLineShade)
UI_MENU_DEFINE_INT(PALBlur)
UI_MENU_DEFINE_INT(PALOddLinePhase)
UI_MENU_DEFINE_INT(PALOddLineOffset)

static const ui_menu_entry_t pal_controls_menu[] = {
    { "PAL emulation",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_PALEmulation_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("PAL controls"),
    { "PAL shade (0-1000) : ",
      MENU_ENTRY_RESOURCE_INT,
      int_PALScanLineShade_callback,
      (ui_callback_data_t)"Set PAL shade (0-1000)"},
    { "PAL blur (0-1000) : ",
      MENU_ENTRY_RESOURCE_INT,
      int_PALBlur_callback,
      (ui_callback_data_t)"Set PAL blur (0-1000)"},
    { "PAL oddline phase (0-2000) : ",
      MENU_ENTRY_RESOURCE_INT,
      int_PALOddLinePhase_callback,
      (ui_callback_data_t)"Set PAL oddline phase (0-2000)"},
    { "PAL oddline offset (0-2000) : ",
      MENU_ENTRY_RESOURCE_INT,
      int_PALOddLineOffset_callback,
      (ui_callback_data_t)"Set PAL oddline offset (0-2000)"},
    { NULL }
};

UI_MENU_DEFINE_INT(ColorGamma)
UI_MENU_DEFINE_INT(ColorTint)
UI_MENU_DEFINE_INT(ColorSaturation)
UI_MENU_DEFINE_INT(ColorContrast)
UI_MENU_DEFINE_INT(ColorBrightness)

static const ui_menu_entry_t color_controls_menu[] = {
    { "Gamma (0-2000) : ",
      MENU_ENTRY_RESOURCE_INT,
      int_ColorGamma_callback,
      (ui_callback_data_t)"Set gamma (0-2000)"},
    { "Tint (0-2000) : ",
      MENU_ENTRY_RESOURCE_INT,
      int_ColorTint_callback,
      (ui_callback_data_t)"Set tint (0-2000)"},
    { "Saturation (0-2000) : ",
      MENU_ENTRY_RESOURCE_INT,
      int_ColorSaturation_callback,
      (ui_callback_data_t)"Set saturation (0-2000)"},
    { "Contrast (0-2000) : ",
      MENU_ENTRY_RESOURCE_INT,
      int_ColorContrast_callback,
      (ui_callback_data_t)"Set contrast (0-2000)"},
    { "Brightness (0-2000) : ",
      MENU_ENTRY_RESOURCE_INT,
      int_ColorBrightness_callback,
      (ui_callback_data_t)"Set brightness (0-2000)"},
    { NULL }
};

UI_MENU_DEFINE_TOGGLE(VICIIFullscreen)
UI_MENU_DEFINE_TOGGLE(VICIIDoubleSize)
UI_MENU_DEFINE_TOGGLE(VICIIDoubleScan)
UI_MENU_DEFINE_TOGGLE(VICIIVideoCache)
UI_MENU_DEFINE_TOGGLE(VDCDoubleSize)
UI_MENU_DEFINE_TOGGLE(VDCDoubleScan)
UI_MENU_DEFINE_TOGGLE(VDCVideoCache)
UI_MENU_DEFINE_TOGGLE(CrtcFullscreen)
UI_MENU_DEFINE_TOGGLE(CrtcDoubleSize)
UI_MENU_DEFINE_TOGGLE(CrtcDoubleScan)
UI_MENU_DEFINE_TOGGLE(CrtcVideoCache)
UI_MENU_DEFINE_TOGGLE(TEDFullscreen)
UI_MENU_DEFINE_TOGGLE(TEDDoubleSize)
UI_MENU_DEFINE_TOGGLE(TEDDoubleScan)
UI_MENU_DEFINE_TOGGLE(TEDVideoCache)
UI_MENU_DEFINE_TOGGLE(VICFullscreen)
UI_MENU_DEFINE_TOGGLE(VICDoubleSize)
UI_MENU_DEFINE_TOGGLE(VICDoubleScan)
UI_MENU_DEFINE_TOGGLE(VICVideoCache)
UI_MENU_DEFINE_TOGGLE(VICExternalPalette)
UI_MENU_DEFINE_TOGGLE(VICIIExternalPalette)
UI_MENU_DEFINE_TOGGLE(TEDExternalPalette)
UI_MENU_DEFINE_RADIO(MachineVideoStandard)

static UI_MENU_CALLBACK(external_vicii_palette_callback)
{
    char *name = NULL;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Choose VICII palette file", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            resources_set_string("VICIIPaletteFile", name);
            lib_free(name);
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(external_vdc_palette_callback)
{
    char *name = NULL;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Choose VDC palette file", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            resources_set_string("VDCPaletteFile", name);
            lib_free(name);
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(external_crtc_palette_callback)
{
    char *name = NULL;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Choose palette file", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            resources_set_string("CrtcPaletteFile", name);
            lib_free(name);
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(external_ted_palette_callback)
{
    char *name = NULL;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Choose palette file", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            resources_set_string("TEDPaletteFile", name);
            lib_free(name);
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(external_vic_palette_callback)
{
    char *name = NULL;

    if (activated) {
        name = sdl_ui_file_selection_dialog("Choose palette file", FILEREQ_MODE_CHOOSE_FILE);
        if (name != NULL) {
            resources_set_string("VICPaletteFile", name);
            lib_free(name);
        }
    }
    return NULL;
}

static UI_MENU_CALLBACK(radio_MachineVideoStandard_vic20_callback)
{
    if (activated) {
        int value = (int)param;
        sdl_ui_set_menu_borders(0, (value == MACHINE_SYNC_PAL) ? 28: 8);
    }
    return sdl_ui_menu_radio_helper(activated, param, "MachineVideoStandard");
}

const ui_menu_entry_t c128_video_menu[] = {
    { "Fullscreen",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIFullscreen_callback,
      NULL },
    { "VICII Double size",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIDoubleSize_callback,
      NULL },
    { "VICII Double scan",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIDoubleScan_callback,
      NULL },
    { "VICII Video cache",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIVideoCache_callback,
      NULL },
    { "VDC Double size",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VDCDoubleSize_callback,
      NULL },
    { "VDC Double scan",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VDCDoubleScan_callback,
      NULL },
    { "VDC Video cache",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VDCVideoCache_callback,
      NULL },
    { "VICII border mode",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)vicii_border_menu },
    { "Color controls",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)color_controls_menu },
    { "PAL emulation controls",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)pal_controls_menu },
    { "External VICII palette",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIExternalPalette_callback,
      NULL },
    { "Select VICII external palette file",
      MENU_ENTRY_DIALOG,
      external_vicii_palette_callback,
      NULL },
    { "Select VDC external palette file",
      MENU_ENTRY_DIALOG,
      external_vdc_palette_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Video Standard"),
    { "PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_callback,
      (ui_callback_data_t)MACHINE_SYNC_PAL },
    { "NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_callback,
      (ui_callback_data_t)MACHINE_SYNC_NTSC },
    { NULL }
};

const ui_menu_entry_t c64_video_menu[] = {
    { "Fullscreen",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIFullscreen_callback,
      NULL },
    { "Double size",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIDoubleSize_callback,
      NULL },
    { "Double scan",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIDoubleScan_callback,
      NULL },
    { "Video cache",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIVideoCache_callback,
      NULL },
    { "VICII border mode",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)vicii_border_menu },
    { "Color controls",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)color_controls_menu },
    { "PAL emulation controls",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)pal_controls_menu },
    { "External palette",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIExternalPalette_callback,
      NULL },
    { "Select external palette file",
      MENU_ENTRY_DIALOG,
      external_vicii_palette_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Video Standard"),
    { "PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_callback,
      (ui_callback_data_t)MACHINE_SYNC_PAL },
    { "NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_callback,
      (ui_callback_data_t)MACHINE_SYNC_NTSC },
    { "NTSC old",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_callback,
      (ui_callback_data_t)MACHINE_SYNC_NTSCOLD },
    { NULL }
};

const ui_menu_entry_t c64dtv_video_menu[] = {
    { "Fullscreen",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIFullscreen_callback,
      NULL },
    { "Double size",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIDoubleSize_callback,
      NULL },
    { "Double scan",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIDoubleScan_callback,
      NULL },
    { "Video cache (broken)",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIVideoCache_callback,
      NULL },
    { "VICII border mode",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)vicii_border_menu },
    { "Color controls",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)color_controls_menu },
    { "PAL emulation controls",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)pal_controls_menu },
    { "External palette",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIExternalPalette_callback,
      NULL },
    { "Select external palette file",
      MENU_ENTRY_DIALOG,
      external_vicii_palette_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Video Standard"),
    { "PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_callback,
      (ui_callback_data_t)MACHINE_SYNC_PAL },
    { "NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_callback,
      (ui_callback_data_t)MACHINE_SYNC_NTSC },
    { NULL }
};

const ui_menu_entry_t cbm5x0_video_menu[] = {
    { "Fullscreen",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIFullscreen_callback,
      NULL },
    { "Double size",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIDoubleSize_callback,
      NULL },
    { "Double scan",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIDoubleScan_callback,
      NULL },
    { "Video cache",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIVideoCache_callback,
      NULL },
    { "VICII border mode",
      MENU_ENTRY_SUBMENU,
      submenu_radio_callback,
      (ui_callback_data_t)vicii_border_menu },
    { "Color controls",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)color_controls_menu },
    { "PAL emulation controls",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)pal_controls_menu },
    { "External palette",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICIIExternalPalette_callback,
      NULL },
    { "Select external palette file",
      MENU_ENTRY_DIALOG,
      external_vicii_palette_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Video Standard"),
    { "PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_callback,
      (ui_callback_data_t)MACHINE_SYNC_PAL },
    { "NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_callback,
      (ui_callback_data_t)MACHINE_SYNC_NTSC },
    { NULL }
};

const ui_menu_entry_t cbm6x0_7x0_video_menu[] = {
    { "Fullscreen",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CrtcFullscreen_callback,
      NULL },
    { "Double size",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CrtcDoubleSize_callback,
      NULL },
    { "Double scan",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CrtcDoubleScan_callback,
      NULL },
    { "Video cache",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CrtcVideoCache_callback,
      NULL },
    { "Select external palette file",
      MENU_ENTRY_DIALOG,
      external_crtc_palette_callback,
      NULL },
    { NULL }
};

const ui_menu_entry_t pet_video_menu[] = {
    { "Fullscreen",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CrtcFullscreen_callback,
      NULL },
    { "Double size",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CrtcDoubleSize_callback,
      NULL },
    { "Double scan",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CrtcDoubleScan_callback,
      NULL },
    { "Video cache",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_CrtcVideoCache_callback,
      NULL },
    { "Select external palette file",
      MENU_ENTRY_DIALOG,
      external_crtc_palette_callback,
      NULL },
    { NULL }
};

const ui_menu_entry_t plus4_video_menu[] = {
    { "Fullscreen",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_TEDFullscreen_callback,
      NULL },
    { "Double size",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_TEDDoubleSize_callback,
      NULL },
    { "Double scan",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_TEDDoubleScan_callback,
      NULL },
    { "Video cache",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_TEDVideoCache_callback,
      NULL },
    { "Color controls",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)color_controls_menu },
    { "PAL emulation controls",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)pal_controls_menu },
    { "External palette",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_TEDExternalPalette_callback,
      NULL },
    { "Select external palette file",
      MENU_ENTRY_DIALOG,
      external_ted_palette_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Video Standard"),
    { "PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_callback,
      (ui_callback_data_t)MACHINE_SYNC_PAL },
    { "NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_callback,
      (ui_callback_data_t)MACHINE_SYNC_NTSC },
    { NULL }
};

const ui_menu_entry_t vic20_video_menu[] = {
    { "Fullscreen",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICFullscreen_callback,
      NULL },
    { "Double size",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICDoubleSize_callback,
      NULL },
    { "Double scan",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICDoubleScan_callback,
      NULL },
    { "Video cache",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICVideoCache_callback,
      NULL },
    { "Color controls",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)color_controls_menu },
    { "PAL emulation controls",
      MENU_ENTRY_SUBMENU,
      submenu_callback,
      (ui_callback_data_t)pal_controls_menu },
    { "External palette",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_VICExternalPalette_callback,
      NULL },
    { "Select external palette file",
      MENU_ENTRY_DIALOG,
      external_vic_palette_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("Video Standard"),
    { "PAL",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_vic20_callback,
      (ui_callback_data_t)MACHINE_SYNC_PAL },
    { "NTSC",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_MachineVideoStandard_vic20_callback,
      (ui_callback_data_t)MACHINE_SYNC_NTSC },
    { NULL }
};


/*
 * menu_video.c - Common SDL video settings functions.
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

#include <stdlib.h>

#include "machine.h"
#include "menu_common.h"
#include "menu_video.h"
#include "resources.h"
#include "ui.h"
#include "uimenu.h"

UI_MENU_DEFINE_INT(ColorGamma)
UI_MENU_DEFINE_INT(ColorTint)
UI_MENU_DEFINE_INT(ColorSaturation)
UI_MENU_DEFINE_INT(ColorContrast)
UI_MENU_DEFINE_INT(ColorBrightness)
UI_MENU_DEFINE_INT(PALScanLineShade)
UI_MENU_DEFINE_INT(PALBlur)
UI_MENU_DEFINE_INT(PALOddLinePhase)
UI_MENU_DEFINE_INT(PALOddLineOffset)
UI_MENU_DEFINE_TOGGLE(PALEmulation)
UI_MENU_DEFINE_RADIO(PALMode)

const ui_menu_entry_t pal_controls_menu[] = {
    { "PAL emulation",
      MENU_ENTRY_RESOURCE_TOGGLE,
      toggle_PALEmulation_callback,
      NULL },
    SDL_MENU_ITEM_SEPARATOR,
    SDL_MENU_ITEM_TITLE("PAL mode"),
    { "Fast",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_PALMode_callback,
      (ui_callback_data_t)0 },
    { "Old",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_PALMode_callback,
      (ui_callback_data_t)1 },
    { "New",
      MENU_ENTRY_RESOURCE_RADIO,
      radio_PALMode_callback,
      (ui_callback_data_t)2 },
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

const ui_menu_entry_t color_controls_menu[] = {
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

/*
 * uimmc64.c
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

#include <stdio.h>

#include "uilib.h"
#include "uimenu.h"
#include "uimmc64.h"

UI_MENU_DEFINE_TOGGLE(MMC64)
UI_MENU_DEFINE_RADIO(MMC64_revision)
UI_MENU_DEFINE_TOGGLE(MMC64_flashjumper)
UI_MENU_DEFINE_TOGGLE(MMC64_bios_write)
UI_MENU_DEFINE_TOGGLE(MMC64_RO)
UI_MENU_DEFINE_RADIO(MMC64_sd_type)

UI_CALLBACK(set_mmc64_bios_name)
{
    uilib_select_file((char *)UI_MENU_CB_PARAM, _("MMC64 BIOS name"), UILIB_FILTER_ALL);
}

UI_CALLBACK(set_mmc64_image_name)
{
    uilib_select_file((char *)UI_MENU_CB_PARAM, _("MMC64 image"), UILIB_FILTER_ALL);
}

static ui_menu_entry_t mmc64_revision_submenu[] = {
    { N_("Rev. A"), UI_MENU_TYPE_TICK, (ui_callback_t)radio_MMC64_revision,
      (ui_callback_data_t)0, NULL },
    { N_("Rev. B"), UI_MENU_TYPE_TICK, (ui_callback_t)radio_MMC64_revision,
      (ui_callback_data_t)1, NULL },
    { NULL }
};

static ui_menu_entry_t mmc64_sd_type_submenu[] = {
    { N_("Auto"), UI_MENU_TYPE_TICK, (ui_callback_t)radio_MMC64_sd_type,
      (ui_callback_data_t)0, NULL },
    { "MMC", UI_MENU_TYPE_TICK, (ui_callback_t)radio_MMC64_sd_type,
      (ui_callback_data_t)1, NULL },
    { "SD", UI_MENU_TYPE_TICK, (ui_callback_t)radio_MMC64_sd_type,
      (ui_callback_data_t)2, NULL },
    { "SDHC", UI_MENU_TYPE_TICK, (ui_callback_t)radio_MMC64_sd_type,
      (ui_callback_data_t)3, NULL },
    { NULL }
};

ui_menu_entry_t mmc64_submenu[] = {
    { N_("Enable MMC64"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_MMC64, NULL, NULL },
    { N_("MMC64 Revision"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, mmc64_revision_submenu },
    { N_("Enable MMC64 flashjumper"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_MMC64_flashjumper, NULL, NULL },
    { N_("Enable MMC64 BIOS save when changed"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_MMC64_bios_write, NULL, NULL },
    { N_("MMC64 BIOS name..."), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)set_mmc64_bios_name,
      (ui_callback_data_t)"MMC64BIOSfilename", NULL },
    { N_("Enable MMC64 image read-only"), UI_MENU_TYPE_TICK,
      (ui_callback_t)toggle_MMC64_RO, NULL, NULL },
    { N_("MMC64 image name..."), UI_MENU_TYPE_NORMAL,
      (ui_callback_t)set_mmc64_image_name,
      (ui_callback_data_t)"MMC64imagefilename", NULL },
    { N_("MMC64 card type"), UI_MENU_TYPE_NORMAL,
      NULL, NULL, mmc64_sd_type_submenu },
    { NULL }
};
